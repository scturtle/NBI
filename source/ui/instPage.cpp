#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/instPage.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "util/theme.hpp"
#include <sys/statvfs.h>

FsFileSystem* fs;
FsFileSystem devices[4];
int statvfs(const char* path, struct statvfs* buf);

double GetSpace(const char* path)
{
	struct statvfs stat;
	if (statvfs(path, &stat) != 0) {
		return -1;
	}
	return stat.f_bsize * stat.f_bavail;
}

#define COLOR(hex) pu::ui::Color::FromHex(hex)

//using namespace std;

namespace inst::ui {
	extern MainApplication* mainApp;

	instPage::instPage() : Layout::Layout() {

		std::string infoRect_colour = "colour.inforect"_theme;
		std::string bg_colour = "colour.background"_theme;
		std::string tbar_colour = "colour.topbar"_theme;
		std::string install_top = inst::config::appDir + "bg_images.install_top"_theme;
		std::string default_background = inst::config::appDir + "bg_images.default_background"_theme;
		std::string pageinfo_colour = "colour.pageinfo_text"_theme;
		std::string installinfo_colour = "colour.installinfo_text"_theme;
		std::string sdinfo_colour = "colour.sdinfo_text"_theme;
		std::string nandinfo_colour = "colour.nandinfo_text"_theme;
		std::string count_colour = "colour.count_text"_theme;
		std::string progress_bg_colour = "colour.progress_bg"_theme;
		std::string progress_fg_colour = "colour.progress_fg"_theme;

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR(infoRect_colour));
		else this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#00000080"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->SetBackgroundColor(COLOR(bg_colour));
		else this->SetBackgroundColor(COLOR("#000000FF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR(tbar_colour));
		else this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#000000FF"));

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(install_top)) this->titleImage = Image::New(0, 0, (install_top));
		else this->titleImage = Image::New(0, 0, "romfs:/images/Install.png");

		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(default_background)) this->SetBackgroundImage(default_background);
		else this->SetBackgroundImage("romfs:/images/Background.png");

		this->pageInfoText = TextBlock::New(10, 109, "");
		this->pageInfoText->SetFont(pu::ui::MakeDefaultFontName(30));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->pageInfoText->SetColor(COLOR(pageinfo_colour));
		else this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));

		this->installInfoText = TextBlock::New(10, 640, "");
		this->installInfoText->SetFont(pu::ui::MakeDefaultFontName(30));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->installInfoText->SetColor(COLOR(installinfo_colour));
		else this->installInfoText->SetColor(COLOR("#FFFFFFFF"));

		this->sdInfoText = TextBlock::New(10, 600, "");
		this->sdInfoText->SetFont(pu::ui::MakeDefaultFontName(30));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->sdInfoText->SetColor(COLOR(sdinfo_colour));
		else this->sdInfoText->SetColor(COLOR("#FFFFFFFF"));

		this->nandInfoText = TextBlock::New(10, 560, "");
		this->nandInfoText->SetFont(pu::ui::MakeDefaultFontName(30));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->nandInfoText->SetColor(COLOR(nandinfo_colour));
		else this->nandInfoText->SetColor(COLOR("#FFFFFFFF"));

		this->countText = TextBlock::New(10, 520, "");
		this->countText->SetFont(pu::ui::MakeDefaultFontName(30));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->countText->SetColor(COLOR(count_colour));
		else this->countText->SetColor(COLOR("#FFFFFFFF"));

		//this->installBar = pu::ui::elm::ProgressBar::New(10, 680, 1260, 30, 100.0f);
		this->installBar = pu::ui::elm::ProgressBar::New(10, 675, 1260, 35, 100.0f);
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->installBar->SetBackgroundColor(COLOR(progress_bg_colour));
		else this->installBar->SetBackgroundColor(COLOR("#000000FF"));
		if (inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json")) this->installBar->SetProgressColor(COLOR(progress_fg_colour));
		else this->installBar->SetProgressColor(COLOR("#565759FF"));

		this->Add(this->topRect);
		this->Add(this->infoRect);
		this->Add(this->titleImage);
		this->Add(this->pageInfoText);
		this->Add(this->installInfoText);
		this->Add(this->sdInfoText);
		this->Add(this->nandInfoText);
		this->Add(this->countText);
		this->Add(this->installBar);
	}

	Result sdfreespace() {
		devices[0] = *fsdevGetDeviceFileSystem("sdmc");
		fs = &devices[0];
		Result rc = fsOpenSdCardFileSystem(fs);
		double mb = 0;
		if (R_FAILED(rc)) {
			return 0;
		}
		else {
			mb = (GetSpace("sdmc:/") / 1024) / 1024; //megabytes
		}
		return mb;
	}

	Result sysfreespace() {
		FsFileSystem nandFS;
		Result rc = fsOpenBisFileSystem(&nandFS, FsBisPartitionId_User, "");
		fsdevMountDevice("user", nandFS);
		double mb = 0;
		if (R_FAILED(rc)) {
			return 0;
		}
		else {
			mb = (GetSpace("user:/") / 1024) / 1024; //megabytes
		}
		fsdevUnmountDevice("user");
		return mb;
	}

	void instPage::setTopInstInfoText(std::string ourText) {
		mainApp->instpage->pageInfoText->SetText(ourText);
		mainApp->CallForRender();
	}

	void instPage::filecount(std::string ourText) {
		mainApp->instpage->countText->SetText(ourText);
		mainApp->CallForRender();
	}

	void instPage::setInstInfoText(std::string ourText) {
		mainApp->instpage->installInfoText->SetText(ourText);
		//
		std::string info = std::to_string(sdfreespace());
		std::string message = ("inst.net.sd"_lang + info + " MB");
		mainApp->instpage->sdInfoText->SetText(message);
		//
		info = std::to_string(sysfreespace());
		message = ("inst.net.nand"_lang + info + " MB");
		mainApp->instpage->nandInfoText->SetText(message);
		//
		mainApp->CallForRender();
	}

	void instPage::setInstBarPerc(double ourPercent) {
		mainApp->instpage->installBar->SetVisible(true);
		mainApp->instpage->installBar->SetProgress(ourPercent);
		mainApp->CallForRender();
	}

	void instPage::loadMainMenu() {
		mainApp->LoadLayout(mainApp->mainPage);
	}

	void instPage::loadInstallScreen() {
		mainApp->instpage->pageInfoText->SetText("");
		mainApp->instpage->installInfoText->SetText("");
		mainApp->instpage->sdInfoText->SetText("");
		mainApp->instpage->nandInfoText->SetText("");
		mainApp->instpage->countText->SetText("");
		mainApp->instpage->installBar->SetProgress(0);
		mainApp->instpage->installBar->SetVisible(false);
		mainApp->LoadLayout(mainApp->instpage);
		mainApp->CallForRender();
	}

	void instPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {
	}
}