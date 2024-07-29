#ifndef FIREWALL_BACKEND_H
#define FIREWALL_BACKEND_H

#include "iptables.h"
#include "libiptc/libiptc.h"

#include "backend/config_backend_base.h"
#include "backend/firewall/firewall_context.h"
#include "tools/cplog.h"
#include "tools/sys.h"

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using std::ostringstream;
using std::shared_ptr;
using std::string;
using std::vector;

class FirewallBackend : public ConfigBackendBase {
public:
  FirewallBackend();

  ~FirewallBackend() override;

  static auto getTableNames() -> vector<string> {
    static vector<string> tables = {"filter", "nat", "mangle", "raw",
                                    "security"};

    return tables;
  };

  auto getSubconfigs(const ctx_t &context) -> vector<string>;

  static auto createContext(const ctx_t &current, const string &name) -> ctx_t;

private:
  std::unordered_map<string, struct iptc_handle *> handles_;

  auto getChainNames(const ctx_t &context) -> vector<string>;

  auto getRules(const ctx_t &context) -> vector<const struct ipt_entry *>;

  /* tool func for iptable rules */
  static auto serializeRule(const struct ipt_entry *rule) -> string;

  static auto deserializeRule(const string &rule) -> struct ipt_entry *;

  static auto shortSerializeRule(const struct ipt_entry *rule) -> string;

  static auto ip2String(uint32_t ip) -> std::string;

  static auto iface2String(const char *iface) -> std::string;

  static auto proto2String(uint8_t proto) -> std::string;

  static constexpr auto kICMPType = 1;
  static constexpr auto kTCPType = 6;
  static constexpr auto kUDPType = 17;
};

#endif
