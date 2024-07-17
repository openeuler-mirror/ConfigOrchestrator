#ifndef CONFIG_FACTORY_H
#define CONFIG_FACTORY_H

#include "config_item_base.h"
#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <vector>

class ConfigFactory {
public:
  using ConfigCreator = std::function<std::shared_ptr<ConfigItemBase>()>;

  static auto instance() -> ConfigFactory & {
    static ConfigFactory factory;
    return factory;
  }

  void registerConfig(const std::string &name, ConfigCreator creator) {
    creators[name] = std::move(creator);
  }

  auto createConfig(const std::string &name)
      -> std::shared_ptr<ConfigItemBase> {
    if (creators.find(name) != creators.end()) {
      return creators[name]();
    }
    return nullptr;
  }

  [[nodiscard]] auto getRegisteredConfigNames() const
      -> std::vector<std::string> {
    std::vector<std::string> names;
    for (const auto &pair : creators) {
      names.push_back(pair.first);
    }
    return names;
  }

private:
  std::map<std::string, ConfigCreator> creators;
};

#endif // CONFIG_FACTORY_H
