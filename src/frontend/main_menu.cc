#include "frontend/main_menu.h"
#include "frontend/firewall_config.h"
#include "frontend/package_config.h"

#include <cstddef>
#include <iostream>
#include <string>

auto MainMenu::userDisplay()
    -> std::function<DisplayResult(YDialog *main_dialog,
                                   YLayoutBox *main_layout_)> {
  return
      [this](YDialog *main_dialog, YLayoutBox *main_layout_) -> DisplayResult {
        (void)main_dialog;

        for (const auto &child : GetChildren()) {
          auto *button = GetFactory()->createPushButton(
              main_layout_, child->GetComponentName());

          menu_buttons_.emplace_back(button);
        }

        return DisplayResult::SUCCESS;
      };
}

auto MainMenu::userHandleEvent() -> std::function<HandleResult(YEvent *event)> {
  return [this]([[maybe_unused]] YEvent *event) -> HandleResult {
    for (size_t i = 0; i < menu_buttons_.size(); i++) {
      if (event->widget() == menu_buttons_[i]) {
        auto display = GetChildren()[i]->display();
        display();

        auto handler = GetChildren()[i]->handleEvent();
        handler();

        break;
      }
    }
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
