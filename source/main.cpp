#include "ui/MainApplication.hpp"
#include "util/error.hpp"
#include "util/util.hpp"
#include <switch.h>

int main(int argc, char *argv[]) {
  inst::util::initApp();
  try {
    auto renderer_opts =
        pu::ui::render::RendererInitOptions(SDL_INIT_EVERYTHING, pu::ui::render::RendererHardwareFlags);
    renderer_opts.UseImage(pu::ui::render::IMGAllFlags);
    renderer_opts.AddDefaultAllSharedFonts();
    // renderer_opts.AddExtraDefaultFontSize(18);
    renderer_opts.UseRomfs();
    renderer_opts.SetInputPlayerCount(1);
    renderer_opts.AddInputNpadStyleTag(HidNpadStyleSet_NpadStandard);
    renderer_opts.AddInputNpadIdType(HidNpadIdType_Handheld);
    renderer_opts.AddInputNpadIdType(HidNpadIdType_No1);
    auto renderer = pu::ui::render::Renderer::New(renderer_opts);
    auto main = inst::ui::MainApplication::New(renderer);
    main->Prepare();
    main->ShowWithFadeIn();
  } catch (std::exception &e) {
    LOG_DEBUG("An error occurred:\n%s", e.what());
  }
  inst::util::deinitApp();
  return 0;
}
