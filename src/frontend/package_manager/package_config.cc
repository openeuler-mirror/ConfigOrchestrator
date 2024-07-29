#include "frontend/package_manager/package_config.h"

#include <iostream>

auto PackageManagerConfig::userDisplay()
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

auto PackageManagerConfig::userHandleEvent()
    -> std::function<HandleResult(YEvent *event)> {
  return []([[maybe_unused]] YEvent *event) -> HandleResult {
    return HandleResult{};
  };
}
auto PackageManagerConfig::getComponentDescription() const -> std::string {
  static std::string componentDescription =
      R"(Configure package manager settings)";

  return componentDescription;
};

auto PackageManagerConfig::getComponentName() const -> std::string {
  static std::string componentName = "Package Manager (dnf) Configuration";
  return componentName;
}
