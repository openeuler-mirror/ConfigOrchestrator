#include "frontend/firewall/firewall_config.h"
#include "backend/firewall/firewall_backend.h"
#include <memory>

const std::string FirewallConfig::nonSuWarnText =
    "Please run the control panel as root to configure firewall.";

auto FirewallConfig::userDisplay()
    -> std::function<DisplayResult(YDialog *main_dialog,
                                   YLayoutBox *main_layout_)> {
  return
      [this](YDialog *main_dialog, YLayoutBox *main_layout_) -> DisplayResult {
        (void)main_dialog;

        for (const auto &child : getChildren()) {
          auto *button = getFactory()->createPushButton(
              main_layout_, child->getComponentName());

          buttons_.emplace_back(button);
        }

        return DisplayResult::SUCCESS;
      };
}

auto FirewallConfig::userHandleEvent()
    -> std::function<HandleResult(YEvent *event)> {
  return [this]([[maybe_unused]] YEvent *event) -> HandleResult {
    auto children = getChildren();
    for (auto i = 0; i < buttons_.size(); i++) {
      if (event->widget() == buttons_[i]) {
        auto display = children[i]->display();
        display();

        auto handler = children[i]->handleEvent();
        handler();

        break;
      }
    }
    return HandleResult::SUCCESS;
  };
}
auto FirewallConfig::getComponentDescription() const -> std::string {
  static std::string componentDescription =
      R"(Configure network firewall settings)";

  return componentDescription;
};

auto FirewallConfig::getComponentName() const -> std::string {
  static std::string componentName = "Network Firewall Configuration";

  if (type_ == FirewallBackendType::OVERALL) {
    return componentName;
  }

  return getName();
}

auto FirewallConfig::init() -> bool {
  if (!isSuperUser()) {
    yuiError() << "Non-root user try to open firewall configuration."
               << std::endl;

    warnDialog(nonSuWarnText);
    return false;
  }

  yuiMilestone() << "Init firewall configuration of type "
                 << static_cast<int>(type_) << " name: " << getName()
                 << std::endl;

  this->reset();
  buttons_.clear();

  auto res = true;
  auto backend = std::make_shared<FirewallBackend>(
      getParent().lock()->getBackend().lock(), type_, getComponentName());

  res &= backend->init();
  setBackend(backend);

  if (!res) {
    yuiError() << "Failed to init firewall backend." << std::endl;
    return false;
  }

  auto parent = shared_from_this();
  if (type_ == FirewallBackendType::OVERALL) {
    auto tables = backend->getTableNames();
    for (const auto &table : tables) {
      auto child = std::make_shared<FirewallConfig>(table, parent,
                                                    FirewallBackendType::TABLE);
      appendChild(child);
    }
  } else if (type_ == FirewallBackendType::TABLE) {
    auto chains = backend->getChainNames();
    for (const auto &chain : chains) {
      auto child = std::make_shared<FirewallConfig>(chain, parent,
                                                    FirewallBackendType::CHAIN);
      appendChild(child);
    }
  } else if (type_ == FirewallBackendType::CHAIN) {
    auto rules = backend->getRules();
    for (const auto &rule : rules) {
      auto child = std::make_shared<FirewallConfig>(rule, parent,
                                                    FirewallBackendType::RULE);
      appendChild(child);
    }
  } else {
    res = false;
  }

  return res;
}
