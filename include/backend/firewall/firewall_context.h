#ifndef FIREWALL_CONTEXT_H
#define FIREWALL_CONTEXT_H

#include <memory>
#include <string>

using std::shared_ptr;
using std::string;
enum class FirewallLevel { OVERALL, TABLE, CHAIN };

class FirewallContext {
public:
  FirewallContext() = default;

  FirewallContext(const shared_ptr<FirewallContext> &context)
      : level_(context->level_), table_(context->table_),
        chain_(context->chain_) {}

  FirewallLevel level_{};
  string table_{};
  string chain_{};

  [[nodiscard]] auto serialize() const -> string {
    string serialized = "Firewall Config";

    if (level_ > FirewallLevel::OVERALL) {
      serialized += " Table: " + table_;
    }

    if (level_ > FirewallLevel::TABLE) {
      serialized += " Chain: " + chain_;
    }
    return serialized;
  }
};

#endif
