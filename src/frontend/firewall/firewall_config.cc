#include "frontend/firewall/firewall_config.h"
#include "backend/config_manager.h"
#include "frontend/ui_base.h"

#include <memory>
#include <utility>

const string FirewallConfig::nonSuWarnText =
    "Please run the control panel as root to configure firewall.";

FirewallConfig::FirewallConfig(const string &name,
                               const shared_ptr<UIBase> &parent,
                               ctx_t firewall_context)
    : UIBase(name, parent), firewall_context_(std::move(firewall_context)) {
  if (!isSuperUser()) {
    yuiError() << "Non-root user try to open firewall configuration." << endl;
    warnDialog(nonSuWarnText);
  }

  if (firewall_context_ == nullptr) {
    firewall_context_ = std::make_shared<FirewallContext>();
  }

  firewall_backend_ = ConfigManager::instance().getBackend<FirewallBackend>();
  subConfigs_ = firewall_backend_->getSubconfigs(firewall_context_);
};

auto FirewallConfig::userDisplay()
    -> function<DisplayResult(YDialog *main_dialog, YLayoutBox *main_layout_)> {
  return
      [this](YDialog *main_dialog, YLayoutBox *main_layout_) -> DisplayResult {
        (void)main_dialog;

        for (const auto &child : subConfigs_) {
          auto *button = getFactory()->createPushButton(main_layout_, child);
          buttons_.emplace_back(button);
        }

        return DisplayResult::SUCCESS;
      };
}

auto FirewallConfig::userHandleEvent()
    -> function<HandleResult(YEvent *event)> {
  return [this]([[maybe_unused]] YEvent *event) -> HandleResult {
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
  };
}

auto FirewallConfig::getComponentDescription() const -> string {
  static string componentDescription = R"(Configure network firewall settings)";

  return componentDescription;
};

auto FirewallConfig::getComponentName() const -> string {
  static string componentName = "Network Firewall Configuration";

  return getName();
}
