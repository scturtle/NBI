#include <thread>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "sigInstall.hpp"
#include "HDInstall.hpp"
#include "data/buffered_placeholder_writer.hpp"
#include "nx/usbhdd.h"
#include "usbhsfs.h"
#include "util/theme.hpp"
#include <sys/statvfs.h>

// Include sdl2 headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
Mix_Music* audio = NULL;

#define COLOR(hex) pu::ui::Color::FromHex(hex)

int statvfs(const char* path, struct statvfs* buf);
s32 prev_touchcount = 0;

double GetAvailableSpace(const char* path)
{
	struct statvfs stat;

	if (statvfs(path, &stat) != 0) {
		// error happens, just quits here
		return -1;
	}

	// the available size is f_bsize * f_bavail
	return stat.f_bsize * stat.f_bavail;
}

double amountOfDiskSpaceUsed(const char* path)
{
	struct statvfs stat;

	if (statvfs(path, &stat) != 0) {
		// error happens, just quits here
		return -1;
	}
	const auto total = static_cast<unsigned long>(stat.f_blocks);
	const auto available = static_cast<unsigned long>(stat.f_bavail);
	const auto availableToRoot = static_cast<unsigned long>(stat.f_bfree);
	const auto used = total - availableToRoot;
	const auto nonRootTotal = used + available;
	return 100.0 * static_cast<double>(used) / static_cast<double>(nonRootTotal);
}

double totalsize(const char* path)
{
	struct statvfs stat;

	if (statvfs(path, &stat) != 0) {
		// error happens, just quits here
		return -1;
	}
	return stat.f_blocks * stat.f_frsize;
}

namespace inst::ui {
	extern MainApplication* mainApp;
	bool appletFinished = false;
	bool updateFinished = false;
	std::string maini_root = inst::config::appDir + "/theme";
	bool maini_theme = util::themeit(maini_root); //check if we have a previous theme directory first.

