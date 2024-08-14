#ifndef WIDGET_MANAGER_H
#define WIDGET_MANAGER_H

#include "YEvent.h"
#include "YWidget.h"
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

class WidgetManager {
public:
  WidgetManager() = default;
  ~WidgetManager() = default;

  auto addWidget(YWidget *widget, std::function<HandleResult()> func) -> void;

  auto removeWidget(YWidget *widget) -> void;

  /*
   * @brief handle an event when widget manager is used as event manager
   */
  auto handleEvent(YEvent *event) -> HandleResult;

  /*
   * @brief execute all widgets' functions when is used as widget collector
   */
  auto exec() -> bool;

private:
  std::unordered_map<YWidget *, std::function<HandleResult()>> widgets_;
};

#endif
