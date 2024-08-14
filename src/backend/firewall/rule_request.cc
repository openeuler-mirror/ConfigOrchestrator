#include "backend/firewall/rule_request.h"
#include "fmt/format.h"
#include "tools/log.h"
#include "tools/nettools.h"
#include <arpa/inet.h>
#include <cstring>
#include <libiptc/libiptc.h>
#include <optional>
#include <string>
#include <utility>

RuleRequest::RuleRequest(iptc_handle *handle, const struct ipt_entry *rule,
                         int index) {
  index_ = index;

  src_ip_ = fmt::to_string(inet_ntoa(rule->ip.src));
  src_mask_ = fmt::to_string(inet_ntoa(rule->ip.smsk));
  dst_ip_ = fmt::to_string(inet_ntoa(rule->ip.dst));
  dst_mask_ = fmt::to_string(inet_ntoa(rule->ip.dmsk));

  proto_ = proto2String(rule->ip.proto);
  if (strlen(rule->ip.iniface) > 0) {
    iniface_ = string(rule->ip.iniface);
  }
  if (strlen(rule->ip.outiface) > 0) {
    outiface_ = string(rule->ip.outiface);
  }
  const auto *match = reinterpret_cast<const ipt_entry_match *>(rule->elems);
  while (reinterpret_cast<const char *>(match) !=
         reinterpret_cast<const char *>(rule) + rule->target_offset) {
    RuleMatch rule_match;

    if (rule->ip.proto == IPPROTO_TCP) {
      const auto *xxp = reinterpret_cast<const ipt_tcp *>(match->data);
      rule_match.src_port_range_ = std::make_tuple(
          std::to_string(xxp->spts[0]), std::to_string(xxp->spts[1]));
      rule_match.dst_port_range_ = std::make_tuple(
          std::to_string(xxp->dpts[0]), std::to_string(xxp->dpts[1]));
    } else if (rule->ip.proto == IPPROTO_UDP) {
      const auto *xxp = reinterpret_cast<const ipt_udp *>(match->data);
      rule_match.src_port_range_ = std::make_tuple(
          std::to_string(xxp->spts[0]), std::to_string(xxp->spts[1]));
      rule_match.dst_port_range_ = std::make_tuple(
          std::to_string(xxp->dpts[0]), std::to_string(xxp->dpts[1]));
    }

    matches_.emplace_back(std::move(rule_match));
    match = reinterpret_cast<const ipt_entry_match *>(
        reinterpret_cast<const char *>(match) + match->u.match_size);
  }

  if (rule->target_offset != rule->next_offset) {
    target_ = iptc_get_target(rule, handle);
  }
}

