#include <filesystem>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/instPage.hpp"
#include "ui/optionsPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/curl.hpp"
#include "util/unzip.hpp"
#include "util/lang.hpp"
#include "ui/instPage.hpp"
#include "sigInstall.hpp"
#include "util/theme.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
	extern MainApplication* mainApp;
	s32 prev_touchcount = 0;
	std::string flag = "romfs:/images/flags/en.png";

	std::vector<std::string> languageStrings = { "Sys", "En", "Jpn", "Fr", "De", "It", "Ru", "Es", "Tw" };

	optionsPage::optionsPage() : Layout::Layout() {
		std::string infoRect_colour = "colour.inforect"_theme;
		std::string bg_colour = "colour.background"_theme;
		std::string tbar_colour = "colour.topbar"_theme;
		std::string bbar_colour = "colour.bottombar"_theme;
		std::string settings_top = inst::config::appDir + "bg_images.settings_top"_theme;
		std::string default_background = inst::config::appDir + "bg_images.default_background"_theme;
		std::string version = "colour.version"_theme;
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

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(settings_top)) this->titleImage = Image::New(0, 0, (settings_top));
		else this->titleImage = Image::New(0, 0, "romfs:/images/Settings.png");

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(default_background)) this->SetBackgroundImage(default_background);
		else this->SetBackgroundImage("romfs:/images/Background.png");

		this->appVersionText = TextBlock::New(1200, 680, "v" + inst::config::appVersion);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->appVersionText->SetColor(COLOR(version));
		else this->appVersionText->SetColor(COLOR("#FFFFFFFF"));
		this->appVersionText->SetFont(pu::ui::MakeDefaultFontName(20));

		this->pageInfoText = TextBlock::New(10, 109, "options.title"_lang);
		this->pageInfoText->SetFont(pu::ui::MakeDefaultFontName(30));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->pageInfoText->SetColor(COLOR(pageinfo_colour));
		else this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));

		this->butText = TextBlock::New(10, 678, "options.buttons"_lang);
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
		this->Add(this->appVersionText);
		this->Add(this->butText);
		this->Add(this->pageInfoText);
		this->setMenuText();
		this->Add(this->menu);
	}

	void optionsPage::askToUpdate(std::vector<std::string> updateInfo) {
		if (!mainApp->CreateShowDialog("options.update.title"_lang, "options.update.desc0"_lang + updateInfo[0] + "options.update.desc1"_lang, { "options.update.opt0"_lang, "common.cancel"_lang }, false, "romfs:/images/icons/update.png")) {
			inst::ui::instPage::loadInstallScreen();
			inst::ui::instPage::setTopInstInfoText("options.update.top_info"_lang + updateInfo[0]);
			inst::ui::instPage::setInstBarPerc(0);
			inst::ui::instPage::setInstInfoText("options.update.bot_info"_lang + updateInfo[0]);
			try {
				std::string downloadName = inst::config::appDir + "/temp_download.zip";
				inst::curl::downloadFile(updateInfo[1], downloadName.c_str(), 0, true);
				romfsExit();
				inst::ui::instPage::setInstInfoText("options.update.bot_info2"_lang + updateInfo[0]);
				inst::zip::extractFile(downloadName, "sdmc:/");
				std::filesystem::remove(downloadName);
				mainApp->CreateShowDialog("options.update.complete"_lang, "options.update.end_desc"_lang, { "common.ok"_lang }, false, "romfs:/images/icons/update.png");
			}
			catch (...) {
				mainApp->CreateShowDialog("options.update.failed"_lang, "options.update.end_desc"_lang, { "common.ok"_lang }, false, "romfs:/images/icons/fail.png");
			}
			mainApp->FadeOut();
			mainApp->Close();
		}
		return;
	}

	std::string optionsPage::getMenuOptionIcon(bool ourBool) {
		if (ourBool) return "romfs:/images/icons/checked.png";
		else return "romfs:/images/icons/unchecked.png";
	}

	std::string optionsPage::getMenuLanguage(int ourLangCode) {
		if (ourLangCode >= 0) {
			if (ourLangCode == 0) flag = "romfs:/images/flags/sys.png";
			else if (ourLangCode == 1) flag = "romfs:/images/flags/en.png";
			else if (ourLangCode == 2) flag = "romfs:/images/flags/jpn.png";
			else if (ourLangCode == 3) flag = "romfs:/images/flags/fr.png";
			else if (ourLangCode == 4) flag = "romfs:/images/flags/de.png";
			else if (ourLangCode == 5) flag = "romfs:/images/flags/it.png";
			else if (ourLangCode == 6) flag = "romfs:/images/flags/ru.png";
			else if (ourLangCode == 7) flag = "romfs:/images/flags/es.png";
			else if (ourLangCode == 8) flag = "romfs:/images/flags/tw.png";
			return languageStrings[ourLangCode];
		}
		else {
			flag = "romfs:/images/flags/en.png";
			return languageStrings[0];
		}
	}

	void sigPatchesMenuItem_Click() {
		sig::installSigPatches();
	}

	void thememessage() {
		int ourResult = inst::ui::mainApp->CreateShowDialog("theme.title"_lang, "theme.desc"_lang, { "common.no"_lang, "common.yes"_lang }, true, "romfs:/images/icons/theme.png");
		if (ourResult != 0) {
			if (!inst::config::useTheme) {
				inst::config::useTheme = true;
				mainApp->FadeOut();
				mainApp->Close();
			}
		}
		else {
			if (inst::config::useTheme) {
				inst::config::useTheme = false;
				mainApp->FadeOut();
				mainApp->Close();
			}
		}
	}

	void optionsPage::setMenuText() {
		this->menu->ClearItems();

		auto ignoreFirmOption = pu::ui::elm::MenuItem::New("options.menu_items.ignore_firm"_lang);
		ignoreFirmOption->SetColor(COLOR("#FFFFFFFF"));
		ignoreFirmOption->SetIcon(this->getMenuOptionIcon(inst::config::ignoreReqVers));
		this->menu->AddItem(ignoreFirmOption);

		auto validateOption = pu::ui::elm::MenuItem::New("options.menu_items.nca_verify"_lang);
		validateOption->SetColor(COLOR("#FFFFFFFF"));
		validateOption->SetIcon(this->getMenuOptionIcon(inst::config::validateNCAs));
		this->menu->AddItem(validateOption);

		auto overclockOption = pu::ui::elm::MenuItem::New("options.menu_items.boost_mode"_lang);
		overclockOption->SetColor(COLOR("#FFFFFFFF"));
		overclockOption->SetIcon(this->getMenuOptionIcon(inst::config::overClock));
		this->menu->AddItem(overclockOption);

		auto deletePromptOption = pu::ui::elm::MenuItem::New("options.menu_items.ask_delete"_lang);
		deletePromptOption->SetColor(COLOR("#FFFFFFFF"));
		deletePromptOption->SetIcon(this->getMenuOptionIcon(inst::config::deletePrompt));
		this->menu->AddItem(deletePromptOption);

		auto autoUpdateOption = pu::ui::elm::MenuItem::New("options.menu_items.auto_update"_lang);
		autoUpdateOption->SetColor(COLOR("#FFFFFFFF"));
		autoUpdateOption->SetIcon(this->getMenuOptionIcon(inst::config::autoUpdate));
		this->menu->AddItem(autoUpdateOption);

		auto useSoundOption = pu::ui::elm::MenuItem::New("options.menu_items.useSound"_lang);
		useSoundOption->SetColor(COLOR("#FFFFFFFF"));
		useSoundOption->SetIcon(this->getMenuOptionIcon(inst::config::useSound));
		this->menu->AddItem(useSoundOption);

		auto fixticket = pu::ui::elm::MenuItem::New("options.menu_items.fixticket"_lang);
		fixticket->SetColor(COLOR("#FFFFFFFF"));
		fixticket->SetIcon(this->getMenuOptionIcon(inst::config::fixticket));
		this->menu->AddItem(fixticket);

		auto listoveride = pu::ui::elm::MenuItem::New("options.menu_items.listoveride"_lang);
		listoveride->SetColor(COLOR("#FFFFFFFF"));
		listoveride->SetIcon(this->getMenuOptionIcon(inst::config::listoveride));
		this->menu->AddItem(listoveride);

		auto httpkeyboard = pu::ui::elm::MenuItem::New("options.menu_items.usehttpkeyboard"_lang);
		httpkeyboard->SetColor(COLOR("#FFFFFFFF"));
		httpkeyboard->SetIcon(this->getMenuOptionIcon(inst::config::httpkeyboard));
		this->menu->AddItem(httpkeyboard);

		auto useThemeOption = pu::ui::elm::MenuItem::New("theme.theme_option"_lang);
		useThemeOption->SetColor(COLOR("#FFFFFFFF"));
		useThemeOption->SetIcon(this->getMenuOptionIcon(inst::config::useTheme));
		this->menu->AddItem(useThemeOption);

		auto ThemeMenuOption = pu::ui::elm::MenuItem::New("theme.theme_menu"_lang);
		ThemeMenuOption->SetColor(COLOR("#FFFFFFFF"));
		ThemeMenuOption->SetIcon("romfs:/images/icons/thememenu.png");
		this->menu->AddItem(ThemeMenuOption);

		//
		auto ThemeUrlOption = pu::ui::elm::MenuItem::New("theme.theme_url"_lang + inst::util::shortenString(inst::config::httplastUrl2, 42, false));
		ThemeUrlOption->SetColor(COLOR("#FFFFFFFF"));
		ThemeUrlOption->SetIcon("romfs:/images/icons/themeurl.png");
		this->menu->AddItem(ThemeUrlOption);
		//

		auto SigPatch = pu::ui::elm::MenuItem::New("main.menu.sig"_lang);
		SigPatch->SetColor(COLOR("#FFFFFFFF"));
		SigPatch->SetIcon("romfs:/images/icons/plaster.png");
		this->menu->AddItem(SigPatch);

		auto sigPatchesUrlOption = pu::ui::elm::MenuItem::New("options.menu_items.sig_url"_lang + inst::util::shortenString(inst::config::sigPatchesUrl, 42, false));
		sigPatchesUrlOption->SetColor(COLOR("#FFFFFFFF"));
		sigPatchesUrlOption->SetIcon("romfs:/images/icons/keyboard.png");
		this->menu->AddItem(sigPatchesUrlOption);

		auto httpServerUrlOption = pu::ui::elm::MenuItem::New("options.menu_items.http_url"_lang + inst::util::shortenString(inst::config::httpIndexUrl, 42, false));
		httpServerUrlOption->SetColor(COLOR("#FFFFFFFF"));
		httpServerUrlOption->SetIcon("romfs:/images/icons/url.png");
		this->menu->AddItem(httpServerUrlOption);

		auto languageOption = pu::ui::elm::MenuItem::New("options.menu_items.language"_lang + this->getMenuLanguage(inst::config::languageSetting));
		languageOption->SetColor(COLOR("#FFFFFFFF"));
		languageOption->SetIcon("romfs:/images/icons/speak.png");
		this->menu->AddItem(languageOption);

		auto updateOption = pu::ui::elm::MenuItem::New("options.menu_items.check_update"_lang);
		updateOption->SetColor(COLOR("#FFFFFFFF"));
		updateOption->SetIcon("romfs:/images/icons/update2.png");
		this->menu->AddItem(updateOption);

		auto creditsOption = pu::ui::elm::MenuItem::New("options.menu_items.credits"_lang);
		creditsOption->SetColor(COLOR("#FFFFFFFF"));
		creditsOption->SetIcon("romfs:/images/icons/credits2.png");
		this->menu->AddItem(creditsOption);
	}

	void optionsPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {

		if (Down & HidNpadButton_B) {
			mainApp->LoadLayout(mainApp->mainPage);
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

		HidTouchScreenState state = { 0 };

		if (hidGetTouchScreenStates(&state, 1)) {

			if ((Down & HidNpadButton_A) || (state.count != prev_touchcount))
			{
				prev_touchcount = state.count;

				if (prev_touchcount != 1) {

					std::string keyboardResult;
					int rc;
					std::vector<std::string> downloadUrl;
					std::vector<std::string> languageList;
					int index = this->menu->GetSelectedIndex();
					switch (index) {
					case 0:
						inst::config::ignoreReqVers = !inst::config::ignoreReqVers;
						inst::config::setConfig();
						this->setMenuText();
						//makes sure to jump back to the selected item once the menu is reloaded
						this->menu->SetSelectedIndex(index);
						//
						break;
					case 1:
						if (inst::config::validateNCAs) {
							if (inst::ui::mainApp->CreateShowDialog("options.nca_warn.title"_lang, "options.nca_warn.desc"_lang, { "common.cancel"_lang, "options.nca_warn.opt1"_lang }, false, "romfs:/images/icons/information.png") == 1) inst::config::validateNCAs = false;
						}
						else inst::config::validateNCAs = true;
						inst::config::setConfig();
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						break;
					case 2:
						inst::config::overClock = !inst::config::overClock;
						inst::config::setConfig();
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						break;
					case 3:
						inst::config::deletePrompt = !inst::config::deletePrompt;
						inst::config::setConfig();
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						break;
					case 4:
						inst::config::autoUpdate = !inst::config::autoUpdate;
						inst::config::setConfig();
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						break;
					case 5:
						if (inst::config::useSound) {
							inst::config::useSound = false;
						}
						else {
							inst::config::useSound = true;
						}
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						inst::config::setConfig();
						break;
					case 6:
						if (inst::config::fixticket) {
							inst::config::fixticket = false;
						}
						else {
							inst::config::fixticket = true;
						}
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						inst::config::setConfig();
						break;
					case 7:
						if (inst::config::listoveride) {
							inst::config::listoveride = false;
						}
						else {
							inst::config::listoveride = true;
						}
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						inst::config::setConfig();
						break;
					case 8:
						if (inst::config::httpkeyboard) {
							inst::config::httpkeyboard = false;
						}
						else {
							inst::config::httpkeyboard = true;
						}
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						inst::config::setConfig();
						break;
					case 9:
						thememessage();
						inst::config::setConfig();
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						break;
					case 10:
						if (inst::util::getIPAddress() == "1.0.0.127") {
							inst::ui::mainApp->CreateShowDialog("main.net.title"_lang, "main.net.desc"_lang, { "common.ok"_lang }, true, "romfs:/images/icons/information.png");
							break;
						}
						mainApp->ThemeinstPage->startNetwork();
						break;
					case 11:
						keyboardResult = inst::util::softwareKeyboard("inst.net.url.hint"_lang, inst::config::httplastUrl2.c_str(), 500);
						if (keyboardResult.size() > 0) {
							inst::config::httplastUrl2 = keyboardResult;
							inst::config::setConfig();
							this->setMenuText();
							this->menu->SetSelectedIndex(index);
						}
						break;
					case 12:
						sigPatchesMenuItem_Click();
						break;
					case 13:
						keyboardResult = inst::util::softwareKeyboard("options.sig_hint"_lang, inst::config::sigPatchesUrl.c_str(), 500);
						if (keyboardResult.size() > 0) {
							inst::config::sigPatchesUrl = keyboardResult;
							inst::config::setConfig();
							this->setMenuText();
							this->menu->SetSelectedIndex(index);
						}
						break;
					case 14:
						keyboardResult = inst::util::softwareKeyboard("inst.net.url.hint"_lang, inst::config::httpIndexUrl.c_str(), 500);
						if (keyboardResult.size() > 0) {
							inst::config::httpIndexUrl = keyboardResult;
							inst::config::setConfig();
							this->setMenuText();
							this->menu->SetSelectedIndex(index);
						}
						break;
					case 15:
						languageList = languageStrings;
						languageList[0] = "options.language.system_language"_lang; //replace "sys" with local language string 
						rc = inst::ui::mainApp->CreateShowDialog("options.language.title"_lang, "options.language.desc"_lang, languageList, false, flag);
						if (rc == -1) break;
						switch (rc) {
						case 0:
							inst::config::languageSetting = 0;
							break;
						case 1:
							inst::config::languageSetting = 1;
							break;
						case 2:
							inst::config::languageSetting = 2;
							break;
						case 3:
							inst::config::languageSetting = 3;
							break;
						case 4:
							inst::config::languageSetting = 4;
							break;
						case 5:
							inst::config::languageSetting = 5;
							break;
						case 6:
							inst::config::languageSetting = 6;
							break;
						case 7:
							inst::config::languageSetting = 7;
							break;
						case 8:
							inst::config::languageSetting = 8;
							break;
						default:
							inst::config::languageSetting = 0;
						}
						inst::config::setConfig();
						mainApp->FadeOut();
						mainApp->Close();
						break;
					case 16:
						if (inst::util::getIPAddress() == "1.0.0.127") {
							inst::ui::mainApp->CreateShowDialog("main.net.title"_lang, "main.net.desc"_lang, { "common.ok"_lang }, true, "romfs:/images/icons/update.png");
							break;
						}
						downloadUrl = inst::util::checkForAppUpdate();
						if (!downloadUrl.size()) {
							mainApp->CreateShowDialog("options.update.title_check_fail"_lang, "options.update.desc_check_fail"_lang, { "common.ok"_lang }, false, "romfs:/images/icons/fail.png");
							break;
						}
						this->askToUpdate(downloadUrl);
						break;
					case 17:
						inst::ui::mainApp->CreateShowDialog("options.credits.title"_lang, "options.credits.desc"_lang, { "common.close"_lang }, true, "romfs:/images/icons/credits.png");
						break;
					default:
						break;
					}
				}
			}
		}
	}
}