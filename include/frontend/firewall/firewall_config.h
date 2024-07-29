#ifndef FIREWALL_CONFIG_H
#define FIREWALL_CONFIG_H

#include "backend/firewall/firewall_backend.h"
#include "backend/firewall/firewall_context.h"
#include "frontend/ui_base.h"

#include "YPushButton.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

using std::function;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

class FirewallConfig : public UIBase {
public:
  FirewallConfig(const string &name, const shared_ptr<UIBase> &parent,
                 shared_ptr<FirewallContext> context = nullptr);

  ~FirewallConfig() override = default;

  [[nodiscard]] auto getComponentDescription() const -> string override;

  [[nodiscard]] auto getComponentName() const -> string override;

private:
  auto userDisplay()
      -> function<DisplayResult(YDialog *main_dialog,
                                YLayoutBox *main_layout_)> override;

  auto userHandleEvent() -> function<HandleResult(YEvent *event)> override;

  shared_ptr<FirewallContext> firewall_context_;
  shared_ptr<FirewallBackend> firewall_backend_;

  vector<string> subConfigs_;
  vector<YPushButton *> buttons_;

  const static string nonSuWarnText;
};

#endif
