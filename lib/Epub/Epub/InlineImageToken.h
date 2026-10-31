#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>

inline std::string makeInlineImageToken(const uint16_t id, const uint16_t width, const uint16_t height) {
  char token[15];
  snprintf(token, sizeof(token), "\x01I%04X%04X%04X", id, width, height);
  return token;
}

inline bool parseInlineImageToken(const std::string_view token, uint16_t& id, uint16_t& width, uint16_t& height) {
  if (token.size() != 14 || token[0] != '\x01' || token[1] != 'I') return false;
  uint16_t values[3] = {};
  for (int group = 0; group < 3; ++group) {
    uint16_t value = 0;
    for (int digit = 0; digit < 4; ++digit) {
      const char c = token[2 + group * 4 + digit];
      uint8_t nibble;
      if (c >= '0' && c <= '9')
        nibble = static_cast<uint8_t>(c - '0');
      else if (c >= 'A' && c <= 'F')
        nibble = static_cast<uint8_t>(c - 'A' + 10);
      else
        return false;
      value = static_cast<uint16_t>((value << 4) | nibble);
    }
    values[group] = value;
  }
  if (values[1] == 0 || values[2] == 0) return false;
  id = values[0];
  width = values[1];
  height = values[2];
  return true;
}
