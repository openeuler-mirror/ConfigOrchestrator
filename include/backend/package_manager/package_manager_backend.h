#include "backend/config_backend_base.h"

class PackageManagerBackend : public ConfigBackendBase {
public:
  PackageManagerBackend(const std::shared_ptr<ConfigBackendBase> &parent)
      : ConfigBackendBase(parent){};

  auto init() -> bool override;

private:
};
