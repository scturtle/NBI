//#include <switch.h>
//#include <string>
#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/sdInstPage.hpp"
#include "sdInstall.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "util/theme.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
	extern MainApplication* mainApp;
	s32 yyy = 0;
	bool show_ext;

	sdInstPage::sdInstPage() : Layout::Layout() {
		std::string default_background = inst::config::appDir + "bg_images.default_background"_theme;
		std::string sd_top = inst::config::appDir + "bg_images.sd_top"_theme;
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

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR(infoRect_colour));
		else this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#00000080"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->SetBackgroundColor(COLOR(bg_colour));
		else this->SetBackgroundColor(COLOR("#000000FF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR(tbar_colour));
		else this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#000000FF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR(bbar_colour));
		else this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR("#000000FF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(sd_top)) this->titleImage = Image::New(0, 0, (sd_top));
		else this->titleImage = Image::New(0, 0, "romfs:/images/Sd.png");

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(default_background)) this->SetBackgroundImage(default_background);
		else this->SetBackgroundImage("romfs:/images/Background.png");

		this->pageInfoText = TextBlock::New(10, 109, "inst.sd.top_info"_lang);
		this->pageInfoText->SetFont(pu::ui::MakeDefaultFontName(30));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->pageInfoText->SetColor(COLOR(pageinfo_colour));
		else this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));

		this->butText = TextBlock::New(10, 678, "inst.sd.buttons"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->butText->SetColor(COLOR(bottombar_text));
		else this->butText->SetColor(COLOR("#FFFFFFFF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR(background_overlay1), COLOR(background_overlay2), 84, (506 / 84));
		else this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), COLOR("#4f4f4d33"), 84, (506 / 84));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu->SetItemsFocusColor(COLOR(focus));
		else this->menu->SetItemsFocusColor(COLOR("#4f4f4dAA"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu->SetScrollbarColor(COLOR(scrollbar));
		else this->menu->SetScrollbarColor(COLOR("#1A1919FF"));

		this->Add(this->topRect);
		this->Add(this->infoRect);
		this->Add(this->botRect);
		this->Add(this->titleImage);
		this->Add(this->butText);
		this->Add(this->pageInfoText);
		this->Add(this->menu);
	}

	void sdInstPage::drawMenuItems(bool clearItems, std::filesystem::path ourPath) {
		int myindex = this->menu->GetSelectedIndex(); //store index so when page redraws we can get the last item we checked.
		std::string text_colour = "colour.main_text"_theme;

		if (clearItems) this->selectedTitles = {};
		this->currentDir = ourPath;

		auto pathStr = this->currentDir.string();

		if (pathStr.length())
		{
			if (pathStr[pathStr.length() - 1] == ':')
			{
				this->currentDir = this->currentDir / "";
			}
		}

		this->menu->ClearItems();
		try {
			this->ourDirectories = util::getDirsAtPath(this->currentDir);
			this->ourFiles = util::getDirectoryFiles(this->currentDir, { ".nsp", ".nsz", ".xci", ".xcz" });
		}

		catch (std::exception& e) {
			this->drawMenuItems(false, this->currentDir.parent_path());
			return;
		}

		std::string itm = "..";
		auto ourEntry = pu::ui::elm::MenuItem::New(itm);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ourEntry->SetColor(COLOR(text_colour));
		else ourEntry->SetColor(COLOR("#FFFFFFFF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.folder_up"_theme)) {
			ourEntry->SetIcon(inst::config::appDir + "icons_others.folder_up"_theme);
		}
		else ourEntry->SetIcon("romfs:/images/icons/folder-upload.png");

		this->menu->AddItem(ourEntry);

		for (auto& file : this->ourDirectories) {
			if (file == "..") break;
			std::string itm = file.filename().string();
			auto ourEntry = pu::ui::elm::MenuItem::New(itm);
			if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ourEntry->SetColor(COLOR(text_colour));
			else ourEntry->SetColor(COLOR("#FFFFFFFF"));

			if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.folder"_theme)) {
				ourEntry->SetIcon(inst::config::appDir + "icons_others.folder"_theme);
			}
			else ourEntry->SetIcon("romfs:/images/icons/folder.png");
			this->menu->AddItem(ourEntry);
		}

		for (auto& file : this->ourFiles) {
			std::string itm;
			if (show_ext == false) {
				itm = inst::util::SplitFilename(file);
			}
			else {
				itm = file.filename().string();
			}
			auto ourEntry = pu::ui::elm::MenuItem::New(itm);
			if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ourEntry->SetColor(COLOR(text_colour));
			else ourEntry->SetColor(COLOR("#FFFFFFFF"));

			ourEntry->SetIcon("romfs:/images/icons/checkbox-blank-outline.png");

			for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
				if (this->selectedTitles[i] == file) {
					ourEntry->SetIcon("romfs:/images/icons/check-box-outline.png");
				}
			}
			this->menu->AddItem(ourEntry);
			this->menu->SetSelectedIndex(myindex); //jump to the index we saved from above
		}
	}

	void sdInstPage::followDirectory() {
		int selectedIndex = this->menu->GetSelectedIndex();
		int dirListSize = this->ourDirectories.size();

		dirListSize++;
		selectedIndex--;

		if (selectedIndex < dirListSize) {
			if (this->menu->GetItems()[this->menu->GetSelectedIndex()]->GetName() == ".." && this->menu->GetSelectedIndex() == 0) {
				this->drawMenuItems(true, this->currentDir.parent_path());
			}
			else {
				this->drawMenuItems(true, this->ourDirectories[selectedIndex]);
			}
			this->menu->SetSelectedIndex(0);
		}
	}

	void sdInstPage::selectNsp(int selectedIndex) {
		int dirListSize = this->ourDirectories.size();
		dirListSize++;

		if (this->menu->GetItems()[selectedIndex]->GetIconPath() == "romfs:/images/icons/check-box-outline.png") {
			for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
				if (this->selectedTitles[i] == this->ourFiles[selectedIndex - dirListSize]) this->selectedTitles.erase(this->selectedTitles.begin() + i);
			}
		}
		else if (this->menu->GetItems()[selectedIndex]->GetIconPath() == "romfs:/images/icons/checkbox-blank-outline.png") this->selectedTitles.push_back(this->ourFiles[selectedIndex - dirListSize]);
		else {
			this->followDirectory();
			return;
		}
		this->drawMenuItems(false, currentDir);
	}

	void sdInstPage::startInstall() {
		int dialogResult = -1;
		if (this->selectedTitles.size() == 1) {
			dialogResult = mainApp->CreateShowDialog("inst.target.desc0"_lang + ":\n\n" + inst::util::shortenString(std::filesystem::path(this->selectedTitles[0]).filename().string(), 32, true) + "\n\n" + "inst.target.desc1"_lang, "\n\n\n\n\n\n\n" + "common.cancel_desc"_lang, { "inst.target.opt0"_lang, "inst.target.opt1"_lang }, false, "romfs:/images/icons/install.png");
		}
		else dialogResult = mainApp->CreateShowDialog("inst.target.desc00"_lang + std::to_string(this->selectedTitles.size()) + "inst.target.desc01"_lang, "\n" + "common.cancel_desc"_lang, { "inst.target.opt0"_lang, "inst.target.opt1"_lang }, false, "romfs:/images/icons/install.png");
		if (dialogResult == -1) return;
		nspInstStuff::installNspFromFile(this->selectedTitles, dialogResult);
	}

	void sdInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {

		if (Down & HidNpadButton_B) {
			mainApp->LoadLayout(mainApp->mainPage);
		}

		HidTouchScreenState state = { 0 };

		if (hidGetTouchScreenStates(&state, 1)) {

			if ((Down & HidNpadButton_A) || (state.count != yyy))
			{
				yyy = state.count;

				if (yyy != 1) {
					int var = this->menu->GetItems().size();
					auto s = std::to_string(var);

					if (s != "0") {
						this->selectNsp(this->menu->GetSelectedIndex());

						if (this->ourFiles.size() == 1 && this->selectedTitles.size() == 1) {
							this->startInstall();
						}
					}
				}
			}
		}

		if ((Down & HidNpadButton_Y)) {
			if (this->selectedTitles.size() == this->ourFiles.size()) this->drawMenuItems(true, currentDir);
			else {
				int topDir = 0;
				topDir++;
				for (long unsigned int i = this->ourDirectories.size() + topDir; i < this->menu->GetItems().size(); i++) {
					if (this->menu->GetItems()[i]->GetIconPath() == "romfs:/images/icons/check-box-outline.png") continue;
					else this->selectNsp(i);
				}
				this->drawMenuItems(false, currentDir);
			}
		}

		if ((Down & HidNpadButton_X)) {
			inst::ui::mainApp->CreateShowDialog("inst.sd.help.title"_lang, "inst.sd.help.desc"_lang, { "common.ok"_lang }, true, "romfs:/images/icons/information.png");
		}

		if (Down & HidNpadButton_Plus) {
			int var = this->menu->GetItems().size();
			auto s = std::to_string(var);

			if (s != "0") {
				if (this->selectedTitles.size() == 0 && this->menu->GetItems()[this->menu->GetSelectedIndex()]->GetIconPath() == "romfs:/images/icons/checkbox-blank-outline.png") {
					this->selectNsp(this->menu->GetSelectedIndex());
				}
				if (this->selectedTitles.size() > 0) this->startInstall();
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

		//don't show file extensions - refresh page
		if (Down & HidNpadButton_Left) {
			show_ext = false;
			this->drawMenuItems(true, currentDir);
			//this->drawMenuItems(true);
		}

		if (Down & HidNpadButton_Right) {
			show_ext = true;
			this->drawMenuItems(true, currentDir);
		}
	}
}