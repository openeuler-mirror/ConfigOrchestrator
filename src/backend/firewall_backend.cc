#include "backend/firewall_backend.h"

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

    for (const struct ipt_entry *entry = iptc_first_rule(chain, handle_);
         entry != nullptr; entry = iptc_next_rule(entry, handle_)) {
      yuiError() << "Rule: " << entry->ip.iniface << std::endl;
    }
  }

  iptc_free(handle_);

  return chains;
}
