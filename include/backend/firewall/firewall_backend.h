#ifndef FIREWALL_BACKEND_H
#define FIREWALL_BACKEND_H

#include "iptables.h"
#include "libiptc/libiptc.h"

#include "backend/config_backend_base.h"
#include "tools/cplog.h"
#include "tools/sys.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class FirewallTabChain;

class FirewallBackend : public ConfigBackendBase {
public:
  FirewallBackend(const std::shared_ptr<ConfigBackendBase> &parent);

  ~FirewallBackend() override = default;

  auto getAllIPChain(int index) -> std::vector<std::string>;

  auto init() -> bool override;

private:
  struct iptc_handle *handle_;

  std::unordered_map<std::string, std::shared_ptr<FirewallTabChain>> tables_;

  static auto getTableNames() -> std::vector<std::string> {
    return {"filter", "nat", "mangle", "raw", "security"};
  };
};

#endif