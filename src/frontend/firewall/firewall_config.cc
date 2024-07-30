#include "frontend/firewall/firewall_config.h"
#include "backend/config_manager.h"
#include "backend/firewall/firewall_context.h"
#include "frontend/ui_base.h"

#include <memory>
#include <sstream>
#include <utility>

using std::stringstream;

const string FirewallConfig::kNonSuWarnText =
    "Please run the control panel as root to configure firewall.";
const string FirewallConfig::kAddRuleButtonText = "&Add Firewall Rule";
const string FirewallConfig::kAddChainButtonText = "&Add Firewall Chain";
const string FirewallConfig::kDelRuleButtonText = "&Delete Firewall Rule";
const string FirewallConfig::kDelChainButtonText = "&Delete Firewall Chain";

FirewallConfig::FirewallConfig(const string &name,
                               const shared_ptr<UIBase> &parent,
                               ctx_t firewall_context)
    : UIBase(name, parent), firewall_context_(std::move(firewall_context)) {
  if (!isSuperUser()) {
    yuiError() << "Non-root user try to open firewall configuration." << endl;
    warnDialog(kNonSuWarnText);
  }

  if (firewall_context_ == nullptr) {
    firewall_context_ = std::make_shared<FirewallContext>();
  }

  firewall_backend_ = ConfigManager::instance().getBackend<FirewallBackend>();
  subConfigs_ = firewall_backend_->getSubconfigs(firewall_context_);
};

auto FirewallConfig::userDisplay(YDialog *main_dialog, DisplayLayout layout)
    -> DisplayResult {
  (void)main_dialog;

  auto *fac = getFactory();
  auto *main_layout = layout.feature_layout_;
  auto *control_layout = layout.user_control_layout_;

  switch (firewall_context_->level_) {
  case FirewallLevel::OVERALL:
    break;
  case FirewallLevel::TABLE:
    add_chain_button_ =
        fac->createPushButton(control_layout, kAddChainButtonText);
    break;
  case FirewallLevel::CHAIN:
    add_rule_button_ =
        fac->createPushButton(control_layout, kAddRuleButtonText);
    del_chain_button_ =
        fac->createPushButton(control_layout, kDelChainButtonText);
    break;
  }

  for (const auto &child : subConfigs_) {
    auto *button = fac->createPushButton(main_layout, child);
    buttons_.emplace_back(button);
  }

  return DisplayResult::SUCCESS;
}

auto FirewallConfig::userControlHandle(YEvent *event) -> HandleResult {
  if (event->widget() == add_chain_button_) {
    auto context = firewall_backend_->createContext(firewall_context_, "");

    auto child = std::make_shared<FirewallConfig>("New Chain",
                                                  shared_from_this(), context);

    child->display();
    child->handleEvent();
  } else if (event->widget() == add_rule_button_) {
    auto context = firewall_backend_->createContext(firewall_context_, "");

    auto child = std::make_shared<FirewallConfig>("New Rule",
                                                  shared_from_this(), context);

    child->display();
    child->handleEvent();
  } else if (event->widget() == del_chain_button_ ||
             event->widget() == del_rule_button_) {
    auto res = firewall_backend_->remove(firewall_context_);
    if (!res) {
      stringstream ss;
      ss << "Failed to remove chain/rule: " << firewall_context_->serialize();

      this->warnDialog(ss.str());
      return HandleResult::SUCCESS;
    }
  } else {
    return HandleResult::CONT;
  }
  return HandleResult::SUCCESS;
}

auto FirewallConfig::userHandleEvent(YEvent *event) -> HandleResult {
  if (auto r = userControlHandle(event); r != HandleResult::CONT) {
    return r;
  }

  for (auto i = 0; i < buttons_.size(); i++) {
    if (event->widget() == buttons_[i]) {
      auto context =
          firewall_backend_->createContext(firewall_context_, subConfigs_[i]);

      auto child = std::make_shared<FirewallConfig>(
          subConfigs_[i], shared_from_this(), context);

      child->display();
      child->handleEvent();

      break;
    }
  }

  return HandleResult::SUCCESS;
}

auto FirewallConfig::getComponentDescription() const -> string {
  static string componentDescription = R"(Configure network firewall settings)";

  return componentDescription;
};

auto FirewallConfig::getComponentName() const -> string {
  static string componentName = "Network Firewall Configuration";

  return getName();
}
