#include "tools/uitools.h"

#include "YDialog.h"
#include "YEvent.h"
#include "YLabel.h"
#include "YLayoutBox.h"
#include "YUI.h"
#include "YWidgetFactory.h"
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

auto showDialog(const string &title, const string &msg) -> void {
  static constexpr YLayoutSize_t kVSpaceSize = 1;

  auto *factory = YUI::widgetFactory();
  auto *dialog = factory->createPopupDialog();

  auto *vbox = factory->createVBox(dialog);
  factory->createLabel(vbox, title);
  factory->createVSpacing(vbox, kVSpaceSize);

  YAlignment *minSize = factory->createMinSize(
      vbox, dialog_meta::kPopDialogMinWidth, dialog_meta::kPopDialogMinHeight);
  YLabel *label = factory->createOutputField(minSize, msg);
  label->setAutoWrap();

  factory->createPushButton(vbox, "OK");
  dialog->waitForEvent();

  dialog->destroy();
}
