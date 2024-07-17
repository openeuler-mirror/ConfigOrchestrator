#ifndef FIREWALL_CONFIG_H
#define FIREWALL_CONFIG_H

#include "config_item_base.h"

class FirewallConfig : public ConfigItemBase {
public:
  FirewallConfig();
  void display() const override;
};

#endif
