#include "tools/widget_manager.h"
#include "controlpanel.h"

auto WidgetManager::addWidget(YWidget *widget,
                              std::function<HandleResult()> func) -> void {
  if (widgets_.contains(widget)) {
    widgets_.at(widget) = std::move(func);
  } else {
    widgets_.emplace(widget, std::move(func));
  }
}

auto WidgetManager::removeWidget(YWidget *widget) -> void {
  for (auto child = widget->childrenBegin(); child != widget->childrenEnd();
       ++child) {
    removeWidget(*child);
  }

  if (widgets_.contains(widget)) {
    widgets_.erase(widget);
  }
}

auto WidgetManager::handleEvent(YEvent *event) -> HandleResult {
  auto *widget_to_handle = event->widget();

  if (widgets_.contains(widget_to_handle)) {
    return widgets_.at(widget_to_handle)();
  }
  return HandleResult::CONT; /* use next widget manager */
}

auto WidgetManager::exec() -> bool {
  auto res = true;
  for (const auto &[_, func] : widgets_) {
    res &= (func() == HandleResult::SUCCESS);
  }

  return res;
}
