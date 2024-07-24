#ifndef FIREWALL_CONFIG_H
#define FIREWALL_CONFIG_H

#include "backend/firewall/firewall_backend.h"
#include "frontend/ui_base.h"
#include <memory>

class FirewallConfig : public UIBase {
public:
  FirewallConfig(const std::string &name,
                 const std::shared_ptr<ConfigManager> &manager,
                 const std::shared_ptr<UIBase> &parent)
      : UIBase(name, manager, parent, std::make_shared<FirewallBackend>()){};

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
