#ifndef FIREWALL_CONFIG_H
#define FIREWALL_CONFIG_H

#include "backend/firewall/firewall_backend.h"
#include "frontend/ui_base.h"
#include <memory>

class FirewallConfig : public UIBase {
public:
  FirewallConfig(const std::string &name, const std::shared_ptr<UIBase> &parent)
      : UIBase(name, parent, nullptr){};

  ~FirewallConfig() override = default;

  [[nodiscard]] auto getComponentDescription() const -> std::string override;

  [[nodiscard]] auto getComponentName() const -> std::string override;

  auto init() -> bool override;

private:
  auto userDisplay()
      -> std::function<DisplayResult(YDialog *main_dialog,
                                     YLayoutBox *main_layout_)> override;

  auto userHandleEvent() -> std::function<HandleResult(YEvent *event)> override;

  const static std::string nonSuWarnText;
};

#endif
