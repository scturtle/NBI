/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cstring>
#include <string>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <thread>
#include <memory>
#include "sdInstall.hpp"
#include "install/install_nsp.hpp"
#include "install/install_xci.hpp"
#include "install/sdmc_xci.hpp"
#include "install/sdmc_nsp.hpp"
#include "nx/fs.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "util/error.hpp"
#include "util/config.hpp"
#include "util/util.hpp"
#include "util/lang.hpp"
#include "ui/MainApplication.hpp"
#include "ui/instPage.hpp"
#include "util/theme.hpp"

namespace inst::ui {
	extern MainApplication* mainApp;
}

namespace inst::ui {
	std::string sdi_root = inst::config::appDir + "/theme";
	bool sdi_theme = util::themeit(sdi_root); //check if we have a previous theme directory first.
}

namespace nspInstStuff {

	void installNspFromFile(std::vector<std::filesystem::path> ourTitleList, int whereToInstall)
	{
		inst::util::initInstallServices();
		inst::ui::instPage::loadInstallScreen();
		bool nspInstalled = true;
		NcmStorageId m_destStorageId = NcmStorageId_SdCard;

		if (whereToInstall) m_destStorageId = NcmStorageId_BuiltInUser;
		unsigned int titleItr;

		std::vector<int> previousClockValues;
		if (inst::config::overClock) {
			previousClockValues.push_back(inst::util::setClockSpeed(0, 1785000000)[0]);
			previousClockValues.push_back(inst::util::setClockSpeed(1, 76800000)[0]);
			previousClockValues.push_back(inst::util::setClockSpeed(2, 1600000000)[0]);
		}

		try
		{
			int togo = ourTitleList.size();
			for (titleItr = 0; titleItr < ourTitleList.size(); titleItr++) {
				auto s = std::to_string(togo);
				inst::ui::instPage::filecount("inst.info_page.queue"_lang + s);
				inst::ui::instPage::setTopInstInfoText("inst.info_page.top_info0"_lang + inst::util::shortenString(ourTitleList[titleItr].filename().string(), 40, true) + "inst.sd.source_string"_lang);
				std::unique_ptr<tin::install::Install> installTask;

				if (ourTitleList[titleItr].extension() == ".xci" || ourTitleList[titleItr].extension() == ".xcz") {
					auto sdmcXCI = std::make_shared<tin::install::xci::SDMCXCI>(ourTitleList[titleItr]);
					installTask = std::make_unique<tin::install::xci::XCIInstallTask>(m_destStorageId, inst::config::ignoreReqVers, sdmcXCI);
				}
				else {
					auto sdmcNSP = std::make_shared<tin::install::nsp::SDMCNSP>(ourTitleList[titleItr]);
					installTask = std::make_unique<tin::install::nsp::NSPInstall>(m_destStorageId, inst::config::ignoreReqVers, sdmcNSP);
				}

				LOG_DEBUG("%s\n", "Preparing installation");
				inst::ui::instPage::setInstInfoText("inst.info_page.preparing"_lang);
				inst::ui::instPage::setInstBarPerc(0);
				installTask->Prepare();
				installTask->InstallTicketCert();
				installTask->Begin(); //install nca files
				togo = (togo - 1);
			}

			inst::ui::instPage::filecount("inst.info_page.queue"_lang + "0");
		}
		catch (std::exception& e)
		{
			LOG_DEBUG("Failed to install");
			LOG_DEBUG("%s", e.what());
			fprintf(stdout, "%s", e.what());
			inst::ui::instPage::setInstInfoText("inst.info_page.failed"_lang + inst::util::shortenString(ourTitleList[titleItr].filename().string(), 42, true));
			inst::ui::instPage::setInstBarPerc(0);

			if (inst::config::useSound) {
				std::string audioPath = "";
				std::string fail = inst::config::appDir + "audio.fail"_theme;
				if (inst::ui::sdi_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(fail)) audioPath = (fail);
				else audioPath = "romfs:/audio/fail.mp3";
				std::thread audioThread(inst::util::playAudio, audioPath);
				audioThread.join();
			}

			std::string fail = "romfs:/images/icons/fail.png";
			if (inst::ui::sdi_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.fail"_theme)) {
				fail = inst::config::appDir + "icons_others.fail"_theme;
			}

			inst::ui::mainApp->CreateShowDialog("inst.info_page.failed"_lang + inst::util::shortenString(ourTitleList[titleItr].filename().string(), 42, true) + "!\n", "inst.info_page.failed_desc"_lang + "\n\n" + (std::string)e.what(), { "common.ok"_lang }, true, pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(fail)));
			nspInstalled = false;
		}

