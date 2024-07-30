#include "frontend/main_menu.h"
#include "frontend/firewall/firewall_config.h"
#include "frontend/package_manager/package_config.h"
#include "frontend/ui_base.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

const string MainMenu::FirewallConfigName = "Firewall Config";
const string MainMenu::PackageManagerName = "Package Manager Config";

auto MainMenu::userDisplay()
    -> function<DisplayResult(YDialog *main_dialog, YLayoutBox *main_layout_)> {
  return
      [this](YDialog *main_dialog, YLayoutBox *main_layout_) -> DisplayResult {
        (void)main_dialog;

        auto menus = getMenuConfigs();
        for (const auto &child : menus) {
          auto *button =
              getFactory()->createPushButton(main_layout_, get<0>(child));

          menu_buttons_.emplace_back(button);
        }

        return DisplayResult::SUCCESS;
      };
}

auto MainMenu::userHandleEvent() -> function<HandleResult(YEvent *event)> {
  return [this]([[maybe_unused]] YEvent *event) -> HandleResult {
    for (size_t i = 0; i < menu_buttons_.size(); i++) {
      if (event->widget() == menu_buttons_[i]) {
        auto display = get<1>(getMenuConfigs()[i]);
        display();

        break;
      }
    }

    return HandleResult::SUCCESS;
  };
}

auto MainMenu::getComponentDescription() const -> string {
  static string componentDescription =
      R"(Main Menu of Control Panel of OpenEuler.
        Developed in 2024 OSPP.)";

  return componentDescription;
};

auto MainMenu::getComponentName() const -> string {
  static string componentName = "Main Menu";
  return componentName;
}

auto MainMenu::getMenuConfigs() -> vector<tuple<string, menu_render>> & {
  static vector<tuple<string, menu_render>> r = {
      {FirewallConfigName,
       [this]() {
         auto parent = shared_from_this();
         auto child = make_shared<FirewallConfig>(FirewallConfigName, parent);

         child->display();
         child->handleEvent();
       }},

      {PackageManagerName, [this]() {
         auto parent = shared_from_this();
         auto child =
             make_shared<PackageManagerConfig>(PackageManagerName, parent);

         child->display();
         child->handleEvent();
       }}};

  return r;
}
