#pragma once

#include <string>
#include <vector>

namespace inst::config {
static const std::string appDir = "sdmc:/switch/tinwoo";
static const std::string configPath = appDir + "/config.json";
static const std::string appVersion = "1.0.22";

extern int languageSetting;
extern bool ignoreReqVers;
extern bool validateNCAs;
extern bool overClock;
extern bool deletePrompt;
extern bool usbAck;
extern bool fixticket;
extern bool listoveride;

void setConfig();
void parseConfig();
} // namespace inst::config
