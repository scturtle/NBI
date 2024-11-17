#pragma once

#include <fstream>
#include <sstream>
#include <string>

namespace Language {
void Load();
std::string LanguageEntry(std::string key);
std::string GetRandomMsg();
} // namespace Language

inline std::string operator""_lang(const char *key, size_t size) {
  return Language::LanguageEntry(std::string(key, size));
}
