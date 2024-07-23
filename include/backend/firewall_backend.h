#include "iptables.h"
#include "libiptc/libiptc.h"

#include "config_backend_base.h"
#include "tools/cplog.h"
#include "tools/sys.h"

#include <string>
#include <vector>

class FirewallBackend : public ConfigBackendBase {
public:
  FirewallBackend() = default;

  ~FirewallBackend() override = default;

  auto getAllIPChain(int index) -> std::vector<std::string>;

private:
  struct iptc_handle *handle_;

  static auto getTableNames() -> std::vector<std::string> {
    return {"filter"};
  };
};
