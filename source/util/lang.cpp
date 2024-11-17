#include "util/lang.hpp"
#include "nlohmann/json.hpp"
#include "util/config.hpp"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <switch.h>

using json = nlohmann::json;

namespace Language {
json lang;

void Load() {
  // https://switchbrew.org/wiki/Settings_services#LanguageCode
  // Get language from the switch system settings.
  SetLanguage ourLang;
  u64 lcode = 0;
  setInitialize();
  setGetSystemLanguage(&lcode);
  setMakeLanguage(lcode, &ourLang);
  setExit();
  int syslang = (int)ourLang;
  std::string languagePath;
  int langInt = inst::config::languageSetting;
  if (std::filesystem::exists(inst::config::appDir + "/lang/custom.json")) {
    languagePath = (inst::config::appDir + "/lang/custom.json");
  } else {
    // clang-format off
    switch (langInt) {
      case 1: languagePath = "romfs:/lang/en.json"; break;
      case 2: languagePath = "romfs:/lang/jp.json"; break;
      case 3: languagePath = "romfs:/lang/fr.json"; break;
      case 4: languagePath = "romfs:/lang/de.json"; break;
      case 5: languagePath = "romfs:/lang/it.json"; break;
      case 6: languagePath = "romfs:/lang/ru.json"; break;
      case 7: languagePath = "romfs:/lang/es.json"; break;
      case 8: languagePath = "romfs:/lang/tw.json"; break;
      case 9: languagePath = "romfs:/lang/cn.json"; break;
      case 0:
        switch (syslang) {
          case 0: languagePath = "romfs:/lang/jp.json"; break;
          case 1: languagePath = "romfs:/lang/en.json"; break;
          case 2: languagePath = "romfs:/lang/fr.json"; break;
          case 3: languagePath = "romfs:/lang/de.json"; break;
          case 4: languagePath = "romfs:/lang/it.json"; break;
          case 5: languagePath = "romfs:/lang/es.json"; break;
          case 6: languagePath = "romfs:/lang/cn.json"; break;
          case 7: languagePath = "romfs:/lang/en.json"; break;
          case 8: languagePath = "romfs:/lang/en.json"; break;
          case 9: languagePath = "romfs:/lang/en.json"; break;
          case 10: languagePath = "romfs:/lang/ru.json"; break;
          case 11: languagePath = "romfs:/lang/tw.json"; break;
          case 12: languagePath = "romfs:/lang/en.json"; break;
          case 13: languagePath = "romfs:/lang/fr.json"; break;
          case 14: languagePath = "romfs:/lang/es.json"; break;
          case 15: languagePath = "romfs:/lang/tw.json"; break;
          case 16: languagePath = "romfs:/lang/cn.json"; break;
          case 17: languagePath = "romfs:/lang/en.json"; break;
          default: languagePath = "romfs:/lang/en.json"; break;
        }
        break;
      default: languagePath = "romfs:/lang/en.json"; break;
    }
    // clang-format on
  }
  std::ifstream ifs;
  if (std::filesystem::exists(languagePath))
    ifs = std::ifstream(languagePath);
  else
    ifs = std::ifstream("romfs:/lang/en.json");
  if (!ifs.good()) {
    std::cout << "[FAILED TO LOAD LANGUAGE FILE]" << std::endl;
    return;
  }
  lang = json::parse(ifs);
  ifs.close();
}

inline json GetRelativeJson(json j, std::string key) {
  std::istringstream ss(key);
  std::string token;
  while (std::getline(ss, token, '.') && j != nullptr) {
    j = j[token];
  }
  return j;
}

std::string LanguageEntry(std::string key) {
  json j = GetRelativeJson(lang, key);
  if (j == nullptr) {
    return "didn't find: " + key;
  }
  return j.get<std::string>();
}

std::string GetRandomMsg() {
  json j = Language::GetRelativeJson(lang, "inst.finished");
  srand(time(NULL));
  return (j[rand() % j.size()]);
}
} // namespace Language
