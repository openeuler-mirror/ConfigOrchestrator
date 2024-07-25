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
        std::cout << "displaying MainMenu with dialog: " << main_dialog
                  << " and layout: " << main_layout_ << std::endl;

        occupy(main_layout_);

        return DisplayResult::SUCCESS;
      };
}

auto FirewallConfig::userHandleEvent()
    -> std::function<HandleResult(YEvent *event)> {
  return []([[maybe_unused]] YEvent *event) -> HandleResult {
    return HandleResult{};
  };
}
auto FirewallConfig::getComponentDescription() const -> std::string {
  static std::string componentDescription =
      R"(Configure network firewall settings)";

  return componentDescription;
};

auto FirewallConfig::getComponentName() const -> std::string {
  static std::string componentName = "Network Firewall Configuration";
  return componentName;
}

auto FirewallConfig::init() -> bool {
  if (!isSuperUser()) {
    yuiError() << "Non-root user try to open firewall configuration."
               << std::endl;

    warnDialog(nonSuWarnText);
    return false;
  }

  auto res = true;
  auto backend = std::make_shared<FirewallBackend>(
      getParent().lock()->getBackend().lock(), FirewallBackendType::OVERALL,
      "Firewall Config");

  res &= backend->init();

  setBackend(backend);

  auto tables = backend->getTableNames();
  auto parent = shared_from_this();
  for (const auto &table : tables) {
  }

  return res;
}
