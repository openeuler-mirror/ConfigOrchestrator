#include "backend/firewall/firewall_backend.h"
#include "backend/firewall/firewall_tab_chain.h"
#include <memory>

FirewallBackend::FirewallBackend(
    const std::shared_ptr<ConfigBackendBase> &parent)
    : ConfigBackendBase(parent) {}

auto FirewallBackend::getAllIPChain(int index) -> std::vector<std::string> {
  if (!isSuperUser()) {
    yuiError() << "You must be root to run this command" << std::endl;
    std::exit(-1);
  }

  std::vector<std::string> chains;

  auto table = getTableNames()[index];
  handle_ = iptc_init(table.c_str());
  if (handle_ == nullptr) {
    yuiError() << "Error initializing: " << iptc_strerror(errno) << std::endl;
  }

  for (const auto *chain = iptc_first_chain(handle_); chain != nullptr;
       chain = iptc_next_chain(handle_)) {
    chains.emplace_back(chain);

    const struct ipt_entry *entry = iptc_first_rule(chain, handle_);
    if (entry == nullptr) {
      yuiMilestone() << "No rule in chain " << chain << std::endl;
    }

    for (; entry != nullptr; entry = iptc_next_rule(entry, handle_)) {
      yuiMilestone() << "Rule: " << entry->ip.iniface << std::endl;
    }
  }

  iptc_free(handle_);

  return chains;
}

auto FirewallBackend::init() -> bool {
  auto names = getTableNames();

  auto parent = shared_from_this();
  for (const auto &name : names) {
    handle_ = iptc_init(name.c_str());
    if (handle_ == nullptr) {
      yuiError() << "Error initializing: " << iptc_strerror(errno) << std::endl;
    }

    tables_.emplace(name, std::make_shared<FirewallTabChain>(
                              parent, IPTType::TABLE, name, handle_));
  }
  return true;
}
