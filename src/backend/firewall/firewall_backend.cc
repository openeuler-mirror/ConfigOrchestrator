#include "backend/firewall/firewall_backend.h"

#include <arpa/inet.h>
#include <memory>
#include <sstream>
#include <vector>

FirewallBackend::FirewallBackend() {
  auto tables = getTableNames();

  for (const auto &table : tables) {
    auto *handle = iptc_init(table.c_str());
    if (handle == nullptr) {
      yuiError() << "Error initializing iptables's table: " << table
                 << " error: " << iptc_strerror(errno) << endl;
    }

    handles_.insert({table, handle});
  }
}

FirewallBackend::~FirewallBackend() {
  for (auto [_, handle_] : handles_) {
    if (handle_ != nullptr) {
      iptc_free(handle_);
    }
  }
}

auto FirewallBackend::getSubconfigs(const shared_ptr<FirewallContext> &context)
    -> vector<string> {
  vector<string> subconfigs;

  switch (context->level_) {
  case FirewallLevel::OVERALL:
    subconfigs = getTableNames();
    break;
  case FirewallLevel::TABLE:
    subconfigs = getChainNames(context);
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
  shared_ptr<FirewallContext> context =
      std::make_shared<FirewallContext>(current);

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

auto FirewallBackend::getChainNames(const ctx_t &context) -> vector<string> {
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

auto FirewallBackend::serializeRule(const struct ipt_entry *rule) -> string {
  ostringstream ss;

  ss << "Source IP: " << ip2String(rule->ip.src.s_addr) << "\n";
  ss << "Destination IP: " << ip2String(rule->ip.dst.s_addr) << "\n";
  ss << "Source Mask: " << ip2String(rule->ip.smsk.s_addr) << "\n";
  ss << "Destination Mask: " << ip2String(rule->ip.dmsk.s_addr) << "\n";
  ss << "Protocol: " << proto2String(rule->ip.proto) << "\n";
  ss << "Flags: " << static_cast<int>(rule->ip.flags) << "\n";
  ss << "Inverse Flags: " << static_cast<int>(rule->ip.invflags) << "\n";
  ss << "Input Interface: " << iface2String(rule->ip.iniface) << "\n";
  ss << "Output Interface: " << iface2String(rule->ip.outiface) << "\n";
  ss << "Nf Cache: " << rule->nfcache << "\n";
  ss << "Target Offset: " << rule->target_offset << "\n";
  ss << "Next Offset: " << rule->next_offset << "\n";
  ss << "Come From: " << rule->comefrom << "\n";
  ss << "Packet Count: " << rule->counters.pcnt << "\n";
  ss << "Byte Count: " << rule->counters.bcnt << "\n";

  return ss.str();
}

auto FirewallBackend::shortSerializeRule(const struct ipt_entry *rule)
    -> string {
  ostringstream ss;

  ss << "SRC: " << ip2String(rule->ip.src.s_addr) << "/"
     << ip2String(rule->ip.smsk.s_addr)
     << ", DST: " << ip2String(rule->ip.dst.s_addr) << "/"
     << ip2String(rule->ip.dmsk.s_addr)
     << " | PROTO: " << proto2String(rule->ip.proto);

  return ss.str();
}

auto FirewallBackend::deserializeRule(const string &rule)
    -> struct ipt_entry * {
  string line;
  string proto;

  std::stringstream ss(rule);
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
    new_rule->ip.proto = kTCPType;
  } else if (proto == "UDP") {
    new_rule->ip.proto = kUDPType;
  } else if (proto == "ICMP") {
    new_rule->ip.proto = kICMPType;
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

auto FirewallBackend::ip2String(uint32_t ip) -> std::string {
  struct in_addr in;
  in.s_addr = ip;
  return inet_ntoa(in);
}

auto FirewallBackend::iface2String(const char *iface) -> std::string {
  return std::string(reinterpret_cast<const char *>(iface));
}

auto FirewallBackend::proto2String(uint8_t proto) -> std::string {
  switch (proto) {
  case kTCPType:
    return "TCP";
  case kUDPType:
    return "UDP";
  case kICMPType:
    return "ICMP";
  default:
    return "UNKNOWN";
  }
}
