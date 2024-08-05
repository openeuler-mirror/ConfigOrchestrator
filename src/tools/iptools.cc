#include "tools/iptools.h"
#include <arpa/inet.h>

auto ip2Tuple(uint32_t ip) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> {
  constexpr uint8_t mask = 0xFF;
  constexpr uint8_t shift = 8;

  uint8_t part1 = (ip >> shift * 3) & mask;
  uint8_t part2 = (ip >> shift * 2) & mask;
  uint8_t part3 = (ip >> shift * 1) & mask;
  uint8_t part4 = ip & mask;
  return std::make_tuple(part1, part2, part3, part4);
}

auto tuple2Ip(const std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> &tuple)
    -> uint32_t {
  constexpr uint8_t mask = 0xFF;
  constexpr uint8_t shift = 8;

  uint32_t ip = (static_cast<uint32_t>(std::get<0>(tuple)) << shift * 3) |
                (static_cast<uint32_t>(std::get<1>(tuple)) << shift * 2) |
                (static_cast<uint32_t>(std::get<2>(tuple)) << shift * 1) |
                (static_cast<uint32_t>(std::get<3>(tuple)));
  return ip;
}

auto ip2String(uint32_t ip) -> string {
  struct in_addr in;
  in.s_addr = ip;
  return inet_ntoa(in);
}

auto iface2String(const char *iface) -> string {
  return string(reinterpret_cast<const char *>(iface));
}

auto protocols() -> vector<tuple<string, uint8_t>> {
  static constexpr auto kICMPType = 1;
  static constexpr auto kTCPType = 6;
  static constexpr auto kUDPType = 17;

  static vector<tuple<string, uint8_t>> r = {
      {"TCP", kTCPType}, {"UDP", kUDPType}, {"ICMP", kICMPType}};
  return r;
}

auto proto2String(uint8_t proto) -> string {
  auto protos = protocols();
  for (const auto &p : protos) {
    if (std::get<1>(p) == proto) {
      return std::get<0>(p);
    }
  }
  return "UNKNOWN";
}

auto string2Proto(const string &proto) -> uint8_t {
  auto protos = protocols();
  for (const auto &p : protos) {
    if (std::get<0>(p) == proto) {
      return std::get<1>(p);
    }
  }

  return 0;
}
