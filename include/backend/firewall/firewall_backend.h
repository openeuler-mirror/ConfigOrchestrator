#ifndef FIREWALL_BACKEND_H
#define FIREWALL_BACKEND_H

#include "iptables.h"
#include "libiptc/libiptc.h"

#include "backend/config_backend_base.h"
#include "backend/firewall/firewall_context.h"
#include "frontend/firewall/chain_request.h"
#include "frontend/firewall/rule_request.h"
#include "tools/log.h"
#include "tools/sys.h"

#include <bits/ranges_algo.h>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using std::array;
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
    static vector<string> tables = {"filter"};

    return tables;
  };

  auto getSubconfigs(const ctx_t &context) -> vector<string>;

  auto getDetailedRule(const ctx_t &context, int index) -> string;

  static auto createContext(const ctx_t &current, const string &name) -> ctx_t;

  /* remove chain when level is chain, rule when level is rule
   * return false when other level call this function
   */
  auto removeChain(const ctx_t &context) -> bool;

  auto removeRule(const ctx_t &context, int index) -> bool;

  auto addRule(const ctx_t &context,
               const shared_ptr<RuleRequest> &request) -> bool;

  auto addChain(const ctx_t &context,
                const shared_ptr<ChainRequest> &request) -> bool;

private:
  unordered_map<string, struct iptc_handle *> handles_;

  auto getChains(const ctx_t &context) -> vector<string>;

  auto getRules(const ctx_t &context) -> vector<const struct ipt_entry *>;

  auto createHandlers() -> bool;

  auto destroyHandlers() -> bool;

  /* tool func for iptable rules */
  static auto serializeRule(const struct ipt_entry *rule) -> string;

  static auto serializeShortRule(const struct ipt_entry *rule) -> string;
};

#endif
