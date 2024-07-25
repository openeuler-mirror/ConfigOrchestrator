#include "frontend/main_menu.h"
#include "frontend/firewall/firewall_config.h"
#include "frontend/package_manager/package_config.h"
#include "frontend/ui_base.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

const std::string MainMenu::FirewallConfigName = "Firewall Config";
const std::string MainMenu::PackageManagerName = "Package Manager Config";

auto MainMenu::userDisplay()
    -> std::function<DisplayResult(YDialog *main_dialog,
                                   YLayoutBox *main_layout_)> {
  return
      [this](YDialog *main_dialog, YLayoutBox *main_layout_) -> DisplayResult {
        (void)main_dialog;

        for (const auto &child : getChildren()) {
          auto *button = getFactory()->createPushButton(
              main_layout_, child->getComponentName());

          menu_buttons_.emplace_back(button);
        }

        return DisplayResult::SUCCESS;
      };
}

auto MainMenu::userHandleEvent() -> std::function<HandleResult(YEvent *event)> {
  return [this]([[maybe_unused]] YEvent *event) -> HandleResult {
    for (size_t i = 0; i < menu_buttons_.size(); i++) {
      if (event->widget() == menu_buttons_[i]) {
        auto display = getChildren()[i]->display();
        display();

        auto handler = getChildren()[i]->handleEvent();
        handler();

        break;
      }
    }
    return HandleResult{};
  };
}

auto MainMenu::getComponentDescription() const -> std::string {
  static std::string componentDescription =
      R"(Main Menu of Control Panel of OpenEuler.
        Developed in 2024 OSPP.)";

  return componentDescription;
};

auto MainMenu::getComponentName() const -> std::string {
  static std::string componentName = "Main Menu";
  return componentName;
}

auto MainMenu::init() -> bool {
  {
    auto parent = shared_from_this();
    {
      auto child = std::make_shared<FirewallConfig>(FirewallConfigName, parent);
      appendChild(child);
    }

    {
      auto child =
          std::make_shared<PackageManagerConfig>(PackageManagerName, parent);
      appendChild(child);
    }
  }

  {
    auto backend = std::make_shared<ConfigManager>(nullptr);
    backend->init();

    setBackend(backend);
  }

  return true;
}
