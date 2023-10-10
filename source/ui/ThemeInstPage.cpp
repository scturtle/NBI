#include <filesystem>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/ThemeInstPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/curl.hpp"
#include "util/lang.hpp"
#include "ThemeInstall.hpp"
#include "util/unzip.hpp"
#include "ui/instPage.hpp"
#include "util/theme.hpp"
#include <sstream>
#include <cstring>
#include <iostream>
#include <thread>

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
	extern MainApplication* mainApp;
	s32 xxxx = 0;
	int myindex;
	int installing = 0;
	std::string root = inst::config::appDir + "/theme";
	bool istheme = util::themeit(root); //check if we have a previous theme directory first.
	

	std::string checked_theme = "romfs:/images/icons/check-box-outline.png";
	std::string unchecked_theme = "romfs:/images/icons/checkbox-blank-outline.png";

	void checkbox_theme() {
		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.checkbox-checked"_theme)) {
			checked_theme = inst::config::appDir + "icons_others.checkbox-checked"_theme;
		}
		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.checkbox-empty"_theme)) {
			unchecked_theme = inst::config::appDir + "icons_others.checkbox-empty"_theme;
		}
	}

	std::string httplastUrl2 = "http://";
	std::string lastFileID2 = "";
	std::string sourceString2 = "";
	std::string ourPath = inst::config::appDir + "/temp_download.zip";

	ThemeInstPage::ThemeInstPage() : Layout::Layout() {
		std::string default_background = inst::config::appDir + "bg_images.default_background"_theme;
		std::string theme_top = inst::config::appDir + "bg_images.theme_top"_theme;
		std::string bg_colour = "colour.background"_theme;
		std::string tbar_colour = "colour.topbar"_theme;
		std::string bbar_colour = "colour.bottombar"_theme;
		std::string infoRect_colour = "colour.inforect"_theme;
		std::string pageinfo_colour = "colour.pageinfo_text"_theme;
		std::string bottombar_text = "colour.bottombar_text"_theme;
		std::string background_overlay1 = "colour.background_overlay1"_theme;
		std::string background_overlay2 = "colour.background_overlay2"_theme;
		std::string focus = "colour.focus"_theme;
		std::string scrollbar = "colour.scrollbar"_theme;
		std::string waiting = inst::config::appDir + "icons_others.waiting_lan"_theme;
		std::string progress_bg_colour = "colour.progress_bg"_theme;
		std::string progress_fg_colour = "colour.progress_fg"_theme;

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR(infoRect_colour));
		else this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#00000080"));

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->SetBackgroundColor(COLOR(bg_colour));
		else this->SetBackgroundColor(COLOR("#000000FF"));

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR(tbar_colour));
		else this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#000000FF"));

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR(bbar_colour));
		else this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR("#000000FF"));

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(theme_top)) this->titleImage = Image::New(0, 0, (theme_top));
		else this->titleImage = Image::New(0, 0, "romfs:/images/Net.png");

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(default_background)) this->SetBackgroundImage(default_background);
		else this->SetBackgroundImage("romfs:/images/Background.png");

		this->pageInfoText = TextBlock::New(10, 109, "inst.hd.top_info"_lang);
		this->pageInfoText->SetFont(pu::ui::MakeDefaultFontName(30));

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->pageInfoText->SetColor(COLOR(pageinfo_colour));
		else this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));

		this->butText = TextBlock::New(10, 678, "inst.hd.buttons"_lang);
		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->butText->SetColor(COLOR(bottombar_text));
		else this->butText->SetColor(COLOR("#FFFFFFFF"));

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR(background_overlay1), COLOR(background_overlay2), 84, (506 / 84));
		else this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), COLOR("#4f4f4d33"), 84, (506 / 84));

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu->SetItemsFocusColor(COLOR(focus));
		else this->menu->SetItemsFocusColor(COLOR("#4f4f4dAA"));

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu->SetScrollbarColor(COLOR(scrollbar));
		else this->menu->SetScrollbarColor(COLOR("#1A1919FF"));

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(waiting)) this->infoImage = Image::New(453, 292, waiting);
		else this->infoImage = Image::New(453, 292, "romfs:/images/icons/lan-connection-waiting.png");

		this->Add(this->topRect);
		this->Add(this->infoRect);
		this->Add(this->botRect);
		this->Add(this->titleImage);
		this->Add(this->butText);
		this->Add(this->pageInfoText);
		this->Add(this->menu);
		this->Add(this->infoImage);

		this->installBar = pu::ui::elm::ProgressBar::New(10, 675, 1260, 35, 100.0f);

		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->installBar->SetBackgroundColor(COLOR(progress_bg_colour));
		else this->installBar->SetBackgroundColor(COLOR("#000000FF"));
		if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->installBar->SetProgressColor(COLOR(progress_fg_colour));
		else this->installBar->SetProgressColor(COLOR("#565759FF"));

		this->Add(this->installBar);
		checkbox_theme();
	}

	void ThemeInstPage::drawMenuItems(bool clearItems) {
		myindex = this->menu->GetSelectedIndex(); //store index so when page redraws we can get the last item we checked.
		if (clearItems) this->selectedUrls = {};
		if (clearItems) this->alternativeNames = {};
		std::string itm;
		std::string text_colour = "colour.main_text"_theme;
		mainApp->ThemeinstPage->installBar->SetProgress(0);
		mainApp->ThemeinstPage->installBar->SetVisible(false);

		this->menu->ClearItems();
		for (auto& urls : this->ourUrls) {
			//Alt code to remove file extension from the item shown on the screen
			std::string base_filename = urls.substr(urls.find_last_of("/") + 1); //just get the filename
			std::string::size_type const p(base_filename.find_last_of('.'));
			std::string file_without_extension = base_filename.substr(0, p); //strip of file extension
			itm = inst::util::shortenString(inst::util::formatUrlString(file_without_extension), 56, true);
			//itm = inst::util::shortenString(inst::util::formatUrlString(urls), 56, true); 
			auto ourEntry = pu::ui::elm::MenuItem::New(itm);
			if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ourEntry->SetColor(COLOR(text_colour));
			else ourEntry->SetColor(COLOR("#FFFFFFFF"));
			ourEntry->SetIcon(unchecked_theme);

			long unsigned int i;
			for (i = 0; i < this->selectedUrls.size(); i++) {
				if (this->selectedUrls[i] == urls) {
					ourEntry->SetIcon(checked_theme);
				}
			}
			this->menu->AddItem(ourEntry);
			this->menu->SetSelectedIndex(myindex); //jump to the index we saved from above
		}
	}

	void ThemeInstPage::setInstBarPerc(double ourPercent) {
		mainApp->ThemeinstPage->installBar->SetProgress(ourPercent);
		if (ourPercent >= 1 && ourPercent != 100) {
			mainApp->ThemeinstPage->installBar->SetVisible(true);
		}
		else {
			mainApp->ThemeinstPage->installBar->SetVisible(false);
		}
		//
		if (installing == 1) {
			std::stringstream x;
			x << ourPercent;
			inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("theme.downloading"_lang + x.str() + "theme.percent"_lang);
			if (x.str() == "100") {
				inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("theme.extracting"_lang);
			}
		}
		else {
			inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("theme.theme_top_info"_lang);
		}
		//
		mainApp->CallForRender();
	}

	void ThemeInstPage::startNetwork() {
		this->butText->SetText("theme.please_wait"_lang);
		this->menu->SetVisible(false);
		this->menu->ClearItems();
		this->infoImage->SetVisible(true);
		mainApp->LoadLayout(mainApp->ThemeinstPage);
		this->ourUrls = ThemeInstStuff::OnSelected();
		mainApp->ThemeinstPage->installBar->SetVisible(false);

		if (!this->ourUrls.size()) {
			mainApp->LoadLayout(mainApp->optionspage);
			return;
		}

		else {
			mainApp->CallForRender(); // If we re-render a few times during this process the main screen won't flicker
			sourceString2 = "inst.net.source_string"_lang;
			netConnected2 = true;
			this->pageInfoText->SetText("theme.theme_top_info"_lang);
			this->butText->SetText("theme.buttons2"_lang);
			this->drawMenuItems(true);
			mainApp->CallForRender();
			this->infoImage->SetVisible(false); //
			this->menu->SetVisible(true);
			this->menu->SetSelectedIndex(0); //when page first loads jump to start of the menu 
		}
		return;
	}

	void ThemeInstPage::selectTitle(int selectedIndex) {
		if (installing != 1) {
			for (long unsigned int i = 0; i < this->selectedUrls.size(); i++) {
				inst::ui::mainApp->ThemeinstPage->setInstBarPerc(0);
				ourPath = inst::config::appDir + "/temp_download.zip";
				installing = 1;
				bool didDownload = inst::curl::downloadFile(selectedUrls[0], ourPath.c_str(), 0, true);
				bool didExtract = false;
				if (didDownload) {
					inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("theme.complete"_lang);
					try {
						didExtract = inst::zip::extractFile(ourPath, "sdmc:/");
					}
					catch (...) {
						//https://www.geeksforgeeks.org/multithreading-in-cpp/
						//extraction failed: still to do - put check in zip function...
					}
				}
				else {
					if (inst::config::useSound) {
						std::string audioPath = "romfs:/audio/fail.mp3";
						std::string fail = inst::config::appDir + "audio.fail"_theme;
						if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(fail)) audioPath = (fail);
						std::thread audioThread(inst::util::playAudio, audioPath);
						audioThread.join();
					}
					inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("theme.failed"_lang);
					installing = 0;
					inst::ui::mainApp->ThemeinstPage->setInstBarPerc(0);
					mainApp->ThemeinstPage->installBar->SetVisible(false);

					std::string fail = "romfs:/images/icons/fail.png";
					if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.fail"_theme)) {
						fail = inst::config::appDir + "icons_others.fail"_theme;
					}
					inst::ui::mainApp->CreateShowDialog("theme.theme_error"_lang, "theme.theme_error_info"_lang, { "common.ok"_lang }, true, fail);
					return;
				}
				std::filesystem::remove(ourPath);
				if (didExtract) {
					if (inst::config::useSound) {
						std::string audioPath = "romfs:/audio/pass.mp3";
						std::string pass = inst::config::appDir + "audio.pass"_theme;
						if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(pass)) audioPath = (pass);
						std::thread audioThread(inst::util::playAudio, audioPath);
						audioThread.join();
					}
					inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("theme.extracted"_lang);
					std::string good = "romfs:/images/icons/good.png";
					if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.good"_theme)) {
						good = inst::config::appDir + "icons_others.good"_theme;
					}
					int close = inst::ui::mainApp->CreateShowDialog("theme.installed"_lang, "theme.restart"_lang, { "sig.later"_lang, "sig.restart"_lang }, true, good);
					inst::ui::mainApp->ThemeinstPage->setInstBarPerc(0);
					mainApp->ThemeinstPage->installBar->SetVisible(false);
					if (close) {
						mainApp->FadeOut();
						mainApp->Close();
					}
					else {
						inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("theme.theme_top_info"_lang);
						installing = 0;
						return;
					}
				}
				else {
					inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("theme.retry"_lang);
					installing = 0;
					inst::ui::mainApp->ThemeinstPage->setInstBarPerc(0);
					mainApp->ThemeinstPage->installBar->SetVisible(false);
					return;

				}
				installing = 0;
				inst::ui::mainApp->ThemeinstPage->setInstBarPerc(0);
				mainApp->ThemeinstPage->installBar->SetVisible(false);
				return;
			}
		}
		else {
			std::string information = "romfs:/images/icons/information.png";
			if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.information"_theme)) {
				information = inst::config::appDir + "icons_others.good"_theme;
			}
			inst::ui::mainApp->CreateShowDialog("theme.wait"_lang, "theme.trying"_lang, { "common.ok"_lang }, true, information);
		}
		installing = 0;
	}

	void ThemeInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {

		if (Down & HidNpadButton_B) {
			if (installing != 1) {
				mainApp->LoadLayout(mainApp->optionspage);
			}
			else {
				std::string information = "romfs:/images/icons/information.png";
				if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.information"_theme)) {
					information = inst::config::appDir + "icons_others.good"_theme;
				}
				inst::ui::mainApp->CreateShowDialog("theme.wait"_lang, "theme.trying"_lang, { "common.ok"_lang }, true, information);
			}
		}

		if (Down & HidNpadButton_X) {
			std::string theme = "romfs:/images/icons/theme.png";
			if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.theme"_theme)) {
				theme = inst::config::appDir + "icons_others.theme"_theme;
			}

			int ourResult = 0;
			std::filesystem::path rootdir = root;
			bool exists = std::filesystem::exists(rootdir);

			if (exists == true) {
				ourResult = inst::ui::mainApp->CreateShowDialog("theme.remove"_lang, "theme.delete"_lang, { "common.no"_lang, "common.yes"_lang }, true, theme);
			}

			else {
				inst::ui::mainApp->CreateShowDialog("theme.notfound"_lang, "theme.notfound2"_lang, { "common.ok"_lang }, false, theme);
				return;
			}

			if (ourResult != 0) {
				try {
					bool done = util::remove_theme(root);
					if (done == true) {
						theme = "romfs:/images/icons/theme.png"; //if the theme was removed the icon will be missing so show this instead.
						inst::ui::mainApp->CreateShowDialog("theme.notice"_lang, "theme.success"_lang, { "common.ok"_lang }, false, theme);
						mainApp->FadeOut();
						mainApp->Close();
					}
					else {
						inst::ui::mainApp->CreateShowDialog("theme.notice"_lang, "theme.notremoved"_lang, { "common.ok"_lang }, false, theme);
					}

				}
				catch (...) {
					std::string fail = "romfs:/images/icons/fail.png";
					if (istheme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.fail"_theme)) {
						fail = inst::config::appDir + "icons_others.fail"_theme;
					}
					inst::ui::mainApp->CreateShowDialog("theme.warning"_lang, "theme.error"_lang, { "common.ok"_lang }, false, fail);
				}
			}
		}

		HidTouchScreenState state = { 0 };

		if (hidGetTouchScreenStates(&state, 1)) {

			if (netConnected2) {
				if ((Down & HidNpadButton_A) || (state.count != xxxx))
				{
					xxxx = state.count;
					selectedUrls.clear();
					this->drawMenuItems(true);
					if (xxxx != 1) {
						int var = this->menu->GetItems().size();
						auto s = std::to_string(var);
						if (s != "0") {
							myindex = this->menu->GetSelectedIndex(); //store index so when page redraws we can get the last item we checked.
							if (this->menu->GetItems()[myindex]->GetIconPath() == checked_theme) {
							}
							this->selectedUrls.push_back(this->ourUrls[myindex]);
							this->drawMenuItems(false);
						}
					}
				}

				if (Down & HidNpadButton_Plus || (state.count != xxxx)) {
					this->selectTitle(myindex);
				}
			}
		}

		if (Down & HidNpadButton_ZL) {
			this->menu->SetSelectedIndex(std::max(0, this->menu->GetSelectedIndex() - 6));
		}

		if (Down & HidNpadButton_ZR) {
			this->menu->SetSelectedIndex(std::min((s32)this->menu->GetItems().size() - 1, this->menu->GetSelectedIndex() + 6));
		}

		//goto top of list	
		if (Down & HidNpadButton_L) {
			int x = this->menu->GetItems().size() - 1;
			this->menu->SetSelectedIndex(std::max(0, this->menu->GetSelectedIndex() - x));
		}

		//goto bottom of list
		if (Down & HidNpadButton_R) {
			int x = this->menu->GetItems().size() - 1;
			this->menu->SetSelectedIndex(std::min((s32)this->menu->GetItems().size() - 1, this->menu->GetSelectedIndex() + x));
		}

		//refresh/redraw the items in the list
		if (Down & HidNpadButton_Left) {
			if (installing != 1) {
				this->drawMenuItems(true);
			}
		}

		if (Down & HidNpadButton_Right) {
			if (installing != 1) {
				this->drawMenuItems(true);
			}
		}
	}
}