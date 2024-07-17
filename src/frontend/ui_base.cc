#include "frontend/ui_base.h"

void UIBase::run(const ConfigManager &configManager) {
  const auto &configs = configManager.getConfigs();
  for (const auto &config : configs) {
    config->display();
  }

  main_dialog_->waitForEvent();
}
