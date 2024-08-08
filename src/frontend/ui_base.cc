
#include "YDialog.h"
#include "YLabel.h"
#include "YTypes.h"

#include "backend/config_manager.h"
#include "controlpanel.h"
#include "frontend/ui_base.h"
#include "tools/log.h"
#include "tools/uitools.h"

#include <cassert>
#include <cstdlib>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

const string UIBase::kSoftwareName = "Control Panel";
const string UIBase::kBackButtonName = "&Back";
const string UIBase::kSearchButtonName = "&Search";
const string UIBase::kCloseButtonName = "&Close";
const string UIBase::kApplyButtonName = "&Apply";
const string UIBase::kHelpButtonName = "&Help";

static auto createMinsizeMarginBox(YWidgetFactory *fac, YWidget *parent) {
  static constexpr YLayoutSize_t kHboxHorMargin = 4;
  static constexpr YLayoutSize_t kHboxVertMargin = 0.1;
  static constexpr YLayoutSize_t kHboxHorMinSize = 10;
  static constexpr YLayoutSize_t kHboxVertMinSize = 1;

  auto *mbox = fac->createMarginBox(parent, kHboxHorMargin, kHboxVertMargin);
  return fac->createMinSize(mbox, kHboxHorMinSize, kHboxVertMinSize);
}

auto UIBase::display() -> void {
  main_dialog_ = factory_->createDialog(YDialogType::YMainDialog);
  main_layout_ = factory_->createVBox(main_dialog_);
  main_layout_->setStretchable(YUIDimension::YD_HORIZ, true);
  main_layout_->setStretchable(YUIDimension::YD_VERT, true);

  {
    auto *msize = createMinsizeMarginBox(factory_, main_layout_);
    upper_layout_ = factory_->createHBox(msize);

    const auto &title = getPageName();
    factory_->createLabel(upper_layout_, title);
  }

  factory_->createVSpacing(main_layout_, kVSpaceSize);

  {
    auto *msize = createMinsizeMarginBox(factory_, main_layout_);

    global_control_layout_ = factory_->createHBox(msize);
    auto *gcl = global_control_layout_; /* alias */

    {
      auto *back_button_ = factory_->createPushButton(gcl, kBackButtonName);
      widget_manager_.addWidget(back_button_, [this]() {
        if (isMainMenu()) {
          return HandleResult::SUCCESS; /* close is required */
        }

        main_dialog_->destroy();
        main_dialog_ = nullptr;
        return HandleResult::BREAK; /* break handler */
      });
    }

    {
      auto *search_button = factory_->createPushButton(gcl, kSearchButtonName);
      widget_manager_.addWidget(search_button, [this]() {
        showDialog(dialog_meta::ERROR, "Search is not implemented yet.");
        return HandleResult::SUCCESS;
      });
    }

    {
      /* merge with cancel handling */
      auto *close_button = factory_->createPushButton(gcl, kCloseButtonName);
      close_button->setRole(YButtonRole::YCancelButton);
      widget_manager_.addWidget(close_button, [this]() {
        if (!ConfigManager::instance().hasUnsavedConfig() || checkExit()) {
          YDialog::deleteAllDialogs(); /* check unsaved configs */
          return HandleResult::EXIT;
        }
        return HandleResult::SUCCESS;
      });
    }

    {
      auto *apply_button = factory_->createPushButton(gcl, kApplyButtonName);
      widget_manager_.addWidget(apply_button, [this]() {
        static const string fail_msg = "Failed to apply all changes.";
        static const string succ_msg = "Successfully applied changes.";

        if (!ConfigManager::instance().apply()) {
          showDialog(dialog_meta::ERROR, fail_msg);
        } else {
          showDialog(dialog_meta::INFO, succ_msg);
        }
        return HandleResult::SUCCESS;
      });

      apply_button->setRole(YButtonRole::YApplyButton);
    }

    {
      auto *help_button = factory_->createPushButton(gcl, kHelpButtonName);
      widget_manager_.addWidget(help_button, [this]() {
        showDialog(dialog_meta::HELP, getPageDescription());
        return HandleResult::SUCCESS;
      });
      help_button->setRole(YButtonRole::YHelpButton);
    }
  }

  factory_->createVSpacing(main_layout_, kVSpaceSize);

  {
    auto *msize = createMinsizeMarginBox(factory_, main_layout_);
    feature_layout_ = factory_->createVBox(msize);
  }

  factory_->createVSpacing(main_layout_, kVSpaceSize);

  {
    auto *msize = createMinsizeMarginBox(factory_, main_layout_);
    user_control_layout_ = factory_->createHBox(msize);
  }

  userDisplay(main_dialog_, {feature_layout_, user_control_layout_});
}

auto UIBase::checkExit() const -> bool {
  const static string msg = "There are unsaved changes. Do you want to exit?";

  YWidgetFactory *fac = YUI::widgetFactory();
  YDialog *warn_dialog = fac->createPopupDialog();

  YLayoutBox *vbox = fac->createVBox(warn_dialog);
  YAlignment *minSize = fac->createMinSize(
      vbox, dialog_meta::kPopDialogMinWidth, dialog_meta::kPopDialogMinHeight);
  YLabel *label = fac->createOutputField(minSize, getPageDescription());
  label->setAutoWrap();

  auto *exit_button = fac->createPushButton(vbox, "Exit");
  auto *cancel_button = fac->createPushButton(vbox, "Cancel");

  auto *event = warn_dialog->waitForEvent();
  while (event->widget() != exit_button && event->widget() != cancel_button &&
         event->eventType() != YEvent::CancelEvent) {
    event = warn_dialog->waitForEvent();
  }

  auto res = event->widget() == exit_button;
  warn_dialog->destroy();
  return res;
}

auto UIBase::handleEvent() -> void {
  if (main_dialog_ == nullptr) {
    auto msg = fmt::format("main_dialog is nullptr when handling event\n");
    yuiError() << msg;
    return;
  }

  while (true) {
    auto *event = main_dialog_->waitForEvent();
    if (event == nullptr) {
      yuiError() << "event is nullptr when return from waiting" << endl;
      std::terminate();
    }

    if (event->eventType() == YEvent::CancelEvent) {
      if (!ConfigManager::instance().hasUnsavedConfig() || checkExit()) {
        YDialog::deleteAllDialogs(); /* check unsaved configs */
        exit(0);
      }
    }

    using handlers_t = vector<function<HandleResult()>>;
    handlers_t handlers = {
        {[this, event]() { return widget_manager_.handleEvent(event); }},
        [this, event]() { return userHandleEvent(event); },
    };

    HandleResult result = HandleResult::CONT;
    for (const auto &handler : handlers) {
      result = handler();
      if (result != HandleResult::CONT) {
        break;
      }
    }
    if (result == HandleResult::BREAK) {
      break;
    }
    if (result == HandleResult::EXIT) {
      exit(0);
    }
  }
}

[[nodiscard]] auto UIBase::getParent() const -> weak_ptr<UIBase> {
  return parent_;
}

[[nodiscard]] auto UIBase::getFactory() const -> YWidgetFactory * {
  return factory_;
}

auto UIBase::isMainMenu() -> bool { return parent_.expired(); }

auto UIBase::getName() const -> string { return name_; }
