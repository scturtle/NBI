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
	std::vector<std::string> languageStrings = { "Sys", "En", "Jpn", "Fr", "De", "It", "Ru", "Es", "Tw", "Cn" };

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

		std::string update = "romfs:/images/icons/update.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.update"_theme)) {
			update = inst::config::appDir + "icons_others.update"_theme;
		}

		if (!mainApp->CreateShowDialog("options.update.title"_lang, "options.update.desc0"_lang + updateInfo[0] + "options.update.desc1"_lang, { "options.update.opt0"_lang, "common.cancel"_lang }, false, update)) {
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
				mainApp->CreateShowDialog("options.update.complete"_lang, "options.update.end_desc"_lang, { "common.ok"_lang }, false, update);
			}
			catch (...) {
				std::string fail = "romfs:/images/icons/fail.png";
				if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.fail"_theme)) {
					fail = inst::config::appDir + "icons_others.fail"_theme;
				}
				mainApp->CreateShowDialog("options.update.failed"_lang, "options.update.end_desc"_lang, { "common.ok"_lang }, false, fail);
			}
			mainApp->FadeOut();
			mainApp->Close();
		}
		return;
	}

	std::string optionsPage::getMenuOptionIcon(bool ourBool) {
		std::string checked = "romfs:/images/icons/checked.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_settings.check_on"_theme)) {
			checked = inst::config::appDir + "icons_settings.check_on"_theme;
		}
		std::string unchecked = "romfs:/images/icons/unchecked.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_settings.check_off"_theme)) {
			unchecked = inst::config::appDir + "icons_settings.check_off"_theme;
		}
		if (ourBool) return checked;
		else return unchecked;
	}

	std::string optionsPage::getMenuLanguage(int ourLangCode) {
		std::string sys = "romfs:/images/flags/sys.png";
		std::string en = "romfs:/images/flags/en.png";
		std::string jpn = "romfs:/images/flags/jpn.png";
		std::string fr = "romfs:/images/flags/fr.png";
		std::string de = "romfs:/images/flags/de.png";
		std::string it = "romfs:/images/flags/it.png";
		std::string ru = "romfs:/images/flags/ru.png";
		std::string es = "romfs:/images/flags/es.png";
		std::string tw = "romfs:/images/flags/tw.png";
		std::string cn = "romfs:/images/flags/cn.png";
		//
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.sys"_theme)) {
			sys = inst::config::appDir + "icons_flags.sys"_theme;
		}
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.en"_theme)) {
			en = inst::config::appDir + "icons_flags.en"_theme;
		}
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.jpn"_theme)) {
			jpn = inst::config::appDir + "icons_flags.jpn"_theme;
		}
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.fr"_theme)) {
			fr = inst::config::appDir + "icons_flags.fr"_theme;
		}
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.de"_theme)) {
			de = inst::config::appDir + "icons_flags.de"_theme;
		}
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.it"_theme)) {
			it = inst::config::appDir + "icons_flags.it"_theme;
		}
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.ru"_theme)) {
			ru = inst::config::appDir + "icons_flags.ru"_theme;
		}
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.es"_theme)) {
			es = inst::config::appDir + "icons_flags.es"_theme;
		}
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.tw"_theme)) {
			tw = inst::config::appDir + "icons_flags.tw"_theme;
		}
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.cn"_theme)) {
			cn = inst::config::appDir + "icons_flags.cn"_theme;
		}
		//
		if (ourLangCode >= 0) {
			if (ourLangCode == 0) flag = sys;
			else if (ourLangCode == 1) flag = en;
			else if (ourLangCode == 2) flag = jpn;
			else if (ourLangCode == 3) flag = fr;
			else if (ourLangCode == 4) flag = de;
			else if (ourLangCode == 5) flag = it;
			else if (ourLangCode == 6) flag = ru;
			else if (ourLangCode == 7) flag = es;
			else if (ourLangCode == 8) flag = tw;
			else if (ourLangCode == 9) flag = cn;
			return languageStrings[ourLangCode];
		}
		else {
			flag = en;
			return languageStrings[0];
		}
	}

	void sigPatchesMenuItem_Click() {
		sig::installSigPatches();
	}

	void thememessage() {
		std::string theme = "romfs:/images/icons/theme.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.theme"_theme)) {
			theme = inst::config::appDir + "icons_others.theme"_theme;
		}
		int ourResult = inst::ui::mainApp->CreateShowDialog("theme.title"_lang, "theme.desc"_lang, { "common.no"_lang, "common.yes"_lang }, true, theme);
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

	void lang_message() {
		std::string flag = "romfs:/images/icons/flags/sys.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_flags.sys"_theme)) {
			flag = inst::config::appDir + "icons_flags.sys"_theme;
		}
		int ourResult = inst::ui::mainApp->CreateShowDialog("sig.restart"_lang, "theme.restart"_lang, { "common.no"_lang, "common.yes"_lang }, true, flag);
		if (ourResult != 0) {
			mainApp->FadeOut();
			mainApp->Close();
		}
	}

	void optionsPage::setMenuText() {
		std::string text_colour = "colour.main_text"_theme;
		this->menu->ClearItems();

		auto ignoreFirmOption = pu::ui::elm::MenuItem::New("options.menu_items.ignore_firm"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ignoreFirmOption->SetColor(COLOR(text_colour));
		else ignoreFirmOption->SetColor(COLOR("#FFFFFFFF"));
		ignoreFirmOption->SetIcon(this->getMenuOptionIcon(inst::config::ignoreReqVers));
		this->menu->AddItem(ignoreFirmOption);

		auto validateOption = pu::ui::elm::MenuItem::New("options.menu_items.nca_verify"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) validateOption->SetColor(COLOR(text_colour));
		else validateOption->SetColor(COLOR("#FFFFFFFF"));
		validateOption->SetIcon(this->getMenuOptionIcon(inst::config::validateNCAs));
		this->menu->AddItem(validateOption);

		auto overclockOption = pu::ui::elm::MenuItem::New("options.menu_items.boost_mode"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) overclockOption->SetColor(COLOR(text_colour));
		else overclockOption->SetColor(COLOR("#FFFFFFFF"));
		overclockOption->SetIcon(this->getMenuOptionIcon(inst::config::overClock));
		this->menu->AddItem(overclockOption);

		auto deletePromptOption = pu::ui::elm::MenuItem::New("options.menu_items.ask_delete"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) deletePromptOption->SetColor(COLOR(text_colour));
		else deletePromptOption->SetColor(COLOR("#FFFFFFFF"));
		deletePromptOption->SetIcon(this->getMenuOptionIcon(inst::config::deletePrompt));
		this->menu->AddItem(deletePromptOption);

		auto autoUpdateOption = pu::ui::elm::MenuItem::New("options.menu_items.auto_update"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) autoUpdateOption->SetColor(COLOR(text_colour));
		else autoUpdateOption->SetColor(COLOR("#FFFFFFFF"));
		autoUpdateOption->SetIcon(this->getMenuOptionIcon(inst::config::autoUpdate));
		this->menu->AddItem(autoUpdateOption);

		auto useSoundOption = pu::ui::elm::MenuItem::New("options.menu_items.useSound"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) useSoundOption->SetColor(COLOR(text_colour));
		else useSoundOption->SetColor(COLOR("#FFFFFFFF"));
		useSoundOption->SetIcon(this->getMenuOptionIcon(inst::config::useSound));
		this->menu->AddItem(useSoundOption);

		auto useMusicOption = pu::ui::elm::MenuItem::New("options.menu_items.useMusic"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) useMusicOption->SetColor(COLOR(text_colour));
		else useMusicOption->SetColor(COLOR("#FFFFFFFF"));
		useMusicOption->SetIcon(this->getMenuOptionIcon(inst::config::useMusic));
		this->menu->AddItem(useMusicOption);

		auto fixticket = pu::ui::elm::MenuItem::New("options.menu_items.fixticket"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) fixticket->SetColor(COLOR(text_colour));
		else fixticket->SetColor(COLOR("#FFFFFFFF"));
		fixticket->SetIcon(this->getMenuOptionIcon(inst::config::fixticket));
		this->menu->AddItem(fixticket);

		auto listoveride = pu::ui::elm::MenuItem::New("options.menu_items.listoveride"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) listoveride->SetColor(COLOR(text_colour));
		else listoveride->SetColor(COLOR("#FFFFFFFF"));
		listoveride->SetIcon(this->getMenuOptionIcon(inst::config::listoveride));
		this->menu->AddItem(listoveride);

		auto httpkeyboard = pu::ui::elm::MenuItem::New("options.menu_items.usehttpkeyboard"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) httpkeyboard->SetColor(COLOR(text_colour));
		else httpkeyboard->SetColor(COLOR("#FFFFFFFF"));
		httpkeyboard->SetIcon(this->getMenuOptionIcon(inst::config::httpkeyboard));
		this->menu->AddItem(httpkeyboard);

		auto useThemeOption = pu::ui::elm::MenuItem::New("theme.theme_option"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) useThemeOption->SetColor(COLOR(text_colour));
		else useThemeOption->SetColor(COLOR("#FFFFFFFF"));
		useThemeOption->SetIcon(this->getMenuOptionIcon(inst::config::useTheme));
		this->menu->AddItem(useThemeOption);

		auto ThemeMenuOption = pu::ui::elm::MenuItem::New("theme.theme_menu"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ThemeMenuOption->SetColor(COLOR(text_colour));
		else ThemeMenuOption->SetColor(COLOR("#FFFFFFFF"));
		std::string thememenu = "romfs:/images/icons/thememenu.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_settings.theme_dl"_theme)) {
			thememenu = inst::config::appDir + "icons_settings.theme_dl"_theme;
		}
		ThemeMenuOption->SetIcon(thememenu);
		this->menu->AddItem(ThemeMenuOption);

		auto ThemeUrlOption = pu::ui::elm::MenuItem::New("theme.theme_url"_lang + inst::util::shortenString(inst::config::httplastUrl2, 42, false));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) ThemeUrlOption->SetColor(COLOR(text_colour));
		else ThemeUrlOption->SetColor(COLOR("#FFFFFFFF"));
		std::string themeurl = "romfs:/images/icons/themeurl.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_settings.theme_server"_theme)) {
			themeurl = inst::config::appDir + "icons_settings.theme_server"_theme;
		}
		ThemeUrlOption->SetIcon(themeurl);
		this->menu->AddItem(ThemeUrlOption);

		auto SigPatch = pu::ui::elm::MenuItem::New("main.menu.sig"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) SigPatch->SetColor(COLOR(text_colour));
		else SigPatch->SetColor(COLOR("#FFFFFFFF"));
		std::string sigs = "romfs:/images/icons/plaster.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_settings.patches"_theme)) {
			sigs = inst::config::appDir + "icons_settings.patches"_theme;
		}
		SigPatch->SetIcon(sigs);
		this->menu->AddItem(SigPatch);

		auto sigPatchesUrlOption = pu::ui::elm::MenuItem::New("options.menu_items.sig_url"_lang + inst::util::shortenString(inst::config::sigPatchesUrl, 42, false));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) sigPatchesUrlOption->SetColor(COLOR(text_colour));
		else sigPatchesUrlOption->SetColor(COLOR("#FFFFFFFF"));
		std::string sigsurl = "romfs:/images/icons/keyboard.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_settings.patches_server"_theme)) {
			sigsurl = inst::config::appDir + "icons_settings.patches_server"_theme;
		}
		sigPatchesUrlOption->SetIcon(sigsurl);
		this->menu->AddItem(sigPatchesUrlOption);

		auto httpServerUrlOption = pu::ui::elm::MenuItem::New("options.menu_items.http_url"_lang + inst::util::shortenString(inst::config::httpIndexUrl, 42, false));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) httpServerUrlOption->SetColor(COLOR(text_colour));
		else httpServerUrlOption->SetColor(COLOR("#FFFFFFFF"));
		std::string neturl = "romfs:/images/icons/url.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_settings.net_source"_theme)) {
			neturl = inst::config::appDir + "icons_settings.net_source"_theme;
		}
		httpServerUrlOption->SetIcon(neturl);
		this->menu->AddItem(httpServerUrlOption);

		auto languageOption = pu::ui::elm::MenuItem::New("options.menu_items.language"_lang + this->getMenuLanguage(inst::config::languageSetting));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) languageOption->SetColor(COLOR(text_colour));
		else languageOption->SetColor(COLOR("#FFFFFFFF"));
		std::string lang = "romfs:/images/icons/speak.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_settings.language"_theme)) {
			lang = inst::config::appDir + "icons_settings.language"_theme;
		}
		languageOption->SetIcon(lang);
		this->menu->AddItem(languageOption);

		auto updateOption = pu::ui::elm::MenuItem::New("options.menu_items.check_update"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) updateOption->SetColor(COLOR(text_colour));
		else updateOption->SetColor(COLOR("#FFFFFFFF"));
		std::string upd = "romfs:/images/icons/update2.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_settings.update"_theme)) {
			upd = inst::config::appDir + "icons_settings.update"_theme;
		}
		updateOption->SetIcon(upd);
		this->menu->AddItem(updateOption);

		auto creditsOption = pu::ui::elm::MenuItem::New("options.menu_items.credits"_lang);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) creditsOption->SetColor(COLOR(text_colour));
		else creditsOption->SetColor(COLOR("#FFFFFFFF"));
		std::string credit = "romfs:/images/icons/credits2.png";
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_settings.credits"_theme)) {
			credit = inst::config::appDir + "icons_settings.credits"_theme;
		}
		creditsOption->SetIcon(credit);
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
							std::string info = "romfs:/images/icons/information.png";
							if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.information"_theme)) {
								info = inst::config::appDir + "icons_others.information"_theme;
							}
							if (inst::ui::mainApp->CreateShowDialog("options.nca_warn.title"_lang, "options.nca_warn.desc"_lang, { "common.cancel"_lang, "options.nca_warn.opt1"_lang }, false, info) == 1) inst::config::validateNCAs = false;
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
						if (inst::config::useMusic) {
							inst::config::useMusic = false;
						}
						else {
							inst::config::useMusic = true;
						}
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						inst::config::setConfig();
						break;
					case 7:
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
					case 8:
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
					case 9:
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
					case 10:
						thememessage();
						inst::config::setConfig();
						this->setMenuText();
						this->menu->SetSelectedIndex(index);
						break;
					case 11:
						if (inst::util::getIPAddress() == "1.0.0.127") {
							std::string info = "romfs:/images/icons/information.png";
							if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.information"_theme)) {
								info = inst::config::appDir + "icons_others.information"_theme;
							}
							inst::ui::mainApp->CreateShowDialog("main.net.title"_lang, "main.net.desc"_lang, { "common.ok"_lang }, true, info);
							break;
						}
						mainApp->ThemeinstPage->startNetwork();
						break;
					case 12:
						keyboardResult = inst::util::softwareKeyboard("inst.net.url.hint"_lang, inst::config::httplastUrl2.c_str(), 500);
						if (keyboardResult.size() > 0) {
							inst::config::httplastUrl2 = keyboardResult;
							inst::config::setConfig();
							this->setMenuText();
							this->menu->SetSelectedIndex(index);
						}
						break;
					case 13:
						sigPatchesMenuItem_Click();
						break;
					case 14:
						keyboardResult = inst::util::softwareKeyboard("options.sig_hint"_lang, inst::config::sigPatchesUrl.c_str(), 500);
						if (keyboardResult.size() > 0) {
							inst::config::sigPatchesUrl = keyboardResult;
							inst::config::setConfig();
							this->setMenuText();
							this->menu->SetSelectedIndex(index);
						}
						break;
					case 15:
						keyboardResult = inst::util::softwareKeyboard("inst.net.url.hint"_lang, inst::config::httpIndexUrl.c_str(), 500);
						if (keyboardResult.size() > 0) {
							inst::config::httpIndexUrl = keyboardResult;
							inst::config::setConfig();
							this->setMenuText();
							this->menu->SetSelectedIndex(index);
						}
						break;
					case 16:
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
						case 9:
							inst::config::languageSetting = 9;
							break;
						default:
							inst::config::languageSetting = 0;
						}
						inst::config::setConfig();
						lang_message();
						break;
					case 17:
						if (inst::util::getIPAddress() == "1.0.0.127") {
							std::string update = "romfs:/images/icons/update.png";
							if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.update"_theme)) {
								update = inst::config::appDir + "icons_others.update"_theme;
							}
							inst::ui::mainApp->CreateShowDialog("main.net.title"_lang, "main.net.desc"_lang, { "common.ok"_lang }, true, update);
							break;
						}
						downloadUrl = inst::util::checkForAppUpdate();
						if (!downloadUrl.size()) {
							std::string fail = "romfs:/images/icons/fail.png";
							if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.fail"_theme)) {
								fail = inst::config::appDir + "icons_others.fail"_theme;
							}
							mainApp->CreateShowDialog("options.update.title_check_fail"_lang, "options.update.desc_check_fail"_lang, { "common.ok"_lang }, false, fail);
							break;
						}
						this->askToUpdate(downloadUrl);
						break;
					case 18:
						if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.credits"_theme)) {
							inst::ui::mainApp->CreateShowDialog("options.credits.title"_lang, "options.credits.desc"_lang, { "common.close"_lang }, true, inst::config::appDir + "icons_others.credits"_theme);
						}
						else {
							inst::ui::mainApp->CreateShowDialog("options.credits.title"_lang, "options.credits.desc"_lang, { "common.close"_lang }, true, "romfs:/images/icons/credits.png");
						}
						break;
					default:
						break;
					}
				}
			}
		}
	}
}