// config_manager.h
#ifndef CONFIG_MANAGER_H_
#define CONFIG_MANAGER_H_

#include "frontend/ui_base.h"

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

class ConfigManager {
public:
  ConfigManager() = default;

  auto hasUnsavedConfig() -> bool { return !unsavedConfigs_.empty(); }

private:
  std::vector<std::function<void(void)>> unsavedConfigs_;
};

#endif // CONFIG_MANAGER_H_
