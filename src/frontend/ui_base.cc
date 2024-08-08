
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

auto UIBase::display() -> void {
  main_dialog_ = factory_->createDialog(YDialogType::YMainDialog);
  main_layout_ = factory_->createVBox(main_dialog_);
  main_layout_->setStretchable(YUIDimension::YD_HORIZ, true);
  main_layout_->setStretchable(YUIDimension::YD_VERT, true);

  {
    YAlignment *mbox = factory_->createMarginBox(main_layout_, kHboxHorMargin,
                                                 kHboxVertMargin);
    YAlignment *min_size =
        factory_->createMinSize(mbox, kHboxHorMinSize, kHboxVertMinSize);

    upper_layout_ = factory_->createHBox(min_size);
    factory_->createLabel(upper_layout_, name_);
  }

  factory_->createVSpacing(main_layout_, kVSpaceSize);

  {
    YAlignment *mbox = factory_->createMarginBox(main_layout_, kHboxHorMargin,
                                                 kHboxVertMargin);
    YAlignment *min_size =
        factory_->createMinSize(mbox, kHboxHorMinSize, kHboxVertMinSize);

    global_control_layout_ = factory_->createHBox(min_size);

    back_button_ =
        factory_->createPushButton(global_control_layout_, kBackButtonName);
    search_button_ =
        factory_->createPushButton(global_control_layout_, kSearchButtonName);
    close_button_ =
        factory_->createPushButton(global_control_layout_, kCloseButtonName);
    apply_button_ =
        factory_->createPushButton(global_control_layout_, kApplyButtonName);
    help_button_ =
        factory_->createPushButton(global_control_layout_, kHelpButtonName);

    close_button_->setRole(YButtonRole::YCancelButton);
    apply_button_->setRole(YButtonRole::YApplyButton);
    help_button_->setRole(YButtonRole::YHelpButton);
  }

  factory_->createVSpacing(main_layout_, kVSpaceSize);

  {
    YAlignment *mbox = factory_->createMarginBox(main_layout_, kHboxHorMargin,
                                                 kHboxVertMargin);
    YAlignment *min_size =
        factory_->createMinSize(mbox, kHboxHorMinSize, kHboxVertMinSize);

    feature_layout_ = factory_->createVBox(min_size);
  }

  factory_->createVSpacing(main_layout_, kVSpaceSize);

  {
    YAlignment *mbox = factory_->createMarginBox(main_layout_, kHboxHorMargin,
                                                 kHboxVertMargin);
    YAlignment *min_size =
        factory_->createMinSize(mbox, kHboxHorMinSize, kHboxVertMinSize);

    user_control_layout_ = factory_->createHBox(min_size);
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
  if (event->widget() == close_button_ ||
      event->eventType() == YEvent::CancelEvent) {
    auto real_exit = true;
    if (ConfigManager::instance().hasUnsavedConfig()) {
      real_exit = handleExit();
    }

    if (real_exit) {
      YDialog::deleteAllDialogs();
      return HandleResult::EXIT;
    }
  }

  if (event->widget() == help_button_) {
    showDialog(dialog_meta::HELP, getPageDescription());
  }

  if (event->widget() == back_button_) {
    assert(main_dialog_ != nullptr);

    main_dialog_->destroy();
    main_dialog_ = nullptr;
    return HandleResult::BREAK;
  }

  if (event->widget() == search_button_) {
    YUIUnImpl("Search Button");
  }
  if (event->widget() == apply_button_) {
    if (!ConfigManager::instance().apply()) {
      showDialog(dialog_meta::ERROR, "Failed to apply all changes.");
    } else {
      showDialog(dialog_meta::INFO, "Successfully applied changes.");
    }
  }

  return HandleResult::SUCCESS;
}

auto UIBase::handleEvent() -> void {
  if (main_dialog_ == nullptr) {
    yuiError() << "main_dialog is nullptr when handling event" << endl;
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
