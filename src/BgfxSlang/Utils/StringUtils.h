#pragma once
#include <cstdint>
#include <format>
#include <span>
#include <string>

namespace BgfxSlang {

inline std::string toHex(std::span<const uint8_t> data) {
  std::string hex;
  hex.reserve(data.size() * 2);
  for (const auto &byte : data) {
    hex += std::format("{:02x}", byte);
  }
  return hex;
}
} // namespace BgfxSlang