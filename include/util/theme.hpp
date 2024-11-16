#pragma once

#include "nlohmann/json.hpp"
#include <fstream>
#include <sstream>
#include <string>

using json = nlohmann::json;

namespace Theme {
void Load();
std::string ThemeEntry(std::string key);
inline json GetRelativeJson(json j, std::string key) {
  std::istringstream ss(key);
  std::string token;

  while (std::getline(ss, token, '.') && j != nullptr) {
    j = j[token];
  }

  return j;
}
} // namespace Theme

inline std::string operator""_theme(const char *key, size_t size) { return Theme::ThemeEntry(std::string(key, size)); }
