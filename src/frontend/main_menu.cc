#include "frontend/main_menu.h"
#include "frontend/firewall/firewall_ui.h"
#include "frontend/package_manager/package_ui.h"
#include "frontend/ui_base.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

const string MainMenu::FirewallConfigName = "Firewall Config";
const string MainMenu::PackageManagerName = "Package Manager Config";

auto MainMenu::userDisplay(YDialog *main_dialog,
                           DisplayLayout layout) -> DisplayResult {
  (void)main_dialog;

  auto menus = getMenuConfigs();
  auto *main_layout = layout.feature_layout_;

  for (const auto &child : menus) {
    auto *button = getFactory()->createPushButton(main_layout, get<0>(child));

    menu_buttons_.emplace_back(button);
  }

  return DisplayResult::SUCCESS;
}

auto MainMenu::userHandleEvent(YEvent *event) -> HandleResult {
  for (size_t i = 0; i < menu_buttons_.size(); i++) {
    if (event->widget() == menu_buttons_[i]) {
      auto display = get<1>(getMenuConfigs()[i]);
      display();

      break;
    }
  }

  return HandleResult::SUCCESS;
}

auto MainMenu::getPageDescription() const -> string {
  static string componentDescription =
      R"(Main Menu of Control Panel of OpenEuler.
        Developed in 2024 OSPP.)";

  return componentDescription;
};

auto MainMenu::getPageName() const -> string {
  static string componentName = "Main Menu";
  return componentName;
}

auto MainMenu::getMenuConfigs() -> vector<tuple<string, menu_render>> & {
  static vector<tuple<string, menu_render>> configs = {
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

  return configs;
}
