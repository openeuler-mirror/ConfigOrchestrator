#include "config_backend_base.h"
#include "iptables.h"
#include "libiptc/libiptc.h"
#include <string>
#include <vector>

class FirewallBackend : public ConfigBackendBase {
public:
  FirewallBackend() {
    for (const auto &table : getTableNames()) {
      handle = iptc_init(table.c_str());
    }

    if (handle != nullptr) {
      fprintf(stderr, "Error initializing: %s\n", iptc_strerror(errno));
      return;
    }

    for (const auto *chain = iptc_first_chain(handle); chain != nullptr;
         chain = iptc_next_chain(handle)) {
      printf("Chain: %s\n", chain);
      for (const struct ipt_entry *entry = iptc_first_rule(chain, handle);
           entry != nullptr; entry = iptc_next_rule(entry, handle)) {
        printf("Rule: %s\n", entry->ip.iniface);
      }
    }

    iptc_free(handle);
  }

private:
  struct iptc_handle *handle;

  static auto getTableNames() -> std::vector<std::string> {
    return {"filter"};
  };
};
