#ifndef package_config_H
#define package_config_H

#include "config_item_base.h"

class PackageManagerConfig : public ConfigItemBase {
public:
  PackageManagerConfig();
  void display() const override;
};

#endif // package_config_H
