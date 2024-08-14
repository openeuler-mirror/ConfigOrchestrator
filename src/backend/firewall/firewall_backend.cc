#include "backend/firewall/firewall_backend.h"
#include "backend/firewall/rule_request.h"
#include "fmt/core.h"
#include "tools/iptools.h"
#include "tools/log.h"

#include <algorithm>
#include <arpa/inet.h>
#include <asm-generic/int-ll64.h>
#include <bits/ranges_algo.h>
#include <cstddef>
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

auto FirewallBackend::getTableNames() -> vector<string> {
  const static vector<string> tables = {"filter", "nat", "mangle", "raw",
                                        "security"};

  return tables;
};

auto FirewallBackend::destroyHandlers() -> bool {
  if (std::ranges::all_of(handles_, [](const auto &pair) {
        if (pair.second != nullptr) {
          iptc_free(pair.second);
        }
        return true;
      })) {
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

auto FirewallBackend::getFirewallChildren(
    const shared_ptr<FirewallContext> &context) -> vector<string> {
  vector<string> subconfigs;

  switch (context->level_) {
  case FirewallLevel::OVERALL:
    subconfigs = getTableNames();
    break;
  case FirewallLevel::TABLE:
    subconfigs = getChains(context);
    break;
  case FirewallLevel::CHAIN:
    auto *handle = handles_.at(context->table_);
    auto rules = getRules(context);
    for (const auto &rule : rules) {
      subconfigs.emplace_back(serializeShortRule(handle, rule));
    }
    break;
  }

  return subconfigs;
}

auto FirewallBackend::getRuleDetails(const ctx_t &context,
                                     int index) -> string {
  auto *handle = handles_.at(context->table_);
  auto chain = context->chain_;
  const auto *rule = iptc_first_rule(chain.c_str(), handle);

  for (int i = 0; i < index; i++) {
    rule = iptc_next_rule(rule, handle);
  }

  return serializeRule(handle, rule);
}

auto FirewallBackend::createContext(const ctx_t &current,
                                    const string &name) -> ctx_t {
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

auto FirewallBackend::getRule(const ctx_t &context,
                              int index) -> shared_ptr<RuleRequest> {
  auto *handle = handles_.at(context->table_);
  auto chain = context->chain_;
  const auto *rule = iptc_first_rule(chain.c_str(), handle);
  for (int i = 0; i < index; i++) {
    rule = iptc_next_rule(rule, handle);
  }

  return std::make_shared<RuleRequest>(handle, rule, index);
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

auto FirewallBackend::updateRule(const ctx_t &context,
                                 const shared_ptr<RuleRequest> &request,
                                 int index) -> bool {
  if (context->level_ != FirewallLevel::CHAIN) {
    context->setLastError("Cannot update rule over table.");
    return false;
  }

  ipt_chainlabel chain;
  auto *handle = handles_.at(context->table_);
  strncpy(chain, context->chain_.c_str(), sizeof(ipt_chainlabel));

  if (auto entry_buffer = request->to_entry_bytes(context)) {
    auto *entry = reinterpret_cast<struct ipt_entry *>(entry_buffer->data());
    if (iptc_replace_entry(chain, entry, request->index_, handle) == 0) {
      context->setLastError(
          fmt::format("Error update rule, reason: {}\n", iptc_strerror(errno)));
      return false;
    }
  } else {
    context->setLastError(fmt::format("Error creating rule entry, reason: {}",
                                      context->getLastError()));
    return false;
  }

  return true;
}

auto FirewallBackend::insertRule(
    const ctx_t &context, const shared_ptr<RuleRequest> &request) -> bool {

  if (context->level_ != FirewallLevel::CHAIN) {
    context->setLastError(fmt::format("Cannot add rule to table over chain.\n",
                                      iptc_strerror(errno)));
    return false;
  }

  ipt_chainlabel chain;
  auto *handle = handles_.at(context->table_);
  strncpy(chain, context->chain_.c_str(), sizeof(ipt_chainlabel));

  /* insert it */
  if (auto entry_buffer = request->to_entry_bytes(context)) {
    auto *entry = reinterpret_cast<struct ipt_entry *>(entry_buffer->data());
    if (request->index_ > getFirewallChildren(context).size()) {
      if (iptc_append_entry(chain, entry, handle) == 0) {
        context->setLastError(fmt::format("Error insert rule, reason: {}\n",
                                          iptc_strerror(errno)));
        return false;
      }
    } else if (iptc_insert_entry(chain, entry, request->index_, handle) == 0) {
      context->setLastError(
          fmt::format("Error insert rule, reason: {}\n", iptc_strerror(errno)));
      return false;
    }
  } else {
    context->setLastError(fmt::format("Error creating rule entry, reason: {}",
                                      context->getLastError()));
    return false;
  }

  return true;
}

auto FirewallBackend::insertChain(
    const ctx_t &context, const shared_ptr<ChainRequest> &request) -> bool {
  auto *handle = handles_.at(context->table_);
  ipt_chainlabel chain;
  strncpy(chain, request->chain_name_.c_str(), sizeof(ipt_chainlabel));

  if (iptc_create_chain(chain, handle) == 0) {
    context->setLastError(
        fmt::format("Error creating chain: {}\n", iptc_strerror(errno)));
    return false;
  }

  return true;
}

auto FirewallBackend::serializeRule(iptc_handle *handle,
                                    const struct ipt_entry *rule) -> string {
  std::string result;

  result += fmt::format("Source IP: {}\n", inet_ntoa(rule->ip.src));
  result += fmt::format("Destination IP: {}\n", inet_ntoa(rule->ip.dst));
  result += fmt::format("Source Mask: {}\n", inet_ntoa(rule->ip.smsk));
  result += fmt::format("Destination Mask: {}\n", inet_ntoa(rule->ip.dmsk));
  result += fmt::format("Protocol: {}\n", proto2String(rule->ip.proto));
  result += fmt::format("Flags: {}\n", rule->ip.flags);
  result += fmt::format("Inverse Flags: {}\n", rule->ip.invflags);
  result += fmt::format("Input Interface: {}\n", rule->ip.iniface);
  result += fmt::format("Output Interface: {}\n", rule->ip.outiface);
  result += fmt::format("Nf Cache: {}\n", rule->nfcache);
  result += fmt::format("Come From: {}\n", rule->comefrom);
  result += fmt::format("Packet Count: {}\n", rule->counters.pcnt);
  result += fmt::format("Byte Count: {}\n", rule->counters.bcnt);

  const auto *match = reinterpret_cast<const ipt_entry_match *>(rule->elems);
  while (reinterpret_cast<const char *>(match) !=
         reinterpret_cast<const char *>(rule) + rule->target_offset) {
    result += fmt::format("Match Name: {}\n", match->u.user.name);
    result += fmt::format("Match Size: {}\n", match->u.match_size);

    if (rule->ip.proto == IPPROTO_TCP) {
      const auto *tcp = reinterpret_cast<const ipt_tcp *>(match->data);
      result += fmt::format("Src Port: {} - {}\n", tcp->spts[0], tcp->spts[1]);
      result += fmt::format("Dest Port: {} - {}\n", tcp->dpts[0], tcp->dpts[1]);
    } else if (rule->ip.proto == IPPROTO_UDP) {
      const auto *udp = reinterpret_cast<const ipt_udp *>(match->data);
      result += fmt::format("Src Port: {} - {}\n", udp->spts[0], udp->spts[1]);
      result += fmt::format("Dest Port: {} - {}\n", udp->dpts[0], udp->dpts[1]);
    }

    match = reinterpret_cast<const ipt_entry_match *>(
        reinterpret_cast<const char *>(match) + match->u.match_size);
  }

  if (rule->target_offset != rule->next_offset) {
    const auto *target = reinterpret_cast<const ipt_entry_target *>(
        reinterpret_cast<const char *>(rule) + rule->target_offset);
    auto target_name = iptc_get_target(rule, handle);
    result += fmt::format("Target Name: {}\n", target_name);
    result += fmt::format("Target Size: {}\n", target->u.user.target_size);
  }

  return result;
}

auto FirewallBackend::serializeShortRule(
    iptc_handle *handle, const struct ipt_entry *rule) -> string {
  std::string result;
  result += fmt::format("SRC: {}, DST: {}, PROTO: {}", inet_ntoa(rule->ip.src),
                        inet_ntoa(rule->ip.dst), proto2String(rule->ip.proto));

  static constexpr int kMinPort = 0;
  static constexpr int kMaxPort = 65535;
  const auto *match = reinterpret_cast<const ipt_entry_match *>(rule->elems);
  while (reinterpret_cast<const char *>(match) !=
         reinterpret_cast<const char *>(rule) + rule->target_offset) {
    auto srcs = std::make_pair(kMinPort, kMaxPort);
    auto dsts = std::make_pair(kMinPort, kMaxPort);
    if (rule->ip.proto == IPPROTO_TCP) {
      const auto *tcp = reinterpret_cast<const ipt_tcp *>(match->data);
      srcs = std::make_pair(tcp->spts[0], tcp->spts[1]);
      dsts = std::make_pair(tcp->dpts[0], tcp->dpts[1]);
    } else if (rule->ip.proto == IPPROTO_UDP) {
      const auto *udp = reinterpret_cast<const ipt_udp *>(match->data);
      srcs = std::make_pair(udp->spts[0], udp->spts[1]);
      dsts = std::make_pair(udp->dpts[0], udp->dpts[1]);
    }
    if (srcs.first != kMinPort && srcs.second != kMaxPort) {
      result += fmt::format(", SRC PORT: {}-{}", srcs.first, srcs.second);
    }
    if (dsts.first != kMinPort && dsts.second != kMaxPort) {
      result += fmt::format(", DST PORT: {}-{}", dsts.first, dsts.second);
    }

    match = reinterpret_cast<const ipt_entry_match *>(
        reinterpret_cast<const char *>(match) + match->u.match_size);
  }

  if (rule->target_offset != rule->next_offset) {
    auto target_name = string(iptc_get_target(rule, handle));
    if (!target_name.empty()) {
      result += fmt::format(" | {}\n", target_name);
    }
  }

  return result;
}
