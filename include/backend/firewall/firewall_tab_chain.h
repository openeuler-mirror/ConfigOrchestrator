#ifndef FIREWALL_TAB_CHAIN_H
#define FIREWALL_TAB_CHAIN_H

#include "backend/firewall/firewall_backend.h"
#include "frontend/ui_base.h"
#include <memory>
#include <utility>

enum class IPTType { TABLE, CHAIN };

class FirewallTabChain : public ConfigBackendBase {
public:
  FirewallTabChain(const std::shared_ptr<ConfigBackendBase> &parent,
                   IPTType type, std::string name, struct iptc_handle *handle)
      : ConfigBackendBase(parent), type_(type), name_(std::move(name)),
        handle_(handle){};

  auto init() -> bool override;

private:
  IPTType type_;
  std::string name_;
  struct iptc_handle *handle_;
};

#endif