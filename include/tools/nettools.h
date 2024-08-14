#ifndef NETTOOLS_H
#define NETTOOLS_H

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

using std::string;
using std::tuple;
using std::vector;

auto ip2Tuple(uint32_t ip) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>;

auto tuple2Ip(const std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> &tuple)
    -> uint32_t;

auto protocols() -> vector<tuple<string, uint8_t>>;

auto proto2String(uint8_t proto) -> string;

auto string2Proto(string proto) -> uint8_t;

auto iptTargets() -> vector<string>;

#endif