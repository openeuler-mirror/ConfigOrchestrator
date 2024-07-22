#include "frontend/ui_base.h"
#include "YDialog.h"
#include "YLabel.h"
#include "YTypes.h"
#include <exception>
#include <string>

const std::string UIBase::kSoftwareName = "Control Panel";
const std::string UIBase::kBackButtonName = "&Back";
const std::string UIBase::kSearchButtonName = "&Search";
const std::string UIBase::kCloseButtonName = "&Close";
const std::string UIBase::kApplyButtonName = "&Apply";
const std::string UIBase::kHelpButtonName = "&Help";

auto UIBase::display() -> std::function<void()> {
  return [this]() {
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

      back_button_ =
          factory_->createPushButton(control_layout_, kBackButtonName);
      search_button_ =
          factory_->createPushButton(control_layout_, kSearchButtonName);
      close_button_ =
          factory_->createPushButton(control_layout_, kCloseButtonName);
      apply_button_ =
          factory_->createPushButton(control_layout_, kApplyButtonName);
      help_button_ =
          factory_->createPushButton(control_layout_, kHelpButtonName);

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
  };
}

auto UIBase::handleHelp() const {
  YWidgetFactory *fac = YUI::widgetFactory();
  YDialog *dialog = fac->createPopupDialog();

  YLayoutBox *vbox = fac->createVBox(dialog);
  YAlignment *minSize =
      fac->createMinSize(vbox, kPopDialogMinWidth, kPopDialogMinHeight);
  YLabel *label = fac->createOutputField(minSize, GetComponentDescription());
  label->setAutoWrap();

  fac->createPushButton(vbox, "OK");
  dialog->waitForEvent();
  dialog->destroy();
}

auto UIBase::handleExit() const -> bool {
  const static std::string msg = "There are unsaved ?";
  YWidgetFactory *fac = YUI::widgetFactory();
  YDialog *dialog = fac->createPopupDialog();

  YLayoutBox *vbox = fac->createVBox(dialog);
  YAlignment *minSize =
      fac->createMinSize(vbox, kPopDialogMinWidth, kPopDialogMinHeight);
  YLabel *label = fac->createOutputField(minSize, GetComponentDescription());
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

auto UIBase::handleButtons(YEvent *event) const {
  if (event->widget() == close_button_ ||
      event->eventType() == YEvent::CancelEvent) {
    if (manager_->hasUnsavedConfig()) {
      auto real_exit = handleExit();
      if (real_exit) {
        YDialog::deleteAllDialogs();
        std::exit(0);
      }
    } else {
      YDialog::deleteAllDialogs();
      std::exit(0);
    }
  }

  if (event->widget() == help_button_) {
    handleHelp();
  }

  if (event->widget() == search_button_) {
    YUIUnImpl("Search Button");
  }
  if (event->widget() == back_button_) {
    YUIUnImpl("Back Button");
  }
  if (event->widget() == apply_button_) {
    YUIUnImpl("Apply Button");
  }
}

auto UIBase::handleEvent() -> std::function<void()> {
  return [this]() {
    auto user_handler = userHandleEvent();
    while (true) {
      auto *event = main_dialog_->waitForEvent();

      if (event == nullptr) {
        yuiError() << "event is nullptr when return from waiting" << std::endl;
        std::terminate();
      }

      handleButtons(event);

      auto user_result = user_handler(event);
      if (user_result == HandleResult::EXIT) {
        break;
      }
    }
  };
}
