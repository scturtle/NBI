#include <filesystem>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/netInstPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/curl.hpp"
#include "util/lang.hpp"
#include "netInstall.hpp"
#include "util/theme.hpp"
#include <sstream>

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
	extern MainApplication* mainApp;
	s32 xxx = 0;
	std::string checked_net = "romfs:/images/icons/check-box-outline.png";
	std::string unchecked_net = "romfs:/images/icons/checkbox-blank-outline.png";

	void checkbox_net() {
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.checkbox-checked"_theme)) {
			checked_net = inst::config::appDir + "icons_others.checkbox-checked"_theme;
		}
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.checkbox-empty"_theme)) {
			unchecked_net = inst::config::appDir + "icons_others.checkbox-empty"_theme;
		}
	}

	std::string httplastUrl = "http://";
	std::string lastFileID = "";
	std::string sourceString = "";

	netInstPage::netInstPage() : Layout::Layout() {
		std::string infoRect_colour = "colour.inforect"_theme;
		std::string bg_colour = "colour.background"_theme;
		std::string tbar_colour = "colour.topbar"_theme;
		std::string bbar_colour = "colour.bottombar"_theme;
		std::string net_top = inst::config::appDir + "bg_images.net_top"_theme;
		std::string default_background = inst::config::appDir + "bg_images.default_background"_theme;
		std::string pageinfo_colour = "colour.pageinfo_text"_theme;
		std::string bottombar_text = "colour.bottombar_text"_theme;
		std::string background_overlay1 = "colour.background_overlay1"_theme;
		std::string background_overlay2 = "colour.background_overlay2"_theme;
		std::string focus = "colour.focus"_theme;
		std::string scrollbar = "colour.scrollbar"_theme;
		std::string waiting = inst::config::appDir + "icons_others.waiting_lan"_theme;


		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR(infoRect_colour));
		else this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#00000080"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->SetBackgroundColor(COLOR(bg_colour));
		else this->SetBackgroundColor(COLOR("#000000FF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR(tbar_colour));
		else this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#000000FF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR(bbar_colour));
		else this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR("#000000FF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(net_top)) this->titleImage = Image::New(0, 0, (net_top));
		else this->titleImage = Image::New(0, 0, "romfs:/images/net.png");

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(default_background)) this->SetBackgroundImage(default_background);
		else this->SetBackgroundImage("romfs:/images/Background.png");

		this->pageInfoText = TextBlock::New(10, 109, "inst.hd.top_info"_lang);
		this->pageInfoText->SetFont(pu::ui::MakeDefaultFontName(30));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->pageInfoText->SetColor(COLOR(pageinfo_colour));
		else this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));

		this->butText = TextBlock::New(10, 678, "inst.hd.buttons"_lang);

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->butText->SetColor(COLOR(bottombar_text));
		else this->butText->SetColor(COLOR("#FFFFFFFF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR(background_overlay1), COLOR(background_overlay2), 84, (506 / 84));
		else this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), COLOR("#4f4f4d33"), 84, (506 / 84));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu->SetItemsFocusColor(COLOR(focus));
		else this->menu->SetItemsFocusColor(COLOR("#4f4f4dAA"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu->SetScrollbarColor(COLOR(scrollbar));
		else this->menu->SetScrollbarColor(COLOR("#1A1919FF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(waiting)) this->infoImage = Image::New(453, 292, waiting);
		else this->infoImage = Image::New(453, 292, "romfs:/images/icons/lan-connection-waiting.png");

		this->Add(this->topRect);
		this->Add(this->infoRect);
		this->Add(this->botRect);
		this->Add(this->titleImage);
		this->Add(this->butText);
		this->Add(this->pageInfoText);
		this->Add(this->menu);
		this->Add(this->infoImage);
		checkbox_net();
	}

	void netInstPage::drawMenuItems_withext(bool clearItems) {
		int myindex = this->menu->GetSelectedIndex(); //store index so when page redraws we can get the last item we checked.
		if (clearItems) this->selectedUrls = {};
		if (clearItems) this->alternativeNames = {};
		std::string itm;
		std::string text_colour = "colour.main_text"_theme;

		this->menu->ClearItems();
		for (auto& urls : this->ourUrls) {
			itm = inst::util::shortenString(inst::util::formatUrlString(urls), 56, true);
			auto ourEntry = pu::ui::elm::MenuItem::New(itm);

			if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ourEntry->SetColor(COLOR(text_colour));
			else ourEntry->SetColor(COLOR("#FFFFFFFF"));

			ourEntry->SetIcon(unchecked_net);
			long unsigned int i;
			for (i = 0; i < this->selectedUrls.size(); i++) {
				if (this->selectedUrls[i] == urls) {
					ourEntry->SetIcon(checked_net);
				}
			}
			this->menu->AddItem(ourEntry);
			this->menu->SetSelectedIndex(myindex); //jump to the index we saved from above
		}
	}

	void netInstPage::drawMenuItems(bool clearItems) {
		int myindex = this->menu->GetSelectedIndex(); //store index so when page redraws we can get the last item we checked.
		if (clearItems) this->selectedUrls = {};
		if (clearItems) this->alternativeNames = {};
		std::string itm;
		std::string text_colour = "colour.main_text"_theme;

		//Degug code to print out and sort a list of the files on the http server.
		/*
		for (auto& xy: this->ourUrls) {
			std::string base_filename = xy.substr(xy.find_last_of("/") + 1); //just get the filename
			std::string::size_type const p(base_filename.find_last_of('.'));
			std::string file_without_extension = base_filename.substr(0, p); //strip of file extension
			itm = inst::util::shortenString(inst::util::formatUrlString(file_without_extension), 56, true);
			modded.push_back(itm);
			std::sort(modded.begin(), modded.end());
		}

		for (auto& yz: this->modded) {
			FILE * fp;
			fp = fopen ("gamelist.txt", "a+");
			auto *info = yz.c_str();
			fprintf(fp, "%s\n", info);
			fclose(fp);
		}
		*/

		this->menu->ClearItems();
		for (auto& urls : this->ourUrls) {
			//Alt code to remove file extension from the item shown on the screen
			std::string base_filename = urls.substr(urls.find_last_of("/") + 1); //just get the filename
			std::string::size_type const p(base_filename.find_last_of('.'));
			std::string file_without_extension = base_filename.substr(0, p); //strip of file extension
			itm = inst::util::shortenString(inst::util::formatUrlString(file_without_extension), 56, true);
			//itm = inst::util::shortenString(inst::util::formatUrlString(urls), 56, true); 
			auto ourEntry = pu::ui::elm::MenuItem::New(itm);

			if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ourEntry->SetColor(COLOR(text_colour));
			else ourEntry->SetColor(COLOR("#FFFFFFFF"));

			ourEntry->SetIcon(unchecked_net);
			long unsigned int i;
			for (i = 0; i < this->selectedUrls.size(); i++) {
				if (this->selectedUrls[i] == urls) {
					ourEntry->SetIcon(checked_net);
				}
			}
			this->menu->AddItem(ourEntry);
			this->menu->SetSelectedIndex(myindex); //jump to the index we saved from above
		}
	}

	void netInstPage::selectTitle(int selectedIndex) {
		if (this->menu->GetItems()[selectedIndex]->GetIconPath() == checked_net) {
			for (long unsigned int i = 0; i < this->selectedUrls.size(); i++) {
				if (this->selectedUrls[i] == this->ourUrls[selectedIndex]) this->selectedUrls.erase(this->selectedUrls.begin() + i);
			}
		}

		else this->selectedUrls.push_back(this->ourUrls[selectedIndex]);
		this->drawMenuItems(false);
	}

	void netInstPage::startNetwork() {
		this->butText->SetText("inst.net.buttons"_lang);
		this->menu->SetVisible(false);
		this->menu->ClearItems();
		this->infoImage->SetVisible(true);
		mainApp->LoadLayout(mainApp->netinstPage);
		this->ourUrls = netInstStuff::OnSelected();

		if (!this->ourUrls.size()) {
			mainApp->LoadLayout(mainApp->mainPage);
			return;
		}

		else if (this->ourUrls[0] == "supplyUrl") {
			std::string keyboardResult;
			std::string update = "romfs:/images/icons/update.png";
			if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.update"_theme)) {
				update = inst::config::appDir + "icons_others.update"_theme;
			}
			switch (mainApp->CreateShowDialog("inst.net.src.title"_lang, "common.cancel_desc"_lang, { "inst.net.src.opt0"_lang, "inst.net.src.opt1"_lang }, false, update)) {
			case 0:
				keyboardResult = inst::util::softwareKeyboard("inst.net.url.hint"_lang, inst::config::httplastUrl, 500);
				if (keyboardResult.size() > 0) {
					httplastUrl = keyboardResult;

					if (keyboardResult == "") {
						keyboardResult = "http://127.0.0.1";
					}
					else {
						inst::config::httplastUrl = keyboardResult;
						inst::config::setConfig();
					}

					if (inst::util::formatUrlString(keyboardResult) == "" || keyboardResult == "https://" || keyboardResult == "http://") {
						std::string fail = "romfs:/images/icons/fail.png";
						if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.fail"_theme)) {
							fail = inst::config::appDir + "icons_others.fail"_theme;
						}
						mainApp->CreateShowDialog("inst.net.url.invalid"_lang, "", { "common.ok"_lang }, false, fail);
						break;
					}
					sourceString = "inst.net.url.source_string"_lang;
					this->selectedUrls = { keyboardResult };
					this->startInstall(true);
					return;
				}
				break;
			case 1:
				keyboardResult = inst::util::softwareKeyboard("inst.net.gdrive.hint"_lang, lastFileID, 50);
				if (keyboardResult.size() > 0) {
					lastFileID = keyboardResult;
					std::string fileName = inst::util::getDriveFileName(keyboardResult);
					if (fileName.size() > 0) this->alternativeNames = { fileName };
					else this->alternativeNames = { "inst.net.gdrive.alt_name"_lang };
					sourceString = "inst.net.gdrive.source_string"_lang;
					this->selectedUrls = { "https://www.googleapis.com/drive/v3/files/" + keyboardResult + "?key=" + inst::config::gAuthKey + "&alt=media" };
					this->startInstall(true);
					return;
				}
				break;
			}
			this->startNetwork();
			return;
		}
		else {
			mainApp->CallForRender(); // If we re-render a few times during this process the main screen won't flicker
			sourceString = "inst.net.source_string"_lang;
			netConnected = true;
			this->pageInfoText->SetText("inst.net.top_info"_lang);
			this->butText->SetText("inst.net.buttons1"_lang);
			this->drawMenuItems(true);
			mainApp->CallForRender();
			this->infoImage->SetVisible(false); //
			this->menu->SetVisible(true);
			this->menu->SetSelectedIndex(0); //when page first loads jump to start of the menu 
		}
		return;
	}

	void netInstPage::startInstall(bool urlMode) {
		int dialogResult = -1;
		std::string install = "romfs:/images/icons/install.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.install"_theme)) {
			install = inst::config::appDir + "icons_others.install"_theme;
		}

		if (this->selectedUrls.size() == 1) {
			std::string ourUrlString;
			if (this->alternativeNames.size() > 0) ourUrlString = inst::util::shortenString(this->alternativeNames[0], 32, true);
			else {
				ourUrlString = inst::util::shortenString(inst::util::formatUrlString(this->selectedUrls[0]), 32, true);
			}

			dialogResult = mainApp->CreateShowDialog("inst.target.desc0"_lang + ":\n\n" + ourUrlString + "\n\n" + "inst.target.desc1"_lang, "\n\n\n\n\n\n" + "common.cancel_desc"_lang, { "inst.target.opt0"_lang, "inst.target.opt1"_lang }, false, install);
		}
		else dialogResult = mainApp->CreateShowDialog("inst.target.desc00"_lang + std::to_string(this->selectedUrls.size()) + "inst.target.desc01"_lang, "\n\n" + "common.cancel_desc"_lang, { "inst.target.opt0"_lang, "inst.target.opt1"_lang }, false, install);
		if (dialogResult == -1 && !urlMode) return;
		else if (dialogResult == -1 && urlMode) {
			this->startNetwork();
			return;
		}
		netInstStuff::installTitleNet(this->selectedUrls, dialogResult, this->alternativeNames, sourceString);
		return;
	}

	void netInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {

		if (Down & HidNpadButton_B) {
			mainApp->LoadLayout(mainApp->mainPage);
		}

		HidTouchScreenState state = { 0 };

		if (hidGetTouchScreenStates(&state, 1)) {

			if (netConnected) {
				if ((Down & HidNpadButton_A) || (state.count != xxx))
				{
					xxx = state.count;

					if (xxx != 1) {
						int var = this->menu->GetItems().size();
						auto s = std::to_string(var);
						if (s != "0") {
							this->selectTitle(this->menu->GetSelectedIndex());
							if (this->menu->GetItems().size() == 1 && this->selectedUrls.size() == 1) {
								this->startInstall(false);
							}
						}
					}
				}
			}

			if ((Down & HidNpadButton_Minus) || (state.count != xxx))
			{
				xxx = state.count;

				if (xxx != 1) {
					int var = this->menu->GetItems().size();
					auto s = std::to_string(var);
					if (s != "0") {
						this->selectTitle(this->menu->GetSelectedIndex());
						if (this->menu->GetItems().size() == 1 && this->selectedUrls.size() == 1) {
							this->startInstall(false);
						}
					}
				}
			}
		}


		if ((Down & HidNpadButton_Y)) {
			if (this->selectedUrls.size() == this->menu->GetItems().size()) this->drawMenuItems(true);
			else {
				for (long unsigned int i = 0; i < this->menu->GetItems().size(); i++) {
					if (this->menu->GetItems()[i]->GetIconPath() == checked_net) continue;
					else this->selectTitle(i);
				}
				this->drawMenuItems(false);
			}
		}

		if (Down & HidNpadButton_Plus) {
			int var = this->menu->GetItems().size();
			auto s = std::to_string(var);

			if (s != "0") {
				if (this->selectedUrls.size() == 0) {
					this->selectTitle(this->menu->GetSelectedIndex());
					this->startInstall(false);
					return;
				}
				this->startInstall(false);
			}
		}

		if (Down & HidNpadButton_ZL)
			this->menu->SetSelectedIndex(std::max(0, this->menu->GetSelectedIndex() - 6));

		if (Down & HidNpadButton_ZR)
			this->menu->SetSelectedIndex(std::min((s32)this->menu->GetItems().size() - 1, this->menu->GetSelectedIndex() + 6));

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
			this->drawMenuItems(true);
		}

		//show file extensions
		if (Down & HidNpadButton_Right) {
			this->drawMenuItems_withext(true);
		}
	}
}