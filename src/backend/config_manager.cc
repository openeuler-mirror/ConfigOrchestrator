#include "backend/config_manager.h"
#include "backend/firewall/firewall_backend.h"
#include "backend/package_manager/package_manager_backend.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

auto ConfigManager::getInitializers(const string &type)
    -> optional<initializer> {
  static unordered_map<string, initializer> initializers = {

  };

  if (initializers.contains(type)) {
    return initializers.at(type);
  }
  return nullopt;
}

auto ConfigManager::registerApplyFunc(const function<bool(void)> &func) -> int {
  unsavedConfigs_.emplace_back(func);
  return static_cast<int>(unsavedConfigs_.size() - 1);
}

auto ConfigManager::apply() -> bool {
  auto res = true;
  for (const auto &func : unsavedConfigs_) {
    res &= func();
  }

  unsavedConfigs_.clear();
  return res;
}
