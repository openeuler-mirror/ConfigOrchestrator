#include "frontend/main_menu.h"
#include "frontend/firewall_config.h"
#include "frontend/package_config.h"

#include <iostream>
#include <string>

auto MainMenu::userDisplay()
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

auto MainMenu::userHandleEvent() -> std::function<HandleResult(YEvent *event)> {
  return []([[maybe_unused]] YEvent *event) -> HandleResult {
    return HandleResult{};
  };
}

auto MainMenu::GetComponentDescription() const -> std::string {
  static std::string componentDescription =
      R"(Main Menu of Control Panel of OpenEuler.
        Developed in 2024 OSPP.)";

  return componentDescription;
};

auto MainMenu::GetComponentName() const -> std::string {
  static std::string componentName = "Main Menu";
  return componentName;
}

auto MainMenu::GetChildrenInitializer() const -> std::vector<std::function<
    std::shared_ptr<UIBase>(const std::shared_ptr<ConfigManager> &manager,
                            const std::shared_ptr<UIBase> &parent)>> {
  return {
      [name = "Network Config"](const std::shared_ptr<ConfigManager> &manager,
                                const std::shared_ptr<UIBase> &parent) {
        return std::make_shared<FirewallConfig>(name, manager, parent);
      },
      [name = "Package Manager Config"](
          const std::shared_ptr<ConfigManager> &manager,
          const std::shared_ptr<UIBase> &parent) {
        return std::make_shared<PackageManagerConfig>(name, manager, parent);
      }};
}
