#ifndef ui_base_H
#define ui_base_H

#include "YDialog.h"
#include "YEvent.h"
#include "YLabel.h"
#include "YLayoutBox.h"
#include "YUI.h"
#include "YWidgetFactory.h"
#include "controlpanel.h"

#include <cstdint>
#include <functional>
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

class ConfigManager;
class ConfigBackendBase;

using config_id_t = uint32_t;

class UIBase : public std::enable_shared_from_this<UIBase> {
public:
  UIBase(std::string name, const std::shared_ptr<UIBase> &parent,
         std::shared_ptr<ConfigBackendBase> backend = nullptr)
      : name_(std::move(name)), parent_(parent), backend_(std::move(backend)),
        main_dialog_(nullptr) {
    factory_ = YUI::widgetFactory();
  }

  virtual ~UIBase() {
    if (main_dialog_ != nullptr) {
      main_dialog_->destroy();
    }
    backend_.reset();
  }

  auto display() -> std::function<void()>;

  auto handleEvent() -> std::function<void()>;

  auto isMainMenu() -> bool;

  auto reset() -> void;

  auto appendChild(const std::shared_ptr<UIBase> &child) -> void;

  auto setBackend(const std::shared_ptr<ConfigBackendBase> &backend) -> void;

  [[nodiscard]] auto getManager() const -> std::shared_ptr<ConfigManager>;

  [[nodiscard]] auto getParent() const -> std::weak_ptr<UIBase>;

  [[nodiscard]] auto getBackend() const -> std::weak_ptr<ConfigBackendBase>;

  [[nodiscard]] auto getChildren() const
      -> std::vector<std::shared_ptr<UIBase>>;

  [[nodiscard]] auto getFactory() const -> YWidgetFactory *;

  [[nodiscard]] virtual auto getComponentDescription() const -> std::string = 0;

  [[nodiscard]] virtual auto getComponentName() const -> std::string = 0;

  virtual auto init() -> bool = 0;

private:
  std::string name_;
  std::weak_ptr<UIBase> parent_;
  std::shared_ptr<ConfigBackendBase> backend_;
  std::vector<std::shared_ptr<UIBase>> children_;

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

  [[nodiscard]] auto handleButtons(YEvent *event) -> HandleResult;

  [[nodiscard]] auto handleHelp() const;

  [[nodiscard]] auto handleExit() const -> bool;
};

#endif // ui_base_H
