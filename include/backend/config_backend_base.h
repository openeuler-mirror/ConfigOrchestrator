#ifndef CONFIG_BACKEND_BASE_H
#define CONFIG_BACKEND_BASE_H

#include <memory>
#include <string>
#include <vector>

class ConfigBackendBase {
public:
  ConfigBackendBase(std::string name);
  virtual ~ConfigBackendBase() = default;

private:
};

#endif
