#include "backend/firewall/firewall_backend.h"
#include <memory>
#include <vector>

auto FirewallBackend::getChainNames() -> std::vector<std::string> {
  assert(type_ == FirewallBackendType::TABLE && handle_ != nullptr);

  std::vector<std::string> chains;

  for (const auto *chain = iptc_first_chain(handle_); chain != nullptr;
       chain = iptc_next_chain(handle_)) {
    chains.emplace_back(chain);
  }
  return chains;
}

auto FirewallBackend::getRules() -> std::vector<const struct ipt_entry *> {
  assert(type_ == FirewallBackendType::CHAIN && handle_ != nullptr);

  std::vector<const struct ipt_entry *> rules;

  for (const struct ipt_entry *entry = iptc_first_rule(name_.c_str(), handle_);
       entry != nullptr; entry = iptc_next_rule(entry, handle_)) {
    rules.emplace_back(entry);
  }
  return rules;
}

auto FirewallBackend::init() -> bool {
  if (type_ == FirewallBackendType::OVERALL) {
  } else if (type_ == FirewallBackendType::TABLE) {
    assert(handle_ == nullptr);

    handle_ = iptc_init(name_.c_str());
    if (handle_ == nullptr) {
      yuiError() << "Error initializing iptables's table: " << name_
                 << " error: " << iptc_strerror(errno) << std::endl;
      return false;
    }
  } else if (type_ == FirewallBackendType::CHAIN) {
    assert(handle_ == nullptr);
    auto parent =
        std::dynamic_pointer_cast<FirewallBackend>(getParent().lock());

    assert(parent != nullptr);
    handle_ = parent->getHandler();

    assert(handle_ != nullptr);
  } else {
    return false;
  }

  return true;
}
