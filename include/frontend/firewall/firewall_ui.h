#ifndef FIREWALL_CONFIG_H
#define FIREWALL_CONFIG_H

#include "YInputField.h"
#include "YWidget.h"
#include "backend/firewall/firewall_backend.h"
#include "backend/firewall/firewall_context.h"
#include "frontend/ui_base.h"

#include "YPushButton.h"
#include <functional>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

using std::function;
using std::optional;
using std::shared_ptr;
using std::string;
using std::tuple;
using std::unique_ptr;
using std::vector;

class FirewallConfig : public UIBase {
public:
  FirewallConfig(const string &name, const shared_ptr<UIBase> &parent,
                 shared_ptr<FirewallContext> context = nullptr);

  ~FirewallConfig() override = default;

  [[nodiscard]] auto getPageDescription() const -> string override;

  [[nodiscard]] auto getPageName() const -> string override;

private:
  auto userDisplay(YDialog *main_dialog,
                   DisplayLayout layout) -> DisplayResult override;

  auto userHandleEvent(YEvent *event) -> HandleResult override;

  auto createUpdateRule(optional<int> index) -> shared_ptr<RuleRequest>;

  auto createChain() -> shared_ptr<ChainRequest>;

  auto fresh(YDialog *main_dialog, DisplayLayout layout) -> bool;

  shared_ptr<FirewallContext> firewall_context_;
  shared_ptr<FirewallBackend> firewall_backend_;
  vector<widget_func_t> widgets_targets_;

  vector<string> iptable_children;

  const static string kNonSuWarnText;
  const static string kAddRuleButtonText;
  const static string kAddChainButtonText;
  const static string kDelRuleButtonText;
  const static string kDelChainButtonText;
};

#endif
