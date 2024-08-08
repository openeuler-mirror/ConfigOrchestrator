#ifndef ui_base_H
#define ui_base_H

#include "controlpanel.h"
#include "tools/uitools.h"

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <ncurses.h>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "YDialog.h"
#include "YEvent.h"
#include "YLabel.h"
#include "YLayoutBox.h"
#include "YUI.h"
#include "YWidgetFactory.h"
#include "tools/widget_manager.h"
#include <yui/YAlignment.h>
#include <yui/YApplication.h>
#include <yui/YDialog.h>
#include <yui/YEvent.h>
#include <yui/YFrame.h>
#include <yui/YLayoutBox.h>
#include <yui/YMenuBar.h>
#include <yui/YMenuButton.h>
#include <yui/YMenuItem.h>
#include <yui/YPushButton.h>
#include <yui/YUI.h>
#include <yui/YWidgetFactory.h>

using std::function;
using std::shared_ptr;
using std::string;
using std::tuple;
using std::weak_ptr;

class DisplayLayout {
public:
  YLayoutBox *feature_layout_;
  YLayoutBox *user_control_layout_;
};

class UIBase : public std::enable_shared_from_this<UIBase> {
public:
  UIBase(string name, const shared_ptr<UIBase> &parent)
      : name_(std::move(name)), parent_(parent), main_dialog_(nullptr) {
    factory_ = YUI::widgetFactory();
  }

  virtual ~UIBase() {
    if (main_dialog_ != nullptr) {
      main_dialog_->destroy();
    }
  }

  auto display() -> void;

  auto handleEvent() -> void;

  auto isMainMenu() -> bool;

  auto getName() const -> string;

  [[nodiscard]] auto getParent() const -> weak_ptr<UIBase>;

  [[nodiscard]] auto getFactory() const -> YWidgetFactory *;

  /**
   * @brief Get the page desc, shown in help dialog
   */
  [[nodiscard]] virtual auto getPageDescription() const -> string = 0;

  /**
   * @brief Get the page name, shown in title
   */
  [[nodiscard]] virtual auto getPageName() const -> string = 0;

protected:
  using widget_func_t = tuple<YWidget *, function<bool()>>;

private:
  string name_;
  weak_ptr<UIBase> parent_;

  YWidgetFactory *factory_;
  YDialog *main_dialog_;
  YLayoutBox *main_layout_;

  YLayoutBox *upper_layout_;
  YLayoutBox *global_control_layout_;
  YLayoutBox *feature_layout_;
  YLayoutBox *user_control_layout_;

  WidgetManager widget_manager_;

  static constexpr YLayoutSize_t kHboxHorMargin = 4;
  static constexpr YLayoutSize_t kHboxVertMargin = 0.1;
  static constexpr YLayoutSize_t kHboxHorMinSize = 10;
  static constexpr YLayoutSize_t kHboxVertMinSize = 1;
  static constexpr YLayoutSize_t kHSpaceLength = 2;
  static constexpr YLayoutSize_t kVSpaceSize = 1;

  static const string kSoftwareName;
  static const string kBackButtonName;
  static const string kSearchButtonName;
  static const string kCloseButtonName;
  static const string kApplyButtonName;
  static const string kHelpButtonName;

  virtual auto userDisplay(YDialog *main_dialog,
                           DisplayLayout layout) -> DisplayResult = 0;

  virtual auto userHandleEvent(YEvent *event) -> HandleResult = 0;

  [[nodiscard]] auto handleButtons(YEvent *event) -> HandleResult;

  [[nodiscard]] auto handleHelp() const;

  [[nodiscard]] auto handleExit() const -> bool;
};

#endif // ui_base_H
