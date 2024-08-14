#ifndef FIREWALL_BACKEND_H
#define FIREWALL_BACKEND_H

#include "iptables.h"
#include "libiptc/libiptc.h"

#include "backend/config_backend_base.h"
#include "backend/firewall/chain_request.h"
#include "backend/firewall/firewall_context.h"
#include "backend/firewall/rule_request.h"
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

  /**
   * As backend, we need to implement the following functions:
   */
  auto apply() -> function<bool()>;

  /*
   * get all firewall tables statically
   */
  static auto getTableNames() -> vector<string>;

  static auto createContext(const ctx_t &current, const string &name) -> ctx_t;

  auto getFirewallChildren(const ctx_t &context) -> vector<string>;

  auto getRuleDetails(const ctx_t &context, int index) -> string;

  auto removeChain(const ctx_t &context) -> bool;

  auto insertChain(const ctx_t &context,
                   const shared_ptr<ChainRequest> &request) -> bool;

  auto removeRule(const ctx_t &context, int index) -> bool;

  auto insertRule(const ctx_t &context,
                  const shared_ptr<RuleRequest> &request) -> bool;

  auto updateRule(const ctx_t &context, const shared_ptr<RuleRequest> &request,
                  int index) -> bool;

  auto getRule(const ctx_t &context, int index) -> shared_ptr<RuleRequest>;

private:
  unordered_map<string, struct iptc_handle *> handles_;

  auto getChains(const ctx_t &context) -> vector<string>;

  auto getRules(const ctx_t &context) -> vector<const struct ipt_entry *>;

  /**
   * iptc handlers need freshed after committing
   */
  auto createHandlers() -> bool;

  auto destroyHandlers() -> bool;

  /* tool func for iptable rules */
  static auto serializeRule(iptc_handle *handle,
                            const struct ipt_entry *rule) -> string;

  static auto serializeShortRule(iptc_handle *handle,
                                 const struct ipt_entry *rule) -> string;

  /**
   * tool func for fronetend-backend conversion
   */
};

#endif