auto RuleRequest::to_entry_bytes(const ctx_t &context)
    -> optional<vector<char>> {
  static constexpr int kIPTEntrySize = XT_ALIGN(sizeof(struct ipt_entry));
  static constexpr int kTCPMatchSize =
      XT_ALIGN(sizeof(struct ipt_entry_match) + sizeof(struct ipt_tcp));
  static constexpr int kUDPMatchSize =
      XT_ALIGN(sizeof(struct ipt_entry_match) + sizeof(struct ipt_udp));
  static constexpr int kIPTEntryTargetSize =
      XT_ALIGN(sizeof(struct ipt_entry_target)) + XT_ALIGN(sizeof(int));
  static constexpr int kMatchSize = kTCPMatchSize;
  static constexpr int kByteMask = 0xFF;
  static constexpr int max_port = 0xFFFF;

  static_assert(
      kTCPMatchSize == kUDPMatchSize,
      "reconsider the code iff tcp and udp match sizes are different");

  /* calculate size of the entry */
  auto matches_number = static_cast<int>(matches_.size());
  auto size = kIPTEntrySize + kIPTEntryTargetSize + kMatchSize * matches_number;

  std::vector<char> entry_buffer(size, 0);
  auto *entry = reinterpret_cast<struct ipt_entry *>(entry_buffer.data());
  auto *target_entry = reinterpret_cast<struct ipt_entry_target *>(
      entry_buffer.data() + kIPTEntrySize +
      static_cast<long>(kMatchSize * matches_number));

  /* Part I: ipt_entry */
  entry->next_offset = size;
  entry->target_offset = kIPTEntrySize + kMatchSize * matches_number;

  if (proto_ == RequestProto::TCP) {
    entry->ip.proto = IPPROTO_TCP;
  } else if (proto_ == RequestProto::UDP) {
    entry->ip.proto = IPPROTO_UDP;
  } else {
    auto msg = fmt::format("Unknown protocol: {}\n", proto_);
    context->setLastError(msg);
    return std::nullopt;
  }

  /* src/dst ip and mask */
  auto setIp = [](const optional<string> &ip, struct in_addr &target,
                  bool is_mask) {
    if (ip.has_value()) {
      target.s_addr = inet_addr(ip->c_str());
    } else {
      if (is_mask) {
        target.s_addr = INADDR_NONE;
      } else {
        target.s_addr = INADDR_ANY;
      }
    }
  };

  setIp(src_ip_, entry->ip.src, false);
  setIp(src_mask_, entry->ip.smsk, true);
  setIp(dst_ip_, entry->ip.dst, false);
  setIp(dst_mask_, entry->ip.dmsk, true);

  /* iface */
  if (iniface_.has_value()) {
    strncpy(entry->ip.iniface, iniface_->c_str(), IFNAMSIZ);
    memset(entry->ip.iniface_mask, kByteMask, iniface_->size() + 1);
  }
  if (outiface_.has_value()) {
    strncpy(entry->ip.outiface, outiface_->c_str(), IFNAMSIZ);
    memset(entry->ip.outiface_mask, kByteMask, outiface_->size() + 1);
  }

  /* we cannot use array here, assert no overflow todo */
  auto setPortRange = [](const optional<tuple<string, string>> &range,
                         __u16 *target) {
    if (range.has_value()) {
      target[0] = static_cast<__u16>(stoi(get<0>(range.value())));
      target[1] = static_cast<__u16>(stoi(get<1>(range.value())));
    } else {
      target[0] = 0;
      target[1] = max_port;
    }
  };

  for (int i = 0; i < matches_.size(); i++) {
    if (proto_ == RequestProto::TCP) {
      auto *match = reinterpret_cast<struct ipt_entry_match *>(
          entry->elems + static_cast<ptrdiff_t>(i * kTCPMatchSize));

      match->u.user.match_size = kTCPMatchSize;
      strncpy(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN);

      auto *tcp = reinterpret_cast<struct ipt_tcp *>(match->data);
      setPortRange(matches_[i].src_port_range_, tcp->spts);
      setPortRange(matches_[i].dst_port_range_, tcp->dpts);
    } else if (proto_ == RequestProto::UDP) {
      auto *match = reinterpret_cast<struct ipt_entry_match *>(
          entry->elems + static_cast<ptrdiff_t>(i * kUDPMatchSize));

      match->u.user.match_size = kUDPMatchSize;
      strncpy(match->u.user.name, "udp", IPT_FUNCTION_MAXNAMELEN);

      auto *udp = reinterpret_cast<struct ipt_udp *>(match->data);
      setPortRange(matches_[i].src_port_range_, udp->spts);
      setPortRange(matches_[i].dst_port_range_, udp->dpts);
    }
  }

  /* Part III: target */
  target_entry->u.user.target_size = kIPTEntryTargetSize;
  if (target_.empty() ||
      std::any_of(iptTargets().begin(), iptTargets().end(),
                  [this](const auto &target) { return target == target_; })) {
    strncpy(target_entry->u.user.name, target_.c_str(),
            IPT_FUNCTION_MAXNAMELEN);
  }

  return entry_buffer;
}
