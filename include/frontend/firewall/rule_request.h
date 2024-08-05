// 8月 05 2024
#ifndef RULE_REQUEST_H
#define RULE_REQUEST_H

#include <optional>
#include <string>
#include <utility>
#include <vector>

using std::optional;
using std::string;
using std::vector;

class RuleMatch {
public:
  optional<string> src_port_;
  optional<string> dst_port_;
};

class RuleRequest {
public:
  /* rule index in new rule list */
  int index_;

  optional<string> src_ip_;
  optional<string> src_mask_;
  optional<string> dst_ip_;
  optional<string> dst_mask_;
  optional<string> proto_;

  optional<string> iniface_;
  optional<string> outiface_;

  vector<RuleMatch> matches_;

  string target;

  RuleRequest() = default;

  RuleRequest(int index, optional<string> src_ip, optional<string> src_mask,
              optional<string> dst_ip, optional<string> dst_mask,
              optional<string> proto, optional<string> iniface,
              optional<string> outiface, vector<RuleMatch> matches,
              string target)
      : index_(index), src_ip_(std::move(src_ip)),
        src_mask_(std::move(src_mask)), dst_ip_(std::move(dst_ip)),
        dst_mask_(std::move(dst_mask)), proto_(std::move(proto)),
        iniface_(std::move(iniface)), outiface_(std::move(outiface)),
        matches_(std::move(matches)), target(std::move(target)) {}
};

#endif