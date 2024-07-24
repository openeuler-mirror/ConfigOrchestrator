#ifndef package_config_H
#define package_config_H

#include "backend/package_manager/package_manager_backend.h"
#include "frontend/ui_base.h"
#include <memory>

class PackageManagerConfig : public UIBase {
public:
  PackageManagerConfig(const std::string &name,
                       const std::shared_ptr<ConfigManager> &manager,
                       const std::shared_ptr<UIBase> &parent)
      : UIBase(name, manager, parent,
               std::make_shared<PackageManagerBackend>()){};

  ~PackageManagerConfig() override = default;

  [[nodiscard]] auto GetComponentDescription() const -> std::string override;

  [[nodiscard]] auto GetComponentName() const -> std::string override;

private:
  auto userDisplay()
      -> std::function<DisplayResult(YDialog *main_dialog,
                                     YLayoutBox *main_layout_)> override;

  auto userHandleEvent() -> std::function<HandleResult(YEvent *event)> override;

  [[nodiscard]] auto GetChildrenInitializer() const
      -> std::vector<std::function<std::shared_ptr<UIBase>(
          const std::shared_ptr<ConfigManager> &manager,
          const std::shared_ptr<UIBase> &parent)>> override;
};

#endif // package_config_H
