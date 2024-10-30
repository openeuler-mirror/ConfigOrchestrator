#include "frontend/package_manager/package_ui.h"

#include <iostream>

auto PackageManagerConfig::userDisplay(YDialog *main_dialog,
                                       DisplayLayout layout) -> DisplayResult {
  auto *main_layout = layout.feature_layout_;

  cout << "displaying MainMenu with dialog: " << main_dialog
       << " and layout: " << main_layout << endl;

  occupy(main_layout);

  return DisplayResult::SUCCESS;
}

auto PackageManagerConfig::userHandleEvent(YEvent *event) -> HandleResult {
  (void)event;
  return HandleResult::SUCCESS;
}

auto PackageManagerConfig::getPageDescription() const -> string {
  static string componentDescription = R"(Configure package manager settings)";

  return componentDescription;
};

auto PackageManagerConfig::getPageName() const -> string {
  static string componentName = "Package Manager (dnf) Configuration";
  return componentName;
}
