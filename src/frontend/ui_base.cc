
#include "YDialog.h"
#include "YLabel.h"
#include "YTypes.h"

#include "backend/config_manager.h"
#include "frontend/ui_base.h"
#include "tools/cplog.h"

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
const string UIBase::kWarnDialogTitle = "Warning";

auto UIBase::display() -> void {
  auto user_displayer = userDisplay();
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

    control_layout_ = factory_->createHBox(min_size);

    back_button_ = factory_->createPushButton(control_layout_, kBackButtonName);
    search_button_ =
        factory_->createPushButton(control_layout_, kSearchButtonName);
    close_button_ =
        factory_->createPushButton(control_layout_, kCloseButtonName);
    apply_button_ =
        factory_->createPushButton(control_layout_, kApplyButtonName);
    help_button_ = factory_->createPushButton(control_layout_, kHelpButtonName);

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
    user_displayer(main_dialog_, feature_layout_);
  }
}

auto UIBase::handleHelp() const {
  YWidgetFactory *fac = YUI::widgetFactory();
  YDialog *dialog = fac->createPopupDialog();

  YLayoutBox *vbox = fac->createVBox(dialog);
  YAlignment *minSize =
      fac->createMinSize(vbox, kPopDialogMinWidth, kPopDialogMinHeight);
  YLabel *label = fac->createOutputField(minSize, getComponentDescription());
  label->setAutoWrap();

  fac->createPushButton(vbox, "OK");
  dialog->waitForEvent();
  dialog->destroy();
}

auto UIBase::handleExit() const -> bool {
  const static string msg = "There are unsaved changes. Do you want to exit?";
  YWidgetFactory *fac = YUI::widgetFactory();
  YDialog *dialog = fac->createPopupDialog();

  YLayoutBox *vbox = fac->createVBox(dialog);
  YAlignment *minSize =
      fac->createMinSize(vbox, kPopDialogMinWidth, kPopDialogMinHeight);
  YLabel *label = fac->createOutputField(minSize, getComponentDescription());
  label->setAutoWrap();

  auto *exit_button = fac->createPushButton(vbox, "Exit");
  auto *cancel_button = fac->createPushButton(vbox, "Cancel");

  auto *event = dialog->waitForEvent();
  while (event->widget() != exit_button && event->widget() != cancel_button) {
    event = dialog->waitForEvent();
  }

  dialog->destroy();
  return event->widget() == exit_button;
}

auto UIBase::handleButtons(YEvent *event) -> HandleResult {
  if (event->widget() == close_button_ ||
      event->eventType() == YEvent::CancelEvent) {
    auto manager = ConfigManager::instance();

    auto real_exit = true;
    if (manager.hasUnsavedConfig()) {
      real_exit = handleExit();
    }

    if (real_exit) {
      YDialog::deleteAllDialogs();
      return HandleResult::EXIT;
    }
  }

  if (event->widget() == help_button_) {
    handleHelp();
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
    YUIUnImpl("Apply Button");

    auto manager = ConfigManager::instance();
  }

  return HandleResult::SUCCESS;
}

auto UIBase::handleEvent() -> void {
  if (main_dialog_ == nullptr) {
    yuiError() << "main_dialog is nullptr when handling event" << endl;
    return;
  }

  auto user_handler = userHandleEvent();
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

    auto user_result = user_handler(event);
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

auto UIBase::warnDialog(const string &warning) -> void {
  YDialog *dialog = factory_->createPopupDialog();

  YLayoutBox *vbox = factory_->createVBox(dialog);
  factory_->createLabel(vbox, kWarnDialogTitle);
  factory_->createVSpacing(vbox, kVSpaceSize);

  YAlignment *minSize =
      factory_->createMinSize(vbox, kPopDialogMinWidth, kPopDialogMinHeight);
  YLabel *label = factory_->createOutputField(minSize, warning);
  label->setAutoWrap();

  factory_->createPushButton(vbox, "OK");
  dialog->waitForEvent();

  dialog->destroy();
}

auto UIBase::getName() const -> string { return name_; }
