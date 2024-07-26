#ifndef FIREWALL_CONFIG_H
#define FIREWALL_CONFIG_H

#include "YPushButton.h"
#include "backend/firewall/firewall_backend.h"
#include "frontend/ui_base.h"
#include <memory>
#include <vector>

class FirewallConfig : public UIBase {
public:
  FirewallConfig(const std::string &name, const std::shared_ptr<UIBase> &parent,
                 FirewallBackendType type = FirewallBackendType::OVERALL)
      : UIBase(name, parent, nullptr), type_(type){};

  ~FirewallConfig() override = default;

  [[nodiscard]] auto getComponentDescription() const -> std::string override;

  [[nodiscard]] auto getComponentName() const -> std::string override;

  auto init() -> bool override;

private:
  auto userDisplay()
      -> std::function<DisplayResult(YDialog *main_dialog,
                                     YLayoutBox *main_layout_)> override;

  auto userHandleEvent() -> std::function<HandleResult(YEvent *event)> override;

  FirewallBackendType type_;
  std::vector<YPushButton *> buttons_;

  const static std::string nonSuWarnText;
};

#endif
