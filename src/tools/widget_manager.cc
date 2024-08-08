#include "tools/widget_manager.h"

auto WidgetManager::addWidget(YWidget *widget, std::function<bool()> func)
    -> void {
  if (widgets_.contains(widget)) {
    widgets_.at(widget) = std::move(func);
  } else {
    widgets_.emplace(widget, std::move(func));
  }
}

auto WidgetManager::removeWidget(YWidget *widget) -> void {
  if (widgets_.contains(widget)) {
    widgets_.erase(widget);
  }
}

auto WidgetManager::handleEvent(YEvent *event) -> HandleResult {
  auto *widget_to_handle = event->widget();

  if (widgets_.contains(widget_to_handle)) {
    if (widgets_.at(widget_to_handle)()) {
      return HandleResult::BREAK;
    }
  }
  return HandleResult::CONT;
}
