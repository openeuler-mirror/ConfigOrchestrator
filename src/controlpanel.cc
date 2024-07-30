#include "controlpanel.h"
#include "backend/config_manager.h"
#include "frontend/main_menu.h"
#include "frontend/ui_base.h"
#include "tools/cplog.h"

#include <memory>

auto main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) -> int {
  YUILog::setLogFileName("/tmp/controlpanel.log");
  YUILog::enableDebugLogging();

  YUI::app()->setApplicationTitle("Control Panel");

  auto menu = std::make_shared<MainMenu>("Main Menu");
  menu->display();
  menu->handleEvent();

  return 0;
}
