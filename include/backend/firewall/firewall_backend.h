#ifndef FIREWALL_BACKEND_H
#define FIREWALL_BACKEND_H

#include "iptables.h"
#include "libiptc/libiptc.h"

#include "backend/config_backend_base.h"
#include "backend/firewall/firewall_context.h"
#include "tools/cplog.h"
#include "tools/sys.h"

#include <bits/ranges_algo.h>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using std::function;
using std::ostringstream;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::vector;

class FirewallBackend : public ConfigBackendBase {
public:
  FirewallBackend();

  ~FirewallBackend() override;

  auto apply() -> function<bool()>;

  static auto getTableNames() -> vector<string> {
    static vector<string> tables = {"filter", "nat", "mangle", "raw",
                                    "security"};

    return tables;
  };

  auto getSubconfigs(const ctx_t &context) -> vector<string>;

  static auto createContext(const ctx_t &current, const string &name) -> ctx_t;

  /* remove chain when level is chain, rule when level is rule
   * return false when other level call this function
   */
  auto remove(const ctx_t &context) -> bool;

private:
  unordered_map<string, struct iptc_handle *> handles_;

  auto getChains(const ctx_t &context) -> vector<string>;

  auto getRules(const ctx_t &context) -> vector<const struct ipt_entry *>;

  /* tool func for iptable rules */
  static auto serializeRule(const struct ipt_entry *rule) -> string;

  static auto deserializeRule(const string &rule) -> struct ipt_entry *;

  static auto shortSerializeRule(const struct ipt_entry *rule) -> string;

  static auto ip2String(uint32_t ip) -> string;

  static auto iface2String(const char *iface) -> string;

  static auto proto2String(uint8_t proto) -> string;

  static constexpr auto kICMPType = 1;
  static constexpr auto kTCPType = 6;
  static constexpr auto kUDPType = 17;
};

#endif
