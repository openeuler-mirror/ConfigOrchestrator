#include "backend/config_item_base.h"

ConfigItemBase::ConfigItemBase(std::string name) : name(std::move(name)) {}

auto ConfigItemBase::getName() const -> const std::string & { return name; }

void ConfigItemBase::addChild(const std::shared_ptr<ConfigItemBase> &child) {
  children.push_back(child);
}

auto ConfigItemBase::getChildren() const
    -> const std::vector<std::shared_ptr<ConfigItemBase>> & {
  return children;
}
