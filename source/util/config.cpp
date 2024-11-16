#include "util/config.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iomanip>

namespace inst::config {
int languageSetting;
bool deletePrompt;
bool ignoreReqVers;
bool overClock;
bool usbAck;
bool validateNCAs;
bool fixticket;
bool listoveride;

void setConfig() {
  nlohmann::json j = {{"deletePrompt", deletePrompt},
                      {"ignoreReqVers", ignoreReqVers},
                      {"languageSetting", languageSetting},
                      {"overClock", overClock},
                      {"usbAck", usbAck},
                      {"validateNCAs", validateNCAs},
                      {"fixticket", fixticket},
                      {"listoveride", listoveride}};
  std::ofstream file(inst::config::configPath);
  file << std::setw(4) << j << std::endl;
}

void parseConfig() {
  try {
    std::ifstream file(inst::config::configPath);
    nlohmann::json j;
    file >> j;
    fixticket = j["fixticket"].get<bool>();
    listoveride = j["listoveride"].get<bool>();
    deletePrompt = j["deletePrompt"].get<bool>();
    ignoreReqVers = j["ignoreReqVers"].get<bool>();
    languageSetting = j["languageSetting"].get<int>();
    overClock = j["overClock"].get<bool>();
    usbAck = j["usbAck"].get<bool>();
    validateNCAs = j["validateNCAs"].get<bool>();
  } catch (...) {
    // If loading values from the config fails, we just load the defaults and overwrite the old config
    languageSetting = 0;
    deletePrompt = true;
    fixticket = true;
    listoveride = false;
    ignoreReqVers = true;
    overClock = true;
    usbAck = false;
    validateNCAs = true;
    setConfig();
  }
}
} // namespace inst::config
