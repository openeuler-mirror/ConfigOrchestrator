#ifndef CONFIG_BACKEND_BASE_H
#define CONFIG_BACKEND_BASE_H

#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

class ConfigBackendBase {
public:
  ConfigBackendBase() = default;
  virtual ~ConfigBackendBase() = default;

private:
};

#endif
