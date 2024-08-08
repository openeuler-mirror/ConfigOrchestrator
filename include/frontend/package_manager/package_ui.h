#ifndef package_config_H
#define package_config_H

#include "backend/package_manager/package_manager_backend.h"
#include "frontend/ui_base.h"
#include <memory>

using std::cout;
using std::endl;

class PackageManagerConfig : public UIBase {
public:
  PackageManagerConfig(const string &name, const shared_ptr<UIBase> &parent)
      : UIBase(name, parent){};

  ~PackageManagerConfig() override = default;

  [[nodiscard]] auto getComponentDescription() const -> string override;

  [[nodiscard]] auto getComponentName() const -> string override;

private:
  auto userDisplay(YDialog *main_dialog, DisplayLayout layout)
      -> DisplayResult override;

  auto userHandleEvent(YEvent *event) -> HandleResult override;
};

#endif // package_config_H
