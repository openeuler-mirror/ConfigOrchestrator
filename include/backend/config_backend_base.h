#ifndef CONFIG_BACKEND_BASE_H
#define CONFIG_BACKEND_BASE_H

#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

class ConfigBackendBase
    : public std::enable_shared_from_this<ConfigBackendBase> {
public:
  ConfigBackendBase(const std::shared_ptr<ConfigBackendBase> &parent)
      : parent_(parent){};

  virtual ~ConfigBackendBase() = default;

  virtual auto init() -> bool = 0;

  auto isConfigManager() -> bool { return parent_.expired(); }

private:
  std::weak_ptr<ConfigBackendBase> parent_;
};

#endif
