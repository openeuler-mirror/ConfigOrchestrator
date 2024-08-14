// 7月 19 2024
#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

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

/**
 * @brief result of handling an event when call a widget's closure
 *
 * SUCCESS:
 *      - in manager: event has been handled (note that not necessarily
 * successful)
 *      - in collector: confirm can be processed
 * BREAK: (in manager) event has been handled and break the event loop
 * EXIT: (in manager) event has been handled and exit the program
 * CONT: (in manager) continue to the next event handler since event not handled
 * ERROR: (in collector) error occurs and cancel proceeding
 */
enum class HandleResult : unsigned int { SUCCESS, BREAK, EXIT, CONT, ERROR };

enum class DisplayResult : unsigned int { SUCCESS, ERROR };

static void occupy(YLayoutBox *layout) {
  [[maybe_unused]] YLabel *label =
      YUI::widgetFactory()->createOutputField(layout, "OCCPUATION");
}

#endif
