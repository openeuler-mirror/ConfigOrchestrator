#include "YDialog.h"
#include "YEvent.h"
#include "YLayoutBox.h"
#include "YUI.h"
#include "YWidgetFactory.h"

auto main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) -> int {
  YDialog *dialog = YUI::widgetFactory()->createPopupDialog();
  YLayoutBox *vbox = YUI::widgetFactory()->createVBox(dialog);
  YUI::widgetFactory()->createLabel(vbox, "Hello, World!");
  YUI::widgetFactory()->createPushButton(vbox, "&OK");
  YUI::widgetFactory()->createPushButton(vbox, "&Not OK");

  dialog->waitForEvent();
  dialog->destroy();
}
