#ifndef FIREWALL_CONFIG_H
#define FIREWALL_CONFIG_H

#include "ui_base.h"

class FirewallConfig : public UIBase {
public:
  FirewallConfig(const std::string &name,
                 const std::shared_ptr<ConfigManager> &manager,
                 const std::shared_ptr<UIBase> &parent)
      : UIBase(name, manager, parent){};

  ~FirewallConfig() override = default;

  [[nodiscard]] auto GetComponentDescription() const -> std::string override;

  [[nodiscard]] auto GetComponentName() const -> std::string override;

private:
  auto userDisplay()
      -> std::function<DisplayResult(YDialog *main_dialog,
                                     YLayoutBox *main_layout_)> override;

  auto userHandleEvent() -> std::function<HandleResult(YEvent *event)> override;

  [[nodiscard]] auto GetChildrenInitializer() const
      -> std::vector<std::function<std::shared_ptr<UIBase>(
          const std::shared_ptr<ConfigManager> &manager,
          const std::shared_ptr<UIBase> &parent)>> override;
};

#endif