	void mathstuff() {
		double math = (GetAvailableSpace("./") / 1024) / 1024; //megabytes
		float math2 = ((float)math / 1024); //gigabytes

		double used = (amountOfDiskSpaceUsed("./")); //same file path as sdmc

		double total = (totalsize("sdmc:/") / 1024) / 1024; //megabytes
		float total2 = ((float)total / 1024); //gigabytes
		//
		float GB = math2;
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << GB; //only show 2 decimal places
		std::string freespace = stream.str();


		float GB2 = total2;
		std::stringstream stream2;
		stream2 << std::fixed << std::setprecision(2) << GB2; //only show 2 decimal places
		std::string sdsize = stream2.str();

		//printf("\nSdCard Free Space in MB: %li", math);
		//printf("\nSdCard Free Space in GB: %.2f", math2);
		std::stringstream stream3;
		stream3 << std::fixed << std::setprecision(2) << used; //only show 2 decimal places
		std::string percent = stream3.str();

		//unmount sd here and mount system....
		//fsdevUnmountDevice("sdmc");
		FsFileSystem nandFS;
		fsOpenBisFileSystem(&nandFS, FsBisPartitionId_User, "");
		fsdevMountDevice("user", nandFS);

		double math3 = (GetAvailableSpace("user:/") / 1024) / 1024; //megabytes
		float math4 = ((float)math3 / 1024); //gigabytes

		double used2 = (amountOfDiskSpaceUsed("user:/")); //same file path as sdmc

		double total3 = (totalsize("user:/") / 1024) / 1024; //megabytes
		float total4 = ((float)total3 / 1024); //gigabytes
		//
		float GB3 = math4;
		std::stringstream stream4;
		stream4 << std::fixed << std::setprecision(2) << GB3; //only show 2 decimal places
		std::string freespace2 = stream4.str();


		float GB4 = total4;
		std::stringstream stream5;
		stream5 << std::fixed << std::setprecision(2) << GB4; //only show 2 decimal places
		std::string sdsize2 = stream5.str();

		//printf("\nSdCard Free Space in MB: %li", math);
		//printf("\nSdCard Free Space in GB: %.2f", math2);
		std::stringstream stream6;
		stream6 << std::fixed << std::setprecision(2) << used2; //only show 2 decimal places
		std::string percent2 = stream6.str();

		//unmount user now as we already know how much space we have	
		fsdevUnmountDevice("user");

		std::string Info = ("usage.system_size"_lang + sdsize2 + "usage.gb"_lang + "usage.freespace"_lang + freespace2 + "usage.gb"_lang + "usage.percent_used"_lang + percent2 + "usage.percent"_lang + "usage.sd_size"_lang + sdsize + "usage.gb"_lang + "usage.sd_space"_lang + freespace + "usage.gb"_lang + "usage.sd_used"_lang + percent + "usage.percent_symbol"_lang);

		std::string drive = "romfs:/images/icons/drive.png";
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.drive"_theme)) {
			drive = inst::config::appDir + "icons_others.drive"_theme;
		}
		inst::ui::mainApp->CreateShowDialog("usage.space_info"_lang, Info, { "common.ok"_lang }, true, "romfs:/images/icons/drive.png");
	}

	void playmusic() {
		SDL_Init(SDL_INIT_AUDIO);
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);
		std::string loadsound = "romfs:/bluesong.mod";
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "audio.music"_theme)) {
			loadsound = inst::config::appDir + "audio.music"_theme;
		}
		const char* x = loadsound.c_str();
		audio = Mix_LoadMUS(x);
		if (audio != NULL) {
			Mix_PlayMusic(audio, -1); //-1 loop "infinitely"
		}
	}

	void mainMenuThread() {
		bool menuLoaded = mainApp->IsShown();
		if (!appletFinished && appletGetAppletType() == AppletType_LibraryApplet) {
			tin::data::NUM_BUFFER_SEGMENTS = 2;
			if (menuLoaded) {
				inst::ui::appletFinished = true;
				std::string information = "romfs:/images/icons/information.png";
				if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.information"_theme)) {
					information = inst::config::appDir + "icons_others.information"_theme;
				}
				mainApp->CreateShowDialog("main.applet.title"_lang, "main.applet.desc"_lang, { "common.ok"_lang }, true, information);
			}
		}
		else if (!appletFinished) {
			inst::ui::appletFinished = true;
			tin::data::NUM_BUFFER_SEGMENTS = 128;
		}
		if (!updateFinished && (!inst::config::autoUpdate || inst::util::getIPAddress() == "1.0.0.127")) updateFinished = true;

		if (!updateFinished && menuLoaded && inst::config::updateInfo.size()) {
			updateFinished = true;
			optionsPage::askToUpdate(inst::config::updateInfo);
		}
	}

	MainPage::MainPage() : Layout::Layout() {
		std::string default_background = inst::config::appDir + "bg_images.default_background"_theme;
		std::string main_top = inst::config::appDir + "bg_images.main_top"_theme;

		std::string icons_sd = inst::config::appDir + "icons_mainmenu.sd"_theme;
		std::string icons_net = inst::config::appDir + "icons_mainmenu.net"_theme;
		std::string icons_usb = inst::config::appDir + "icons_mainmenu.usb"_theme;
		std::string icons_hdd = inst::config::appDir + "icons_mainmenu.hdd"_theme;
		std::string icons_hdd_connected = inst::config::appDir + "icons_mainmenu.hdd_connected"_theme;
		std::string icons_settings = inst::config::appDir + "icons_mainmenu.settings"_theme;
		std::string icons_exit = inst::config::appDir + "icons_mainmenu.exit"_theme;

		std::string bg_colour = "colour.background"_theme;
		std::string tbar_colour = "colour.topbar"_theme;
		std::string bbar_colour = "colour.bottombar"_theme;
		std::string bottombar_text = "colour.bottombar_text"_theme;
		std::string text_colour = "colour.main_text"_theme;
		std::string background_overlay1 = "colour.background_overlay1"_theme;
		std::string background_overlay2 = "colour.background_overlay2"_theme;
		std::string focus = "colour.focus"_theme;
		std::string scrollbar = "colour.scrollbar"_theme;


		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->SetBackgroundColor(COLOR(bg_colour));
		else this->SetBackgroundColor(COLOR("#000000FF"));

		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR(tbar_colour));
		else this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#000000FF"));

		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR(bbar_colour));
		else this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR("#000000FF"));

		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(main_top)) this->titleImage = Image::New(0, 0, (main_top));
		else this->titleImage = Image::New(0, 0, "romfs:/images/Main.png");

		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(default_background)) this->SetBackgroundImage(default_background);
		else this->SetBackgroundImage("romfs:/images/Background.png");

		this->butText = TextBlock::New(10, 678, "main.buttons"_lang);
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->butText->SetColor(COLOR(bottombar_text));
		else this->butText->SetColor(COLOR("#FFFFFFFF"));

		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->optionMenu = pu::ui::elm::Menu::New(0, 95, 1280, COLOR(background_overlay1), COLOR(background_overlay2), 94, 6);
		else this->optionMenu = pu::ui::elm::Menu::New(0, 95, 1280, COLOR("#FFFFFF00"), COLOR("#4f4f4d33"), 94, 6);

		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->optionMenu->SetItemsFocusColor(COLOR(focus));
		else this->optionMenu->SetItemsFocusColor(COLOR("#4f4f4dAA"));

		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->optionMenu->SetScrollbarColor(COLOR(scrollbar));
		else this->optionMenu->SetScrollbarColor(COLOR("#1A1919FF"));

		this->installMenuItem = pu::ui::elm::MenuItem::New("main.menu.sd"_lang);
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->installMenuItem->SetColor(COLOR(text_colour));
		else this->installMenuItem->SetColor(COLOR("#FFFFFFFF"));
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(icons_sd)) this->installMenuItem->SetIcon(icons_sd);
		else this->installMenuItem->SetIcon("romfs:/images/icons/micro-sd.png");

		this->netInstallMenuItem = pu::ui::elm::MenuItem::New("main.menu.net"_lang);
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->netInstallMenuItem->SetColor(COLOR(text_colour));
		else this->netInstallMenuItem->SetColor(COLOR("#FFFFFFFF"));
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(icons_net)) this->netInstallMenuItem->SetIcon(icons_net);
		else this->netInstallMenuItem->SetIcon("romfs:/images/icons/cloud-download.png");

		this->usbInstallMenuItem = pu::ui::elm::MenuItem::New("main.menu.usb"_lang);
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->usbInstallMenuItem->SetColor(COLOR(text_colour));
		else this->usbInstallMenuItem->SetColor(COLOR("#FFFFFFFF"));
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(icons_usb)) this->usbInstallMenuItem->SetIcon(icons_usb);
		else this->usbInstallMenuItem->SetIcon("romfs:/images/icons/usb-port.png");

		this->HdInstallMenuItem = pu::ui::elm::MenuItem::New("main.menu.hdd"_lang);
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->HdInstallMenuItem->SetColor(COLOR(text_colour));
		else this->HdInstallMenuItem->SetColor(COLOR("#FFFFFFFF"));
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(icons_hdd)) this->HdInstallMenuItem->SetIcon(icons_hdd);
		else this->HdInstallMenuItem->SetIcon("romfs:/images/icons/usb-hd.png");

		this->settingsMenuItem = pu::ui::elm::MenuItem::New("main.menu.set"_lang);
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->settingsMenuItem->SetColor(COLOR(text_colour));
		else this->settingsMenuItem->SetColor(COLOR("#FFFFFFFF"));
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(icons_settings)) this->settingsMenuItem->SetIcon(icons_settings);
		else this->settingsMenuItem->SetIcon("romfs:/images/icons/settings.png");

		this->exitMenuItem = pu::ui::elm::MenuItem::New("main.menu.exit"_lang);
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->exitMenuItem->SetColor(COLOR(text_colour));
		else this->exitMenuItem->SetColor(COLOR("#FFFFFFFF"));
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(icons_exit)) this->exitMenuItem->SetIcon(icons_exit);
		else this->exitMenuItem->SetIcon("romfs:/images/icons/exit-run.png");

		this->Add(this->topRect);
		this->Add(this->botRect);
		this->Add(this->titleImage);
		this->Add(this->butText);
		this->optionMenu->AddItem(this->installMenuItem);
		this->optionMenu->AddItem(this->netInstallMenuItem);
		this->optionMenu->AddItem(this->usbInstallMenuItem);
		this->optionMenu->AddItem(this->HdInstallMenuItem);
		this->optionMenu->AddItem(this->settingsMenuItem);
		this->optionMenu->AddItem(this->exitMenuItem);
		if (nx::hdd::count() && nx::hdd::rootPath()) {
			if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(icons_hdd_connected)) this->hdd = Image::New(1156, 669, icons_hdd_connected);
			else this->hdd = Image::New(1156, 669, "romfs:/images/icons/usb-hd-connected.png");
			this->Add(this->hdd);
		}
		this->Add(this->optionMenu);
		this->AddRenderCallback(mainMenuThread);
		if (inst::config::useMusic) {
			std::thread t(&playmusic);
			t.join();
		}
	}

	void MainPage::installMenuItem_Click() {
		mainApp->sdinstPage->drawMenuItems(true, "sdmc:/");
		mainApp->sdinstPage->menu->SetSelectedIndex(0);
		mainApp->LoadLayout(mainApp->sdinstPage);
	}

	void MainPage::netInstallMenuItem_Click() {
		if (inst::util::getIPAddress() == "1.0.0.127") {
			if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.information"_theme)) {
				inst::ui::mainApp->CreateShowDialog("main.net.title"_lang, "main.net.desc"_lang, { "common.ok"_lang }, true, inst::config::appDir + "icons_others.information"_theme);
			}
			else inst::ui::mainApp->CreateShowDialog("main.net.title"_lang, "main.net.desc"_lang, { "common.ok"_lang }, true, "romfs:/images/icons/information.png");
			return;
		}
		mainApp->netinstPage->startNetwork();
	}

	void MainPage::usbInstallMenuItem_Click() {
		std::string usb = "romfs:/images/icons/usb.png";
		if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.usb"_theme)) {
			usb = inst::config::appDir + "icons_others.usb"_theme;
		}
		if (!inst::config::usbAck) {
			if (mainApp->CreateShowDialog("main.usb.warn.title"_lang, "main.usb.warn.desc"_lang, { "common.ok"_lang, "main.usb.warn.opt1"_lang }, false, usb) == 1) {
				inst::config::usbAck = true;
				inst::config::setConfig();
			}
		}
		if (inst::util::getUsbState() == 5) mainApp->usbinstPage->startUsb();
		else mainApp->CreateShowDialog("main.usb.error.title"_lang, "main.usb.error.desc"_lang, { "common.ok"_lang }, true, usb);
	}

	void MainPage::HdInstallMenuItem_Click() {
		if (nx::hdd::count() && nx::hdd::rootPath()) {
			mainApp->HDinstPage->drawMenuItems(true, nx::hdd::rootPath());
			mainApp->HDinstPage->menu->SetSelectedIndex(0);
			mainApp->LoadLayout(mainApp->HDinstPage);
		}
		else {
			std::string drive = "romfs:/images/icons/drive.png";
			if (inst::ui::maini_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.drive"_theme)) {
				drive = inst::config::appDir + "icons_others.drive"_theme;
			}
			inst::ui::mainApp->CreateShowDialog("main.hdd.title"_lang, "main.hdd.notfound"_lang, { "common.ok"_lang }, true, drive);
		}
	}

	void MainPage::exitMenuItem_Click() {
		mainApp->FadeOut();
		mainApp->Close();
	}

	void MainPage::settingsMenuItem_Click() {
		mainApp->LoadLayout(mainApp->optionspage);
	}

	void MainPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {

		if (((Down & HidNpadButton_Plus) || (Down & HidNpadButton_Minus) || ((Held & HidNpadButton_L) && (Down & HidNpadButton_R)) || ((Down & HidNpadButton_L) && (Held & HidNpadButton_R))) && mainApp->IsShown()) {
			mainApp->FadeOut();
			mainApp->Close();
		}

		HidTouchScreenState state = { 0 };

		if (hidGetTouchScreenStates(&state, 1)) {

			if ((Down & HidNpadButton_A) || (state.count != prev_touchcount))
			{
				prev_touchcount = state.count;

				if (prev_touchcount != 1) {
					int menuindex = this->optionMenu->GetSelectedIndex();
					switch (menuindex) {
					case 0:
						this->installMenuItem_Click();
						break;
					case 1:
						this->netInstallMenuItem_Click();
						break;
					case 2:
						MainPage::usbInstallMenuItem_Click();
						break;
					case 3:
						MainPage::HdInstallMenuItem_Click();
						break;
					case 4:
						MainPage::settingsMenuItem_Click();
						break;
					case 5:
						MainPage::exitMenuItem_Click();
						break;
					default:
						break;
					}
				}
			}
		}

		if (Down & HidNpadButton_X) {
		}

		if (Up & HidNpadButton_A) {
		}

		if (Down & HidNpadButton_Y) {
			mathstuff();
		}

		if (Down & HidNpadButton_ZL) {
		}

		if (Down & HidNpadButton_ZR) {
		}

		if (Down & Held & HidNpadButton_L) {
			std::thread t(&playmusic);
			t.join();   // main thread waits for the thread t to finish
		}

		if (Down & Held & HidNpadButton_R) {
			Mix_FreeMusic(audio);
			audio = NULL;
			Mix_Quit();
		}
	}
}