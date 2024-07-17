#ifndef config_regsitry_H
#define config_regsitry_H

#include "backend/config_factory.h"
#include "backend/firewall_config.h"
#include "backend/package_config.h"

class ConfigRegistry {
public:
  static void registerConfigs() {
    ConfigFactory::instance().registerConfig(
        "NetworkConfig", []() { return std::make_shared<FirewallConfig>(); });

    ConfigFactory::instance().registerConfig("PackageManagerConfig", []() {
      return std::make_shared<PackageManagerConfig>();
    });
  }

private:
  static struct ConfigInitializer {
    ConfigInitializer() { ConfigRegistry::registerConfigs(); }
  } initializer;
};

#endif // config_regsitry_H