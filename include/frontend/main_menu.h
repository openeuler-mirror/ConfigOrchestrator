#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "backend/config_manager.h"
#include "frontend/ui_base.h"

#include <memory>
#include <utility>

class MainMenu : public UIBase {
public:
  MainMenu(const std::string &name,
           const std::shared_ptr<ConfigManager> &manager)
      : UIBase(name, manager, nullptr) {}

  ~MainMenu() override = default;

  [[nodiscard]] auto GetComponentDescription() const -> std::string override;

  [[nodiscard]] auto GetComponentName() const -> std::string override;

  [[nodiscard]] auto GetChildrenInitializer() const
      -> std::vector<std::function<std::shared_ptr<UIBase>(
          const std::shared_ptr<ConfigManager> &manager,
          const std::shared_ptr<UIBase> &parent)>> override;

private:
  auto userDisplay()
      -> std::function<DisplayResult(YDialog *main_dialog,
                                     YLayoutBox *main_layout_)> override;

  auto userHandleEvent() -> std::function<HandleResult(YEvent *event)> override;
};

#endif
