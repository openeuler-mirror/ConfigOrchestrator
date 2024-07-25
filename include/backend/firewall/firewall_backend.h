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
#include <utility>
#include <vector>

enum class FirewallBackendType { OVERALL, TABLE, CHAIN };
class FirewallTabChain;

class FirewallBackend : public ConfigBackendBase {
public:
  FirewallBackend(const std::shared_ptr<ConfigBackendBase> &parent,
                  FirewallBackendType type, std::string name)
      : ConfigBackendBase(parent), type_(type), name_(std::move(name)){};

  ~FirewallBackend() override {
    if (type_ == FirewallBackendType::TABLE && handle_ != nullptr) {
      iptc_free(handle_);
    }
  }

  static auto getTableNames() -> std::vector<std::string> {
    static std::vector<std::string> tables = {"filter", "nat", "mangle", "raw",
                                              "security"};

    return tables;
  };

  auto getChainNames() -> std::vector<std::string>;

  auto getRules() -> std::vector<const struct ipt_entry *>;

  auto getHandler() -> struct iptc_handle * {
    assert((type_ == FirewallBackendType::TABLE) ||
           (type_ == FirewallBackendType::CHAIN) && handle_ != nullptr);

    return handle_;
  }

  auto init() -> bool override;

private:
  struct iptc_handle *handle_;
  FirewallBackendType type_;
  std::string name_;
};

#endif
