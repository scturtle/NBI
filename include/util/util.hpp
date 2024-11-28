#pragma once
#include <filesystem>
#include <tuple>
#include <vector>

#include <pu/ui/render/render_SDL2.hpp>

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::icon {
constexpr const char *fail = "romfs:/images/icons/fail.png";
constexpr const char *bin = "romfs:/images/icons/bin.png";
constexpr const char *info = "romfs:/images/icons/information.png";
constexpr const char *good = "romfs:/images/icons/good.png";
constexpr const char *upper = "romfs:/images/icons/folder-upload.png";
constexpr const char *folder = "romfs:/images/icons/folder.png";
constexpr const char *install = "romfs:/images/icons/install.png";
} // namespace inst::icon

namespace inst::util {
void initApp();
void deinitApp();
void initInstallServices();
void deinitInstallServices();
std::tuple<double, double, double> getSpaceInfo(const char *path);
std::vector<std::filesystem::path> getDirectoryFiles(const std::string &dir,
                                                     const std::vector<std::string> &extensions);
std::vector<std::filesystem::path> getDirsAtPath(const std::string &dir);
std::string shortenString(std::string ourString, int ourLength, bool isFile);
std::vector<uint32_t> setClockSpeed(int deviceToClock, uint32_t clockSpeed);

inline pu::sdl2::TextureHandle::Ref LoadTexture(const std::string &path) {
  return pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(path));
}

void msg(std::string title, std::string message);

} // namespace inst::util
