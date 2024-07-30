#include "frontend/package_manager/package_config.h"

#include <iostream>

auto PackageManagerConfig::userDisplay(YDialog *main_dialog,
                                       DisplayLayout layout) -> DisplayResult {
  auto *main_layout = layout.feature_layout_;

  std::cout << "displaying MainMenu with dialog: " << main_dialog
            << " and layout: " << main_layout << std::endl;

  occupy(main_layout);

  return DisplayResult::SUCCESS;
}

auto PackageManagerConfig::userHandleEvent(YEvent *event) -> HandleResult {
  (void)event;
  return HandleResult::SUCCESS;
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
