#pragma once

#include <vector>

namespace inst::config {
	static const std::string appDir = "sdmc:/switch/tinwoo";
	static const std::string configPath = appDir + "/config.json";
	static const std::string appVersion = "1.0.22";

	extern std::string gAuthKey;
	extern std::string sigPatchesUrl;
	extern std::string httpIndexUrl;
	extern std::string httplastUrl;
	extern std::string httplastUrl2;
	extern std::vector<std::string> updateInfo;
	extern int languageSetting;
	extern bool ignoreReqVers;
	extern bool validateNCAs;
	extern bool overClock;
	extern bool deletePrompt;
	extern bool autoUpdate;
	extern bool usbAck;
	extern bool useTheme;
	extern bool useSound;
	extern bool useMusic;
	extern bool fixticket;
	extern bool listoveride;
	extern bool httpkeyboard;

	void setConfig();
	void parseConfig();
}
