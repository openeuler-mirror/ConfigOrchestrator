#include "backend/config_manager.h"
#include "frontend/ui_base.h"

auto main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) -> int {
  ConfigManager configManager;

  UIBase tui;
  tui.run(configManager);

  return 0;
}
