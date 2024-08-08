
#include "YDialog.h"
#include "YLabel.h"
#include "YTypes.h"

#include "backend/config_manager.h"
#include "frontend/ui_base.h"
#include "tools/cplog.h"
#include "tools/uitools.h"

#include <cassert>
#include <cstdlib>
#include <exception>
#include <memory>
#include <string>
#include <type_traits>

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
          return false; /* close is required */
        }

        main_dialog_->destroy();
        main_dialog_ = nullptr;
        return true;
      });
    }

    {
      auto *search_button = factory_->createPushButton(gcl, kSearchButtonName);
      widget_manager_.addWidget(search_button, [this]() {
        showDialog(dialog_meta::ERROR, "Search is not implemented yet.");
        return true;
      });
    }

    {
      /* merge with cancel handling */
      auto *close_button = factory_->createPushButton(gcl, kCloseButtonName);
      close_button->setRole(YButtonRole::YCancelButton);
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
        return true;
      });

      apply_button->setRole(YButtonRole::YApplyButton);
    }

    {
      auto *help_button = factory_->createPushButton(gcl, kHelpButtonName);
      widget_manager_.addWidget(help_button, [this]() {
        showDialog(dialog_meta::HELP, getPageDescription());
        return true;
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

auto UIBase::handleExit() const -> bool {
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

auto UIBase::handleButtons(YEvent *event) -> HandleResult {
  if (event->eventType() == YEvent::CancelEvent) {
    auto real_exit = true; /* check unsaved configs */
    if (ConfigManager::instance().hasUnsavedConfig()) {
      real_exit = handleExit();
    }

    if (real_exit) {
      YDialog::deleteAllDialogs();
      return HandleResult::EXIT;
    }
  }

  return HandleResult::SUCCESS;
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

    auto res = handleButtons(event);
    if (res == HandleResult::EXIT) {
      exit(0);
    } else if (res == HandleResult::BREAK) {
      break;
    }

    auto user_result = userHandleEvent(event);
    if (user_result == HandleResult::EXIT) {
      break;
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
