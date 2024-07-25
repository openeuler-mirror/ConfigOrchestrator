#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "YPushButton.h"
#include "backend/config_manager.h"
#include "frontend/ui_base.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

class MainMenu : public UIBase {
public:
  MainMenu(const std::string &name) : UIBase(name, nullptr) {}

  ~MainMenu() override = default;

  [[nodiscard]] auto getComponentDescription() const -> std::string override;

  [[nodiscard]] auto getComponentName() const -> std::string override;

  auto init() -> bool override;

private:
  auto userDisplay()
      -> std::function<DisplayResult(YDialog *main_dialog,
                                     YLayoutBox *main_layout_)> override;

  auto userHandleEvent() -> std::function<HandleResult(YEvent *event)> override;

  std::vector<YPushButton *> menu_buttons_;

  static const std::string FirewallConfigName;
  static const std::string PackageManagerName;
};

#endif
