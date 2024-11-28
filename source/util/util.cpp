#include "util/util.hpp"
#include "nlohmann/json.hpp"
#include "nx/ipc/tin_ipc.h"
#include "nx/usbhdd.h"
#include "switch.h"
#include "ui/MainApplication.hpp"
#include "util/config.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <sys/statvfs.h>
#include <tuple>
#include <unistd.h>
#include <vector>

namespace inst::ui {
extern MainApplication *mainApp;
}

int statvfs(const char *path, struct statvfs *buf);

namespace inst::util {
void initApp() {
  if (!std::filesystem::exists("sdmc:/switch"))
    std::filesystem::create_directory("sdmc:/switch");
  if (!std::filesystem::exists(inst::config::appDir))
    std::filesystem::create_directory(inst::config::appDir);
  inst::config::parseConfig();
  socketInitializeDefault();
#ifdef __DEBUG__
  nxlinkStdio();
#endif
  nx::hdd::init();
}

void deinitApp() {
  nx::hdd::exit();
  socketExit();
}

void initInstallServices() {
  ncmInitialize();
  nsInitialize();
  nsextInitialize();
  esInitialize();
  splCryptoInitialize();
  splInitialize();
}

void deinitInstallServices() {
  ncmExit();
  nsExit();
  nsextExit();
  esExit();
  splCryptoExit();
  splExit();
}

std::tuple<double, double, double> getSpaceInfo(const char *path) {
  struct statvfs stat;
  if (statvfs(path, &stat) != 0)
    return {0, 0, 0};
  long total = stat.f_blocks;
  long free = stat.f_bavail;
  long freeForRoot = stat.f_bfree;
  long used = total - freeForRoot;
  long nonRootTotal = used + free;
  return {stat.f_frsize * free, stat.f_frsize * used, stat.f_frsize * nonRootTotal};
}

auto caseInsensitiveLess = [](auto &x, auto &y) -> bool {
  return toupper(static_cast<unsigned char>(x)) < toupper(static_cast<unsigned char>(y));
};

static bool ignoreCaseCompare(const std::string &a, const std::string &b) {
  return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), caseInsensitiveLess);
}

std::vector<std::filesystem::path> getDirectoryFiles(const std::string &dir,
                                                     const std::vector<std::string> &extensions) {
  std::vector<std::filesystem::path> files;
  for (auto &p : std::filesystem::directory_iterator(dir)) {
    if (std::filesystem::is_regular_file(p)) {
      std::string ourExtension = p.path().extension().string();
      std::transform(ourExtension.begin(), ourExtension.end(), ourExtension.begin(), ::tolower);
      if (extensions.empty() || std::find(extensions.begin(), extensions.end(), ourExtension) != extensions.end()) {
        files.push_back(p.path());
      }
    }
  }
  std::sort(files.begin(), files.end(), ignoreCaseCompare);
  return files;
}

std::vector<std::filesystem::path> getDirsAtPath(const std::string &dir) {
  std::vector<std::filesystem::path> files;
  for (auto &p : std::filesystem::directory_iterator(dir)) {
    if (std::filesystem::is_directory(p)) {
      files.push_back(p.path());
    }
  }
  std::sort(files.begin(), files.end(), ignoreCaseCompare);
  files.erase(std::remove(files.begin(), files.end(), "ums0:/System Volume Information"), files.end());
  return files;
}

std::string shortenString(std::string ourString, int ourLength, bool isFile) {
  std::filesystem::path ourStringAsAPath = ourString;
  std::string ourExtension = ourStringAsAPath.extension().string();
  if (ourString.size() - ourExtension.size() > (unsigned long)ourLength) {
    if (isFile)
      return (std::string)ourString.substr(0, ourLength) + "(...)" + ourExtension;
    else
      return (std::string)ourString.substr(0, ourLength) + "...";
  } else
    return ourString;
}

std::vector<uint32_t> setClockSpeed(int deviceToClock, uint32_t clockSpeed) {
  uint32_t hz = 0;
  uint32_t previousHz = 0;

  if (deviceToClock > 2 || deviceToClock < 0)
    return {0, 0};

  if (hosversionAtLeast(8, 0, 0)) {
    ClkrstSession session = {0};
    PcvModuleId pcvModuleId;
    pcvInitialize();
    clkrstInitialize();

    switch (deviceToClock) {
    case 0:
      pcvGetModuleId(&pcvModuleId, PcvModule_CpuBus);
      break;
    case 1:
      pcvGetModuleId(&pcvModuleId, PcvModule_GPU);
      break;
    case 2:
      pcvGetModuleId(&pcvModuleId, PcvModule_EMC);
      break;
    }

    clkrstOpenSession(&session, pcvModuleId, 3);
    clkrstGetClockRate(&session, &previousHz);
    clkrstSetClockRate(&session, clockSpeed);
    clkrstGetClockRate(&session, &hz);

    pcvExit();
    clkrstCloseSession(&session);
    clkrstExit();

    return {previousHz, hz};
  } else {
    PcvModule pcvModule;
    pcvInitialize();

    switch (deviceToClock) {
    case 0:
      pcvModule = PcvModule_CpuBus;
      break;
    case 1:
      pcvModule = PcvModule_GPU;
      break;
    case 2:
      pcvModule = PcvModule_EMC;
      break;
    }

    pcvGetClockRate(pcvModule, &previousHz);
    pcvSetClockRate(pcvModule, clockSpeed);
    pcvGetClockRate(pcvModule, &hz);

    pcvExit();

    return {previousHz, hz};
  }
}

void msg(std::string title, std::string message) { inst::ui::mainApp->CreateShowDialog(title, message, {"OK"}, true); }

} // namespace inst::util
