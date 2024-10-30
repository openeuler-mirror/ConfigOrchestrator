#ifndef RULE_REQUEST_H
#define RULE_REQUEST_H

#include "backend/firewall/firewall_context.h"

#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <libiptc/libiptc.h>

using std::optional;
using std::string;
using std::tuple;
using std::vector;

namespace RequestProto {
const string TCP = "TCP";
const string UDP = "UDP";
} // namespace RequestProto

class RuleMatch {
public:
  optional<tuple<string, string>> src_port_range_;
  optional<tuple<string, string>> dst_port_range_;
};

class RuleRequest {
public:
  /* rule index in new rule list */
  int index_{1};

  optional<string> src_ip_;
  optional<string> src_mask_;
  optional<string> dst_ip_;
  optional<string> dst_mask_;
  string proto_;

  optional<string> iniface_;
  optional<string> outiface_;

  vector<RuleMatch> matches_;

  string target_;

  RuleRequest() : proto_(RequestProto::TCP), target_(IPTC_LABEL_ACCEPT) {};

  RuleRequest(int index, optional<string> src_ip, optional<string> src_mask,
              optional<string> dst_ip, optional<string> dst_mask, string proto,
              optional<string> iniface, optional<string> outiface,
              vector<RuleMatch> matches, string target)
      : index_(index), src_ip_(std::move(src_ip)),
        src_mask_(std::move(src_mask)), dst_ip_(std::move(dst_ip)),
        dst_mask_(std::move(dst_mask)), proto_(std::move(proto)),
        iniface_(std::move(iniface)), outiface_(std::move(outiface)),
        matches_(std::move(matches)), target_(std::move(target)) {}

  RuleRequest(iptc_handle *handle, const struct ipt_entry *rule, int index);

  auto to_entry_bytes(const ctx_t &context) -> optional<vector<char>>;
};

#endif
