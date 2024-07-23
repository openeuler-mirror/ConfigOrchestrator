// 7月 19 2024
#ifndef CONTROLPANE_H
#define CONTROLPANE_H

#include "YDialog.h"
#include "YEvent.h"
#include "YLabel.h"
#include "YLayoutBox.h"
#include "YUI.h"
#include "YWidgetFactory.h"

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

#define YUILogComponent "controlpane"
#include <yui/YUILog.h>

enum class HandleResult : unsigned int { SUCCESS, BREAK, EXIT, NEW_PAGE };

enum class DisplayResult : unsigned int { SUCCESS, ERROR };

static void YUIUnImpl(const std::string &component) {
  static const std::string msg = "Unimplemented function called";

  YWidgetFactory *fac = YUI::widgetFactory();
  YDialog *dialog = fac->createPopupDialog();

  auto real_msg = component + ": " + msg;
  YLayoutBox *vbox = fac->createVBox(dialog);
  YLabel *label = fac->createOutputField(vbox, real_msg);

  fac->createPushButton(vbox, "OK");
  dialog->waitForEvent();
  dialog->destroy();
}

static void occupy(YLayoutBox *layout) {
  [[maybe_unused]] YLabel *label =
      YUI::widgetFactory()->createOutputField(layout, "OCCPUATION");
}

#endif
