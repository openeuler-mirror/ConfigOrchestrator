// config_manager.h
#ifndef CONFIG_MANAGER_H_
#define CONFIG_MANAGER_H_

#include "backend/config_backend_base.h"
#include "frontend/ui_base.h"
#include "tools/log.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

using std::function;
using std::nullopt;
using std::optional;
using std::shared_ptr;
using std::string;
using std::type_index;
using std::unordered_map;
using std::vector;

class ConfigManager : public ConfigBackendBase {
public:
  using initializer = function<shared_ptr<ConfigBackendBase>(void)>;

  static auto instance() -> ConfigManager & {
    static ConfigManager config_manager;
    return config_manager;
  }

  template <typename BackendType>
  auto getBackend(const string &id = string()) -> shared_ptr<BackendType> {
    type_index type_index(typeid(BackendType));
    string key = type_index.name() + id;

    if (backends.find(key) == backends.end()) {
      auto initializer_func = getInitializers(type_index.name());

      if (initializer_func.has_value()) {
        backends.insert({key, initializer_func.value()()});
      } else {
        backends.insert({key, std::make_shared<BackendType>()});
      }
    }

    return dynamic_pointer_cast<BackendType>(backends[key]);
  }

  auto hasUnsavedConfig() -> bool { return !unsavedConfigs_.empty(); }

  auto registerApplyFunc(const function<bool(void)> &) -> int;

  auto apply() -> bool;

private:
  vector<function<bool(void)>> unsavedConfigs_;

  // backend manager
  unordered_map<string, shared_ptr<ConfigBackendBase>> backends;

  static auto getInitializers(const string &type) -> optional<initializer>;

  ConfigManager() = default;
};

#endif // CONFIG_MANAGER_H_
