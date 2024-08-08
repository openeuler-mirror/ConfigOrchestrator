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

  auto addWidget(YWidget *widget, std::function<bool()> func) -> void;

  auto removeWidget(YWidget *widget) -> void;

  auto handleEvent(YEvent *event) -> HandleResult;

private:
  std::unordered_map<YWidget *, std::function<bool()>> widgets_;
};

#endif
