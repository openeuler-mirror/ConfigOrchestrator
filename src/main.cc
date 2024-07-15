#include "YDialog.h"
#include "YEvent.h"
#include "YLayoutBox.h"
#include "YUI.h"
#include "YWidgetFactory.h"

auto main(int argc, char **argv) -> int {
  YDialog *dialog = YUI::widgetFactory()->createPopupDialog();
  YLayoutBox *vbox = YUI::widgetFactory()->createVBox(dialog);
  YUI::widgetFactory()->createLabel(vbox, "Hello, World!");
  YUI::widgetFactory()->createPushButton(vbox, "&OK");

  dialog->waitForEvent();
  dialog->destroy();
}
