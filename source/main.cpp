#include <thread>
#include <string>
#include <switch.h>
#include "util/error.hpp"
#include "ui/MainApplication.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/theme.hpp"

using namespace pu::ui::render;
int main(int argc, char* argv[])
{
	inst::util::initApp();
	try {
		Theme::Load();
		int x = 0;
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "fonts.default"_theme)) {
			x = 1;
		}
		auto renderer_opts = pu::ui::render::RendererInitOptions(SDL_INIT_EVERYTHING, pu::ui::render::RendererHardwareFlags);
		renderer_opts.UseImage(pu::ui::render::IMGAllFlags);
		renderer_opts.UseAudio(pu::ui::render::MixerAllFlags);
		if (x == 1) {
			const auto default_font_path = (inst::config::appDir + "fonts.default"_theme);
			renderer_opts.UseTTF(default_font_path);
		}
		renderer_opts.UseTTF();
		if (x == 1) {
			std::string size = "fonts.default_size"_theme;
			int myint1 = stoi(size);
			renderer_opts.SetExtraDefaultFontSize(myint1);
		}
		renderer_opts.UseRomfs();
		auto renderer = pu::ui::render::Renderer::New(renderer_opts);

		auto main = inst::ui::MainApplication::New(renderer);
		std::thread updateThread;
		if (inst::config::autoUpdate && inst::util::getIPAddress() != "1.0.0.127") updateThread = std::thread(inst::util::checkForAppUpdate);
		main->Prepare();
		main->ShowWithFadeIn();
		updateThread.join();
	}
	catch (std::exception& e) {
		LOG_DEBUG("An error occurred:\n%s", e.what());
	}
	inst::util::deinitApp();
	return 0;
}
