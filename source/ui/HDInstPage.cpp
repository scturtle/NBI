#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/HDInstPage.hpp"
#include "HDInstall.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "util/theme.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
	extern MainApplication* mainApp;
	std::string hd_root = inst::config::appDir + "/theme";
	bool hd_theme = util::themeit(hd_root); //check if we have a previous theme directory first.
	s32 zzz = 0; //touchscreen variable
	bool show_file_ext;

	std::string checked_hdd = "romfs:/images/icons/check-box-outline.png";
	std::string unchecked_hdd = "romfs:/images/icons/checkbox-blank-outline.png";

	void checkbox_hdd() {
		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.checkbox-checked"_theme)) {
			checked_hdd = inst::config::appDir + "icons_others.checkbox-checked"_theme;
		}
		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.checkbox-empty"_theme)) {
			unchecked_hdd = inst::config::appDir + "icons_others.checkbox-empty"_theme;
		}
	}

	HDInstPage::HDInstPage() : Layout::Layout() {
		std::string infoRect_colour = "colour.inforect"_theme;
		std::string bg_colour = "colour.background"_theme;
		std::string tbar_colour = "colour.topbar"_theme;
		std::string bbar_colour = "colour.bottombar"_theme;
		std::string hd_top = inst::config::appDir + "bg_images.hd_top"_theme;
		std::string default_background = inst::config::appDir + "bg_images.default_background"_theme;
		std::string pageinfo_colour = "colour.pageinfo_text"_theme;
		std::string bottombar_text = "colour.bottombar_text"_theme;
		std::string background_overlay1 = "colour.background_overlay1"_theme;
		std::string background_overlay2 = "colour.background_overlay2"_theme;
		std::string focus = "colour.focus"_theme;
		std::string scrollbar = "colour.scrollbar"_theme;

		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR(infoRect_colour));
		else this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#00000080"));

		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->SetBackgroundColor(COLOR(bg_colour));
		else this->SetBackgroundColor(COLOR("#000000FF"));

		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR(tbar_colour));
		else this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#000000FF"));

		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR(bbar_colour));
		else this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR("#000000FF"));

		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(hd_top)) this->titleImage = Image::New(0, 0, (hd_top));
		else this->titleImage = Image::New(0, 0, "romfs:/images/Hd.png");

		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(default_background)) this->SetBackgroundImage(default_background);
		else this->SetBackgroundImage("romfs:/images/Background.png");

		this->pageInfoText = TextBlock::New(10, 109, "inst.hd.top_info"_lang);
		this->pageInfoText->SetFont(pu::ui::MakeDefaultFontName(30));
		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->pageInfoText->SetColor(COLOR(pageinfo_colour));
		else this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));

		this->butText = TextBlock::New(10, 678, "inst.hd.buttons"_lang);
		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->butText->SetColor(COLOR(bottombar_text));
		else this->butText->SetColor(COLOR("#FFFFFFFF"));

		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR(background_overlay1), COLOR(background_overlay2), 84, (506 / 84));
		else this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), COLOR("#4f4f4d33"), 84, (506 / 84));

		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu->SetItemsFocusColor(COLOR(focus));
		else this->menu->SetItemsFocusColor(COLOR("#4f4f4dAA"));

		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->menu->SetScrollbarColor(COLOR(scrollbar));
		else this->menu->SetScrollbarColor(COLOR("#1A1919FF"));

		this->Add(this->topRect);
		this->Add(this->infoRect);
		this->Add(this->botRect);
		this->Add(this->titleImage);
		this->Add(this->butText);
		this->Add(this->pageInfoText);
		this->Add(this->menu);
		checkbox_hdd();
	}

	void HDInstPage::drawMenuItems(bool clearItems, std::filesystem::path ourPath) {
		int myindex = this->menu->GetSelectedIndex(); //store index so when page redraws we can get the last item we checked.
		if (clearItems) this->selectedTitles = {};
		std::string text_colour = "colour.main_text"_theme;
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
		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ourEntry->SetColor(COLOR(text_colour));
		else ourEntry->SetColor(COLOR("#FFFFFFFF"));

		std::string folder_up = "romfs:/images/icons/folder-upload.png";
		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.folder_up"_theme)) {
			folder_up = inst::config::appDir + "icons_others.folder_up"_theme;
		}

		ourEntry->SetIcon(folder_up);
		this->menu->AddItem(ourEntry);

		for (auto& file : this->ourDirectories) {
			if (file == "..") break;
			std::string itm = file.filename().string();
			auto ourEntry = pu::ui::elm::MenuItem::New(itm);
			if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ourEntry->SetColor(COLOR(text_colour));
			else ourEntry->SetColor(COLOR("#FFFFFFFF"));

			std::string folder = "romfs:/images/icons/folder.png";
			if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.folder"_theme)) {
				folder = inst::config::appDir + "icons_others.folder"_theme;
			}
			ourEntry->SetIcon(folder);
			this->menu->AddItem(ourEntry);
		}
		for (auto& file : this->ourFiles) {
			std::string itm;
			if (show_file_ext == false) {
				itm = inst::util::SplitFilename(file);
			}
			else {
				itm = file.filename().string();
			}
			auto ourEntry = pu::ui::elm::MenuItem::New(itm);
			if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ourEntry->SetColor(COLOR(text_colour));
			else ourEntry->SetColor(COLOR("#FFFFFFFF"));
			ourEntry->SetIcon(unchecked_hdd);
			for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
				if (this->selectedTitles[i] == file) {
					ourEntry->SetIcon(checked_hdd);
				}
			}
			this->menu->AddItem(ourEntry);
			this->menu->SetSelectedIndex(myindex); //jump to the index we saved from above
		}
	}

	void HDInstPage::followDirectory() {
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

	void HDInstPage::selectNsp(int selectedIndex) {
		int dirListSize = this->ourDirectories.size();
		dirListSize++;

		if (this->menu->GetItems()[selectedIndex]->GetIconPath() == checked_hdd) {
			for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
				if (this->selectedTitles[i] == this->ourFiles[selectedIndex - dirListSize]) this->selectedTitles.erase(this->selectedTitles.begin() + i);
			}
		}
		else if (this->menu->GetItems()[selectedIndex]->GetIconPath() == unchecked_hdd) this->selectedTitles.push_back(this->ourFiles[selectedIndex - dirListSize]);
		else {
			this->followDirectory();
			return;
		}
		this->drawMenuItems(false, currentDir);
	}

	void HDInstPage::startInstall() {
		int dialogResult = -1;
		std::string install = "romfs:/images/icons/install.png";
		if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.install"_theme)) {
			install = inst::config::appDir + "icons_others.install"_theme;
		}
		if (this->selectedTitles.size() == 1) {
			dialogResult = mainApp->CreateShowDialog("inst.target.desc0"_lang + ":\n\n" + inst::util::shortenString(std::filesystem::path(this->selectedTitles[0]).filename().string(), 32, true) + "\n\n" + "inst.target.desc1"_lang, "\n\n\n\n\n\n\n" + "common.cancel_desc"_lang, { "inst.target.opt0"_lang, "inst.target.opt1"_lang }, false, install);
		}
		else dialogResult = mainApp->CreateShowDialog("inst.target.desc00"_lang + std::to_string(this->selectedTitles.size()) + "inst.target.desc01"_lang, "\n" + "common.cancel_desc"_lang, { "inst.target.opt0"_lang, "inst.target.opt1"_lang }, false, install);
		if (dialogResult == -1) return;
		nspInstStuff_B::installNspFromFile(this->selectedTitles, dialogResult);
	}

	void HDInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {
		if (Down & HidNpadButton_B) {
			mainApp->LoadLayout(mainApp->mainPage);
		}

		HidTouchScreenState state = { 0 };

		if (hidGetTouchScreenStates(&state, 1)) {

			if ((Down & HidNpadButton_A) || (state.count != zzz))
			{
				zzz = state.count;

				if (zzz != 1) {
					this->selectNsp(this->menu->GetSelectedIndex());
					if (this->ourFiles.size() == 1 && this->selectedTitles.size() == 1) {
						this->startInstall();
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
					if (this->menu->GetItems()[i]->GetIconPath() == checked_hdd) continue;
					else this->selectNsp(i);
				}
				this->drawMenuItems(false, currentDir);
			}
		}

		if ((Down & HidNpadButton_X)) {
			std::string information = "romfs:/images/icons/information.png";
			if (hd_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.information"_theme)) {
				information = inst::config::appDir + "icons_others.information"_theme;
			}
			inst::ui::mainApp->CreateShowDialog("inst.hd.help.title"_lang, "inst.hd.help.desc"_lang, { "common.ok"_lang }, true, information);
		}

		if (Down & HidNpadButton_Plus) {
			if (this->selectedTitles.size() == 0 && this->menu->GetItems()[this->menu->GetSelectedIndex()]->GetIconPath() == unchecked_hdd) {
				this->selectNsp(this->menu->GetSelectedIndex());
			}
			if (this->selectedTitles.size() > 0) this->startInstall();
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
			show_file_ext = false;
			this->drawMenuItems(true, currentDir);
			//this->drawMenuItems(true);
		}

		if (Down & HidNpadButton_Right) {
			show_file_ext = true;
			this->drawMenuItems(true, currentDir);
		}
	}
}