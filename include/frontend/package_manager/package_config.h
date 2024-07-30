#ifndef package_config_H
#define package_config_H

#include "backend/package_manager/package_manager_backend.h"
#include "frontend/ui_base.h"
#include <memory>

class PackageManagerConfig : public UIBase {
public:
  PackageManagerConfig(const std::string &name,
                       const std::shared_ptr<UIBase> &parent)
      : UIBase(name, parent){};

  ~PackageManagerConfig() override = default;

  [[nodiscard]] auto getComponentDescription() const -> std::string override;

  [[nodiscard]] auto getComponentName() const -> std::string override;

private:
  auto userDisplay(YDialog *main_dialog, DisplayLayout layout)
      -> DisplayResult override;

  auto userHandleEvent(YEvent *event) -> HandleResult override;
};

#endif // package_config_H
