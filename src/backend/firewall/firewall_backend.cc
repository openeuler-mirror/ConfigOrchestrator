#include "backend/firewall/firewall_backend.h"
#include "tools/iptools.h"

#include <algorithm>
#include <arpa/inet.h>
#include <asm-generic/int-ll64.h>
#include <bits/ranges_algo.h>
#include <cstring>
#include <libiptc/libiptc.h>
#include <linux/in.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sstream>
#include <string>
#include <vector>

auto FirewallBackend::createHandlers() -> bool {
  auto tables = getTableNames();

  return std::ranges::all_of(tables, [this](const auto &table) {
    auto *handle = iptc_init(table.c_str());
    if (handle == nullptr) {
      yuiError() << "Error initializing iptables's table: " << table
                 << " error: " << iptc_strerror(errno) << endl;
      return false;
    }

    handles_.insert({table, handle});
    return true;
  });
}

auto FirewallBackend::destroyHandlers() -> bool {
  auto r = std::ranges::all_of(handles_, [](const auto &pair) {
    if (pair.second != nullptr) {
      iptc_free(pair.second);
    }
    return true;
  });

  if (r) {
    handles_.clear();
    return true;
  }

  return false;
}

FirewallBackend::FirewallBackend() {
  if (!createHandlers()) {
    throw std::runtime_error("Error creating iptables's table handlers.");
  }
}

FirewallBackend::~FirewallBackend() { destroyHandlers(); }

auto FirewallBackend::apply() -> function<bool()> {
  return [this]() {
    if (std::ranges::all_of(handles_, [](const auto &pair) {
          if (iptc_commit(pair.second) > 0) {
            return true;
          }

          yuiError() << "Error committing iptables's table: " << pair.first
                     << ". " << iptc_strerror(errno) << endl;
          return false;
        })) {
      // after calling commit, close all handle and create new one
      return destroyHandlers() && createHandlers();
    }

    return false;
  };
}

auto FirewallBackend::getSubconfigs(const shared_ptr<FirewallContext> &context)
    -> vector<string> {
  vector<string> subconfigs;

  switch (context->level_) {
  case FirewallLevel::OVERALL:
    subconfigs = getTableNames();
    break;
  case FirewallLevel::TABLE:
    subconfigs = getChains(context);
    break;
  case FirewallLevel::CHAIN:
    auto rules = getRules(context);
    for (const auto &rule : rules) {
      subconfigs.emplace_back(shortSerializeRule(rule));
    }
    break;
  }

  return subconfigs;
}

auto FirewallBackend::createContext(const ctx_t &current, const string &name)
    -> ctx_t {
  shared_ptr<FirewallContext> context = make_shared<FirewallContext>(current);

  switch (context->level_) {
  case FirewallLevel::OVERALL:
    context->level_ = FirewallLevel::TABLE;
    context->table_ = name;
    break;
  case FirewallLevel::TABLE:
    context->level_ = FirewallLevel::CHAIN;
    context->chain_ = name;
    break;
  case FirewallLevel::CHAIN:
    yuiError() << "Cannot create context from chain." << endl;
    break;
  }

  return context;
}

auto FirewallBackend::getChains(const ctx_t &context) -> vector<string> {
  vector<string> chains;

  auto *handle = handles_.at(context->table_);
  for (const auto *chain = iptc_first_chain(handle); chain != nullptr;
       chain = iptc_next_chain(handle)) {
    chains.emplace_back(chain);
  }

  return chains;
}

auto FirewallBackend::getRules(const ctx_t &context)
    -> vector<const struct ipt_entry *> {
  vector<const struct ipt_entry *> rules;

  auto *handle = handles_.at(context->table_);
  auto chain = context->chain_;
  for (const struct ipt_entry *entry = iptc_first_rule(chain.c_str(), handle);
       entry != nullptr; entry = iptc_next_rule(entry, handle)) {
    rules.emplace_back(entry);
  }

  return rules;
}

auto FirewallBackend::removeChain(const ctx_t &context) -> bool {
  if (context->level_ == FirewallLevel::CHAIN) {
    return iptc_delete_chain(context->chain_.c_str(),
                             handles_.at(context->table_)) == 0;
  }

  if (context->level_ == FirewallLevel::OVERALL) {
    auto *handle = handles_.at(context->table_);
    (void)handle;
  } else {
    yuiError() << "Cannot remove chain from other." << endl;
    return false;
  }

  return true;
}

auto FirewallBackend::removeRule(const ctx_t &context, int index) -> bool {
  auto *handle = handles_.at(context->table_);
  auto chain = context->chain_;

  if (iptc_delete_num_entry(chain.c_str(), index, handle) == 0) {
    yuiError() << "Error deleting rule: " << iptc_strerror(errno) << endl;
    context->setLastError(iptc_strerror(errno));
    return false;
  }

  return true;
}

