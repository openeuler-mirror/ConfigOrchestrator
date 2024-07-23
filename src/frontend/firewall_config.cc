#include "frontend/firewall_config.h"

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
auto FirewallConfig::GetComponentDescription() const -> std::string {
  static std::string componentDescription =
      R"(Configure network firewall settings)";

  return componentDescription;
};

auto FirewallConfig::GetComponentName() const -> std::string {
  static std::string componentName = "Network Firewall Configuration";
  return componentName;
}

auto FirewallConfig::GetChildrenInitializer() const
    -> std::vector<std::function<
        std::shared_ptr<UIBase>(const std::shared_ptr<ConfigManager> &manager,
                                const std::shared_ptr<UIBase> &parent)>> {
  return {};
}