		if (previousClockValues.size() > 0) {
			inst::util::setClockSpeed(0, previousClockValues[0]);
			inst::util::setClockSpeed(1, previousClockValues[1]);
			inst::util::setClockSpeed(2, previousClockValues[2]);
		}

		if (nspInstalled) {
			inst::ui::instPage::setInstInfoText("inst.info_page.complete"_lang);
			inst::ui::instPage::setInstBarPerc(100);

			std::string bin = "romfs:/images/icons/bin.png";
			if (inst::ui::sdi_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.bin"_theme)) {
				bin = inst::config::appDir + "icons_others.bin"_theme;
			}
			std::string info = "romfs:/images/icons/information.png";
			if (inst::ui::sdi_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.information"_theme)) {
				info = inst::config::appDir + "icons_others.information"_theme;
			}
			std::string good = "romfs:/images/icons/good.png";
			if (inst::ui::sdi_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.good"_theme)) {
				good = inst::config::appDir + "icons_others.good"_theme;
			}

			if (inst::config::useSound) {
				std::string audioPath = "";
				std::string pass = inst::config::appDir + "audio.pass"_theme;
				if (inst::ui::sdi_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(pass)) {
					audioPath = (pass);
				}
				else {
					audioPath = "romfs:/audio/pass.mp3";
				}
				std::thread audioThread(inst::util::playAudio, audioPath);
				audioThread.join();
			}

			if (ourTitleList.size() > 1) {
				if (inst::config::deletePrompt) {
					if (inst::ui::mainApp->CreateShowDialog(std::to_string(ourTitleList.size()) + "inst.sd.delete_info_multi"_lang, "inst.sd.delete_desc"_lang, { "common.no"_lang,"common.yes"_lang }, false, pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(bin))) == 1) {
						for (long unsigned int i = 0; i < ourTitleList.size(); i++) {
							if (std::filesystem::exists(ourTitleList[i])) {
								std::filesystem::remove(ourTitleList[i]);
							}
						}
					}
				}
				else {
					inst::ui::mainApp->CreateShowDialog(std::to_string(ourTitleList.size()) + "inst.info_page.desc0"_lang, Language::GetRandomMsg(), { "common.ok"_lang }, true, pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(good)));
				}
			}
			else {
				if (inst::config::deletePrompt) {
					if (inst::ui::mainApp->CreateShowDialog(inst::util::shortenString(ourTitleList[0].filename().string(), 32, true) + "inst.sd.delete_info"_lang, "inst.sd.delete_desc"_lang, { "common.no"_lang,"common.yes"_lang }, false, pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(bin))) == 1) if (std::filesystem::exists(ourTitleList[0])) std::filesystem::remove(ourTitleList[0]);
				}
				else inst::ui::mainApp->CreateShowDialog(inst::util::shortenString(ourTitleList[0].filename().string(), 42, true) + "inst.info_page.desc1"_lang, Language::GetRandomMsg(), { "common.ok"_lang }, true, pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(info)));
			}
		}

		LOG_DEBUG("Done");
		inst::ui::instPage::loadMainMenu();
		inst::util::deinitInstallServices();
		return;
	}
}
