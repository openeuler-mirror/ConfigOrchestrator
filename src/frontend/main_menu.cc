#include "frontend/main_menu.h"
#include <iostream>
#include <string>

static void occupy(YLayoutBox *layout) {
  [[maybe_unused]] YLabel *label =
      YUI::widgetFactory()->createOutputField(layout, "OCCPUATION");
}

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
