#include <string>
#include <thread>
// #include <sstream>
#include "ui/MainApplication.hpp"
#include "util/config.hpp"
#include "util/error.hpp"
#include "util/theme.hpp"
#include "util/util.hpp"
#include <switch.h>

namespace inst::ui {
std::string main_root = inst::config::appDir + "/theme";
bool main_theme = util::themeit(main_root); // check if we have a previous theme directory first.
} // namespace inst::ui

using namespace pu::ui::render;
int main(int argc, char *argv[]) {
  inst::util::initApp();
  try {
    Theme::Load();
    int x = 0;
    int langInt = inst::config::languageSetting;
    // Don't use custom fonts if Taiwanese or Japanese language is selected.
    // but still use custom fonts if the system language is selected.
    if (langInt != 2) {
      if (langInt != 8) {
        if (langInt != 9) {
          if (inst::ui::main_theme && inst::config::useTheme &&
              std::filesystem::exists(inst::config::appDir + "/theme/theme.json") &&
              std::filesystem::exists(inst::config::appDir + "fonts.default"_theme)) {
            x = 1;
          }
        }
      }
    }
    auto renderer_opts =
        pu::ui::render::RendererInitOptions(SDL_INIT_EVERYTHING, pu::ui::render::RendererHardwareFlags,
                                            pu::ui::render::BaseScreenWidth, pu::ui::render::BaseScreenHeight);
    renderer_opts.UseImage(pu::ui::render::IMGAllFlags);
    renderer_opts.UseAudio(pu::ui::render::MixerAllFlags);
    if (x == 1) {
      const auto default_font_path = (inst::config::appDir + "fonts.default"_theme);
      renderer_opts.AddDefaultFontPath(default_font_path);
    }
    renderer_opts.AddDefaultAllSharedFonts();
    if (x == 1) {
      std::string size = "fonts.default_size"_theme;
      int myint1 = stoi(size);
      renderer_opts.AddExtraDefaultFontSize(myint1);
    }
    renderer_opts.UseRomfs();

    renderer_opts.SetInputPlayerCount(1);
    renderer_opts.AddInputNpadStyleTag(HidNpadStyleSet_NpadStandard);
    renderer_opts.AddInputNpadIdType(HidNpadIdType_Handheld);
    renderer_opts.AddInputNpadIdType(HidNpadIdType_No1);

    auto renderer = pu::ui::render::Renderer::New(renderer_opts);

    auto main = inst::ui::MainApplication::New(renderer);
    // std::thread updateThread;
    // if (inst::config::autoUpdate && inst::util::getIPAddress() != "1.0.0.127") updateThread =
    // std::thread(inst::util::checkForAppUpdate);
    main->Prepare();
    main->ShowWithFadeIn();
    // updateThread.join();
  } catch (std::exception &e) {
    LOG_DEBUG("An error occurred:\n%s", e.what());
  }
  inst::util::deinitApp();
  return 0;
}
