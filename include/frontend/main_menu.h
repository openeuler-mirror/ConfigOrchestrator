#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "frontend/ui_base.h"
#include <utility>

class MainMenu : public UIBase {
public:
  MainMenu(const std::string &name, ConfigManager manager)
      : UIBase(name, std::move(manager)) {
    const auto &configs = manager_.getConfigs();
  }

  ~MainMenu() override = default;

  [[nodiscard]] auto GetComponentDescription() const -> std::string override;

  [[nodiscard]] auto GetComponentName() const -> std::string override;

private:
  auto userDisplay()
      -> std::function<DisplayResult(YDialog *main_dialog,
                                     YLayoutBox *main_layout_)> override;

  auto userHandleEvent() -> std::function<HandleResult(YEvent *event)> override;
};

#endif
