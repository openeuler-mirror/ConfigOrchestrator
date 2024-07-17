// config_manager.h
#ifndef CONFIG_MANAGER_H_
#define CONFIG_MANAGER_H_

#include "config_factory.h"
#include "config_item_base.h"
#include <memory>
#include <vector>

class ConfigManager {
public:
  ConfigManager() { loadAllConfigs(); }

  void addConfig(const std::shared_ptr<ConfigItemBase> &config) {
    configs.push_back(config);
  }

  [[nodiscard]] auto getConfigs() const
      -> const std::vector<std::shared_ptr<ConfigItemBase>> & {
    return configs;
  }

private:
  void loadAllConfigs() {
    std::vector<std::string> configNames =
        ConfigFactory::instance().getRegisteredConfigNames();
    for (const auto &name : configNames) {
      auto config = ConfigFactory::instance().createConfig(name);
      if (config) {
        addConfig(config);
      }
    }
  }

  std::vector<std::shared_ptr<ConfigItemBase>> configs;
};

#endif // CONFIG_MANAGER_H_
