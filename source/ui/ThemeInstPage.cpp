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
#include <sstream>
#include <cstring>
#include <iostream>

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
	extern MainApplication* mainApp;
	s32 xxxx = 0;
	int myindex;
	int installing = 0;

	std::string httplastUrl2 = "http://";
	std::string lastFileID2 = "";
	std::string sourceString2 = "";
	std::string ourPath = inst::config::appDir + "/temp_download.zip";

	ThemeInstPage::ThemeInstPage() : Layout::Layout() {
		this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#00000080"));
		this->SetBackgroundColor(COLOR("#000000FF"));
		this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#000000FF"));
		this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR("#000000FF"));

		if (inst::config::useTheme) {
			if (std::filesystem::exists(inst::config::appDir + "/images/Net.png")) this->titleImage = Image::New(0, 0, (inst::config::appDir + "/images/Net.png"));
			else this->titleImage = Image::New(0, 0, "romfs:/images/Net.png");
			if (std::filesystem::exists(inst::config::appDir + "/images/Background.png")) this->SetBackgroundImage(inst::config::appDir + "/images/Background.png");
			else this->SetBackgroundImage("romfs:/images/Background.png");
			this->appVersionText = TextBlock::New(1210, 680, "");
		}
		else {
			this->SetBackgroundImage("romfs:/images/Background.png");
			this->titleImage = Image::New(0, 0, "romfs:/images/Net.png");
			this->appVersionText = TextBlock::New(1210, 680, "");
		}
		this->appVersionText->SetColor(COLOR("#FFFFFFFF"));
		this->pageInfoText = TextBlock::New(10, 109, "");
		this->pageInfoText->SetFont(pu::ui::MakeDefaultFontName(30));
		this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
		this->butText = TextBlock::New(10, 678, "");
		this->butText->SetColor(COLOR("#FFFFFFFF"));
		this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), COLOR("#4f4f4d33"), 84, (506 / 84));
		this->menu->SetItemsFocusColor(COLOR("#4f4f4dAA"));
		this->menu->SetScrollbarColor(COLOR("#1A1919FF"));
		this->infoImage = Image::New(453, 292, "romfs:/images/icons/lan-connection-waiting.png");
		this->Add(this->topRect);
		this->Add(this->infoRect);
		this->Add(this->botRect);
		this->Add(this->titleImage);
		this->Add(this->appVersionText);
		this->Add(this->butText);
		this->Add(this->pageInfoText);
		this->Add(this->menu);
		this->Add(this->infoImage);
		this->installBar = pu::ui::elm::ProgressBar::New(10, 675, 1260, 35, 100.0f);
		this->installBar->SetBackgroundColor(COLOR("#000000FF"));
		this->installBar->SetProgressColor(COLOR("#00FF00FF"));
		this->Add(this->installBar);
	}

	void ThemeInstPage::drawMenuItems_withext(bool clearItems) {
		myindex = this->menu->GetSelectedIndex(); //store index so when page redraws we can get the last item we checked.
		if (clearItems) this->selectedUrls = {};
		if (clearItems) this->alternativeNames = {};
		mainApp->ThemeinstPage->installBar->SetProgress(0);
		mainApp->ThemeinstPage->installBar->SetVisible(false);
		std::string itm;

		this->menu->ClearItems();
		for (auto& urls : this->ourUrls) {
			itm = inst::util::shortenString(inst::util::formatUrlString(urls), 56, true);
			auto ourEntry = pu::ui::elm::MenuItem::New(itm);
			ourEntry->SetColor(COLOR("#FFFFFFFF"));
			ourEntry->SetIcon("romfs:/images/icons/checkbox-blank-outline.png");
			long unsigned int i;
			for (i = 0; i < this->selectedUrls.size(); i++) {
				if (this->selectedUrls[i] == urls) {
					ourEntry->SetIcon("romfs:/images/icons/check-box-outline.png");
				}
			}
			this->menu->AddItem(ourEntry);
			this->menu->SetSelectedIndex(myindex); //jump to the index we saved from above
		}
	}

	void ThemeInstPage::drawMenuItems(bool clearItems) {
		myindex = this->menu->GetSelectedIndex(); //store index so when page redraws we can get the last item we checked.
		if (clearItems) this->selectedUrls = {};
		if (clearItems) this->alternativeNames = {};
		std::string itm;
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
			ourEntry->SetColor(COLOR("#FFFFFFFF"));
			ourEntry->SetIcon("romfs:/images/icons/checkbox-blank-outline.png");
			long unsigned int i;
			for (i = 0; i < this->selectedUrls.size(); i++) {
				if (this->selectedUrls[i] == urls) {
					ourEntry->SetIcon("romfs:/images/icons/check-box-outline.png");
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
						//extraction failed: still to do - put check in zip function...
					}
				}
				else {
					inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("theme.failed"_lang);
					installing = 0;
					inst::ui::mainApp->ThemeinstPage->setInstBarPerc(0);
					mainApp->ThemeinstPage->installBar->SetVisible(false);
					inst::ui::mainApp->CreateShowDialog("theme.theme_error"_lang, "theme.theme_error_info"_lang, { "common.ok"_lang }, true, "romfs:/images/icons/fail.png");
					return;
				}
				std::filesystem::remove(ourPath);
				if (didExtract) {
					inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("theme.extracted"_lang);
					int close = inst::ui::mainApp->CreateShowDialog("theme.installed"_lang, "theme.restart"_lang, { "sig.later"_lang, "sig.restart"_lang }, false, "romfs:/images/icons/good.png");
					inst::ui::mainApp->ThemeinstPage->setInstBarPerc(0);
					mainApp->ThemeinstPage->installBar->SetVisible(false);
					if (close != 0) {
						mainApp->FadeOut();
						mainApp->Close();
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
			inst::ui::mainApp->CreateShowDialog("theme.wait"_lang, "theme.trying"_lang, { "common.ok"_lang }, true, "romfs:/images/icons/information.png");
		}
		installing = 0;
	}

	void ThemeInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {

		if (Down & HidNpadButton_B) {
			if (installing != 1) {
				mainApp->LoadLayout(mainApp->optionspage);
			}
			else {
				inst::ui::mainApp->CreateShowDialog("theme.wait"_lang, "theme.trying"_lang, { "common.ok"_lang }, true, "romfs:/images/icons/information.png");
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
							if (this->menu->GetItems()[myindex]->GetIconPath() == "romfs:/images/icons/check-box-outline.png") {
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

		//don't show file extensions
		if (Down & HidNpadButton_Left) {
			if (installing != 1) {
				this->drawMenuItems(true);
			}
		}

		//show file extensions
		if (Down & HidNpadButton_Right) {
			if (installing != 1) {
				this->drawMenuItems_withext(true);
			}
		}
	}
}