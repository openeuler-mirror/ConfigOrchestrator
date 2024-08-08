#ifndef CHAIN_REQUEST_H
#define CHAIN_REQUEST_H

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

class ChainRequest {
public:
  ChainRequest() = default;

  ChainRequest(const ChainRequest &request) = default;

  ChainRequest(string chain_) : chain_name_(std::move(chain_)) {}

  string chain_name_;
};

#endif