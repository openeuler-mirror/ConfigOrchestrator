# Event handler mechanism in control panel

## 独立产生 event 的组件

比如 button 这类点击会独立产生一个 event 的组件，用 `ui_base.cc` 的 `WidgetManager` 管理起来，创建一个和事件相关的 closure, 比如：

 ```cpp
widget_manager_.addWidget(update_button, [this, iptable_child, index,
										main_dialog, layout]() {
	auto request = createUpdateRule(index);
	if (request == nullptr) {
	  return HandleResult::SUCCESS; /* cancel */
	}
	/* else handle request from popup dialog ... */
	return HandleResult::SUCCESS;
});
 ```

注意：

- 作为事件驱动框架的返回值，返回 SUCCESS 不代表执行成功了，而是这个事件被处理了
- 如果处理完这个事件后需要退出页面，应返回 BREAK
- 如果处理完这个事件后需要退出程序，应返回 EXIT
- 如果在 handler 中才能判断是否需要处理，且不是这个 handler 处理，返回 CONT

## 不产生 event 的组件

比如下拉框，InputField, 单选框这种不会修改后马上产生 event 的组件，也用 `ui_base.cc` 的 `WidgetManager` 收集，然后将需要这些值的 event 和 `WidgetManager::exec()` 关联起来。

```cpp
WidgetManager collector;
{ /* when confirm is pressed, get value and fill in req */
	collector.addWidget(proto_box, [proto_box, &request]() {
	  auto *widget = dynamic_cast<YComboBox *>(proto_box);
	  auto *item = widget->selectedItem();
	
	  auto value = item->label();
	  request->proto_ = value;
	  return HandleResult::SUCCESS;
	});
}
// immediate event producer
auto *confirm = fac->createPushButton(control_layout, "&OK");
auto *cancel = fac->createPushButton(control_layout, "&Cancel");

while (true) {
	auto *event = dialog->waitForEvent();
	if (event->widget() == confirm) {
	  if (!collector.exec()) {
		static const string kInvalidInput = "Invalid input.";
		showDialog(dialog_meta::ERROR, kInvalidInput);
		continue;
	  } // input is legal here
	  dialog->destroy();
	  return request;
	}
	
	if (event->widget() == cancel ||
		event->eventType() == YEvent::CancelEvent) {
	  break;
	}
}
```

注意这里的 SUCCESS 就表示输入是合法的了。

## 小结

以上是框架推荐的事件驱动的处理，利用 closure 使得事件处理和产生事件的组件更近，更易理解。当然不是所有页面都需要这样处理，有些页面只是简单的展示，比如 `uitools.cc` 中的弹窗：

```cpp
auto showDialog(const string &title, const string &msg) -> void {
  /* 。。。 */
  YAlignment *minSize = factory->createMinSize(
      vbox, dialog_meta::kPopDialogMinWidth, dialog_meta::kPopDialogMinHeight);
  YLabel *label = factory->createOutputField(minSize, msg);
  label->setAutoWrap();

  factory->createPushButton(vbox, "OK");
  dialog->waitForEvent();

  dialog->destroy();
}
```

保持简洁最重要。
