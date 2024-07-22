#include "controlpane.h"
#include "backend/config_manager.h"
#include "frontend/main_menu.h"
#include "frontend/ui_base.h"
#include <memory>

auto main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) -> int {

  YUI::app()->setApplicationTitle("Control Panel");

  YUILog::setLogFileName("/tmp/controlpanel.log");
  YUILog::enableDebugLogging();

  auto configManager = std::make_shared<ConfigManager>();
  auto main = std::make_shared<MainMenu>("Main Menu", configManager);

  auto display = main->display();
  display();

  auto event_handler = main->handleEvent();
  event_handler();

  return 0;
}
