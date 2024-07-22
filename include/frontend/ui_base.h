#ifndef ui_base_H
#define ui_base_H

#include <functional>

#include "YDialog.h"
#include "YEvent.h"
#include "YLabel.h"
#include "YLayoutBox.h"
#include "YUI.h"
#include "YWidgetFactory.h"
#include "backend/config_manager.h"
#include "controlpane.h"
#include <iostream>
#include <memory>
#include <ncurses.h>
#include <stack>
#include <string>
#include <utility>
#include <vector>
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

class UIBase {
public:
  UIBase(std::string name, std::shared_ptr<ConfigManager> manager)
      : name_(std::move(name)), manager_(std::move(manager)) {
    factory_ = YUI::widgetFactory();
  }

  virtual ~UIBase() {
    if (main_dialog_ != nullptr) {
      main_dialog_->destroy();
    }
  }

  auto display() -> std::function<void()>;

  auto handleEvent() -> std::function<void()>;

  [[nodiscard]] virtual auto GetComponentDescription() const -> std::string = 0;

  [[nodiscard]] virtual auto GetComponentName() const -> std::string = 0;

private:
  std::string name_;
  std::shared_ptr<ConfigManager> manager_;

  YWidgetFactory *factory_;
  YDialog *main_dialog_;
  YLayoutBox *main_layout_;

  YLayoutBox *upper_layout_;
  YLayoutBox *control_layout_;
  YLayoutBox *feature_layout_;

  YPushButton *back_button_;
  YPushButton *search_button_;
  YPushButton *close_button_;
  YPushButton *apply_button_;
  YPushButton *help_button_;

  static constexpr YLayoutSize_t kHboxHorMargin = 4;
  static constexpr YLayoutSize_t kHboxVertMargin = 0.1;
  static constexpr YLayoutSize_t kHboxHorMinSize = 10;
  static constexpr YLayoutSize_t kHboxVertMinSize = 1;
  static constexpr YLayoutSize_t kHSpaceLength = 2;
  static constexpr YLayoutSize_t kVSpaceSize = 1;
  static constexpr YLayoutSize_t kPopDialogMinWidth = 60;
  static constexpr YLayoutSize_t kPopDialogMinHeight = 10;

  static const std::string kSoftwareName;
  static const std::string kBackButtonName;
  static const std::string kSearchButtonName;
  static const std::string kCloseButtonName;
  static const std::string kApplyButtonName;
  static const std::string kHelpButtonName;

  virtual auto userDisplay()
      -> std::function<DisplayResult(YDialog *main_dialog,
                                     YLayoutBox *main_layout_)> = 0;

  virtual auto userHandleEvent()
      -> std::function<HandleResult(YEvent *event)> = 0;

  [[nodiscard]] auto handleButtons(YEvent *event) const;

  [[nodiscard]] auto handleHelp() const;

  [[nodiscard]] auto handleExit() const -> bool;
};

#endif // ui_base_H
