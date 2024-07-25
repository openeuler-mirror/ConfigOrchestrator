// config_manager.h
#ifndef CONFIG_MANAGER_H_
#define CONFIG_MANAGER_H_

#include "backend/config_backend_base.h"
#include "frontend/ui_base.h"

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

class ConfigManager : public ConfigBackendBase {
public:
  ConfigManager(const std::shared_ptr<ConfigBackendBase> &parent)
      : ConfigBackendBase(parent){};

  auto hasUnsavedConfig() -> bool { return !unsavedConfigs_.empty(); }

  auto init() -> bool override;

private:
  std::vector<std::function<void(void)>> unsavedConfigs_;
};

#endif // CONFIG_MANAGER_H_
