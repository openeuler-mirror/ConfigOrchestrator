#ifndef ui_bas_H
#define ui_bas_H

#include "YDialog.h"
#include "YEvent.h"
#include "YLayoutBox.h"
#include "YUI.h"
#include "YWidgetFactory.h"
#include "backend/config_manager.h"
#include <ncurses.h>
#include <stack>

class UIBase {
public:
  UIBase() {
    main_dialog_ = YUI::widgetFactory()->createPopupDialog();
    vbox_left_ = YUI::widgetFactory()->createVBox(main_dialog_);
    YUI::widgetFactory()->createLabel(vbox_left_, "Hello, World!");
    YUI::widgetFactory()->createPushButton(vbox_left_, "&OK");
  }

  ~UIBase() { main_dialog_->destroy(); }

  void run(const ConfigManager &configManager);

private:
  YDialog *main_dialog_;
  YLayoutBox *vbox_left_;
};

#endif // ui_bas_H
