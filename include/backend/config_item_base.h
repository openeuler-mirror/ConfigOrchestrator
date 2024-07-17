#ifndef CONFIG_ITEM_BASE_H
#define CONFIG_ITEM_BASE_H

#include <memory>
#include <string>
#include <vector>

class ConfigItemBase {
public:
  ConfigItemBase(std::string name);
  virtual ~ConfigItemBase() = default;

  virtual void display() const = 0;

  [[nodiscard]] auto getName() const -> const std::string &;
  void addChild(std::shared_ptr<ConfigItemBase> child);
  [[nodiscard]] auto getChildren() const
      -> const std::vector<std::shared_ptr<ConfigItemBase>> &;
  void addChild(const std::shared_ptr<ConfigItemBase> &child);

private:
  std::string name;
  std::vector<std::shared_ptr<ConfigItemBase>> children;
};

#endif