auto FirewallBackend::addRule(const ctx_t &context,
                              const shared_ptr<RuleRequest> &request) -> bool {
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

  if (context->level_ != FirewallLevel::CHAIN) {
    yuiError() << "Cannot add rule to table over chain" << endl;
    return false;
  }

  auto *handle = handles_.at(context->table_);
  ipt_chainlabel chain;
  strncpy(chain, context->chain_.c_str(), sizeof(ipt_chainlabel));

  /* calculate size of the entry */
  auto matches_number = static_cast<int>(request->matches_.size());
  auto size = kIPTEntrySize + kIPTEntryTargetSize + kMatchSize * matches_number;

  std::vector<char> entry_buffer(size, 0);
  auto *entry = reinterpret_cast<struct ipt_entry *>(entry_buffer.data());
  auto *target_entry = reinterpret_cast<struct ipt_entry_target *>(
      entry_buffer.data() + kIPTEntrySize + kMatchSize * matches_number);

  /* Part I: ipt_entry */
  entry->next_offset = size;
  entry->target_offset = kIPTEntrySize + kMatchSize * matches_number;

  if (request->proto_ == RequestProto::TCP) {
    entry->ip.proto = IPPROTO_TCP;
  } else if (request->proto_ == RequestProto::UDP) {
    entry->ip.proto = IPPROTO_UDP;
  } else {
    yuiError() << "Unknown protocol: " << request->proto_ << endl;
    return false;
  }

  /* src/dst ip and mask */
  auto setIp = [](const optional<string> &ip, struct in_addr &target) {
    if (ip.has_value()) {
      target.s_addr = inet_addr(ip->c_str());
    }
  };
  setIp(request->src_ip_, entry->ip.src);
  setIp(request->src_mask_, entry->ip.smsk);
  setIp(request->dst_ip_, entry->ip.dst);
  setIp(request->dst_mask_, entry->ip.dmsk);

  /* iface */
  if (request->iniface_.has_value()) {
    strncpy(entry->ip.iniface, request->iniface_->c_str(), IFNAMSIZ);
    memset(entry->ip.iniface_mask, kByteMask, request->iniface_->size() + 1);
  }
  if (request->outiface_.has_value()) {
    strncpy(entry->ip.outiface, request->outiface_.value().c_str(), IFNAMSIZ);
    memset(entry->ip.outiface_mask, kByteMask, request->outiface_->size() + 1);
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

  for (int i = 0; i < request->matches_.size(); i++) {
    if (request->proto_ == RequestProto::TCP) {
      auto *match = reinterpret_cast<struct ipt_entry_match *>(
          entry->elems + i * kTCPMatchSize);

      match->u.user.match_size = kTCPMatchSize;
      strncpy(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN);

      auto *tcp = reinterpret_cast<struct ipt_tcp *>(match->data);
      setPortRange(request->matches_[i].src_port_range_, tcp->spts);
      setPortRange(request->matches_[i].dst_port_range_, tcp->dpts);
    } else if (request->proto_ == RequestProto::UDP) {
      auto *match = reinterpret_cast<struct ipt_entry_match *>(
          entry->elems + i * kUDPMatchSize);

      match->u.user.match_size = kUDPMatchSize;
      strncpy(match->u.user.name, "udp", IPT_FUNCTION_MAXNAMELEN);

      auto *udp = reinterpret_cast<struct ipt_udp *>(match->data);
      setPortRange(request->matches_[i].src_port_range_, udp->spts);
      setPortRange(request->matches_[i].dst_port_range_, udp->dpts);
    }
  }

  /* Part III: target */
  target_entry->u.user.target_size = kIPTEntryTargetSize;
  if (request->target_.empty() ||
      std::any_of(iptTargets().begin(), iptTargets().end(),
                  [&request](const auto &target) {
                    return target == request->target_;
                  })) {
    strncpy(target_entry->u.user.name, request->target_.c_str(),
            sizeof(target_entry->u.user.name));
  }

  /* insert it */
  if (request->index_ > getSubconfigs(context).size()) {
    if (iptc_append_entry(chain, entry, handle) == 0) {
      yuiError() << "Error adding rule: " << iptc_strerror(errno) << endl;
      return false;
    }
  } else if (iptc_insert_entry(chain, entry, request->index_, handle) == 0) {
    std::cerr << "Error adding rule: " << iptc_strerror(errno) << std::endl;
    return false;
  }

  return true;
}

auto FirewallBackend::serializeRule(const struct ipt_entry *rule) -> string {
  ostringstream ss;

  ss << "Source IP: " << string(inet_ntoa(rule->ip.src)) << "\n";
  ss << "Destination IP: " << string(inet_ntoa(rule->ip.dst)) << "\n";
  ss << "Source Mask: " << string(inet_ntoa(rule->ip.smsk)) << "\n";
  ss << "Destination Mask: " << string(inet_ntoa(rule->ip.dmsk)) << "\n";
  ss << "Protocol: " << proto2String(rule->ip.proto) << "\n";
  ss << "Flags: " << static_cast<int>(rule->ip.flags) << "\n";
  ss << "Inverse Flags: " << static_cast<int>(rule->ip.invflags) << "\n";
  ss << "Input Interface: " << string(rule->ip.iniface) << "\n";
  ss << "Output Interface: " << string(rule->ip.outiface) << "\n";
  ss << "Nf Cache: " << rule->nfcache << "\n";
  ss << "Target Offset: " << rule->target_offset << "\n";
  ss << "Next Offset: " << rule->next_offset << "\n";
  ss << "Come From: " << rule->comefrom << "\n";
  ss << "Packet Count: " << rule->counters.pcnt << "\n";
  ss << "Byte Count: " << rule->counters.bcnt << "\n";

  const auto *match = reinterpret_cast<const ipt_entry_match *>(rule->elems);
  while (reinterpret_cast<const char *>(match) !=
         reinterpret_cast<const char *>(rule) + rule->target_offset) {
    ss << "Match Name: " << match->u.user.name << "\n";
    ss << "Match Size: " << match->u.match_size << "\n";

    if (rule->ip.proto == IPPROTO_TCP) {
      const auto *tcp = reinterpret_cast<const ipt_tcp *>(match->data);
      ss << "Source Port: " << tcp->spts[0] << " - " << tcp->spts[1] << "\n";
      ss << "Destination Port: " << tcp->dpts[0] << " - " << tcp->dpts[1]
         << "\n";
    } else if (rule->ip.proto == IPPROTO_UDP) {
      const auto *udp = reinterpret_cast<const ipt_udp *>(match->data);
      ss << "Source Port: " << udp->spts[0] << " - " << udp->spts[1] << "\n";
      ss << "Destination Port: " << udp->dpts[0] << " - " << udp->dpts[1]
         << "\n";
    }

    match = reinterpret_cast<const ipt_entry_match *>(
        reinterpret_cast<const char *>(match) + match->u.match_size);
  }

  if (rule->target_offset != rule->next_offset) {
    const auto *target = reinterpret_cast<const ipt_entry_target *>(
        reinterpret_cast<const char *>(rule) + rule->target_offset);
    ss << "Target Name: " << target->u.user.name << "\n";
    ss << "Target Size: " << target->u.user.target_size << "\n";
  }
  return ss.str();
}

auto FirewallBackend::addChain(const ctx_t &context,
                               const shared_ptr<ChainRequest> &request)
    -> bool {
  auto *handle = handles_.at(context->table_);
  ipt_chainlabel chain;
  strncpy(chain, request->chain_name_.c_str(), sizeof(ipt_chainlabel));

  if (iptc_create_chain(chain, handle) == 0) {
    yuiError() << "Error creating chain: " << iptc_strerror(errno) << endl;
    return false;
  }

  return true;
}

auto FirewallBackend::shortSerializeRule(const struct ipt_entry *rule)
    -> string {
  ostringstream ss;

  ss << "SRC: " << string(inet_ntoa(rule->ip.src)) << "/"
     << string(inet_ntoa(rule->ip.smsk))
     << ", DST: " << string(inet_ntoa(rule->ip.dst)) << "/"
     << string(inet_ntoa(rule->ip.dmsk))
     << " | PROTO: " << proto2String(rule->ip.proto);

  return ss.str();
}

auto FirewallBackend::deserializeRule(const string &rule)
    -> struct ipt_entry * {
  string line;
  string proto;

  stringstream ss(rule);
  auto new_rule = std::make_unique<struct ipt_entry>();

  getline(ss, line, ':');
  ss >> new_rule->ip.src.s_addr;
  getline(ss, line, ':');
  ss >> new_rule->ip.dst.s_addr;
  getline(ss, line, ':');
  ss >> new_rule->ip.smsk.s_addr;
  getline(ss, line, ':');
  ss >> new_rule->ip.dmsk.s_addr;
  getline(ss, line, ':');

  ss >> proto;
  if (proto == "TCP") {
    new_rule->ip.proto = IPPROTO_TCP;
  } else if (proto == "UDP") {
    new_rule->ip.proto = IPPROTO_UDP;
  } else if (proto == "ICMP") {
    new_rule->ip.proto = IPPROTO_ICMP;
  } else {
    new_rule->ip.proto = 0;
  }

  getline(ss, line, ':');
  ss >> new_rule->ip.flags;
  getline(ss, line, ':');
  ss >> new_rule->ip.invflags;

  getline(ss, line, ':');
  ss >> new_rule->ip.iniface;
  getline(ss, line, ':');
  ss >> new_rule->ip.outiface;

  getline(ss, line, ':');
  ss >> new_rule->nfcache;
  getline(ss, line, ':');
  ss >> new_rule->target_offset;
  getline(ss, line, ':');
  ss >> new_rule->next_offset;
  getline(ss, line, ':');
  ss >> new_rule->comefrom;
  getline(ss, line, ':');
  ss >> new_rule->counters.pcnt;
  getline(ss, line, ':');
  ss >> new_rule->counters.bcnt;

  return new_rule.release();
}
