#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "YPushButton.h"
#include "backend/config_manager.h"
#include "frontend/ui_base.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

using std::function;
using std::shared_ptr;
using std::string;
using std::tuple;
using std::vector;

class MainMenu : public UIBase {
public:
  using menu_render = function<void()>;

  MainMenu(const string &name) : UIBase(name, nullptr) {}

  ~MainMenu() override = default;

  [[nodiscard]] auto getPageDescription() const -> string override;

  [[nodiscard]] auto getPageName() const -> string override;

private:
  auto userDisplay(YDialog *main_dialog, DisplayLayout layout)
      -> DisplayResult override;

  auto userHandleEvent(YEvent *event) -> HandleResult override;

  auto getMenuConfigs() -> vector<tuple<string, menu_render>> &;

  vector<YPushButton *> menu_buttons_;

  static const string FirewallConfigName;
  static const string PackageManagerName;
};

#endif
