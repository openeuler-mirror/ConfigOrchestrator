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
