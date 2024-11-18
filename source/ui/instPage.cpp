#include "ui/instPage.hpp"
#include "ui/MainApplication.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "util/util.hpp"
#include <filesystem>
#include <sys/statvfs.h>

FsFileSystem *fs;
FsFileSystem devices[4];
int statvfs(const char *path, struct statvfs *buf);

double GetSpace(const char *path) {
  struct statvfs stat;
  if (statvfs(path, &stat) != 0) {
    return -1;
  }
  return stat.f_bsize * stat.f_bavail;
}

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
extern MainApplication *mainApp;

instPage::instPage() : Layout::Layout() {

  this->SetBackgroundColor(COLOR("#000000FF"));

  this->pageInfoText = TextBlock::New(10, 14, "");
  this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));

  this->countText = TextBlock::New(10, 55, "");
  this->countText->SetColor(COLOR("#FFFFFFFF"));

  this->nandInfoText = TextBlock::New(10, 95, "");
  this->nandInfoText->SetColor(COLOR("#FFFFFFFF"));

  this->sdInfoText = TextBlock::New(10, 135, "");
  this->sdInfoText->SetColor(COLOR("#FFFFFFFF"));

  this->installInfoText = TextBlock::New(10, 175, "");
  this->installInfoText->SetColor(COLOR("#FFFFFFFF"));

  this->installBar = pu::ui::elm::ProgressBar::New(10, 215, 1900, 35, 100.0f);
  this->installBar->SetBackgroundColor(COLOR("#000000FF"));
  this->installBar->SetProgressColor(COLOR("#565759FF"));

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
  } else {
    mb = (GetSpace("sdmc:/") / 1024) / 1024; // megabytes
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
  } else {
    mb = (GetSpace("user:/") / 1024) / 1024; // megabytes
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
  std::string info = std::to_string(sdfreespace());
  std::string message = ("inst.net.sd"_lang + info + " MB");
  mainApp->instpage->sdInfoText->SetText(message);
  info = std::to_string(sysfreespace());
  message = ("inst.net.nand"_lang + info + " MB");
  mainApp->instpage->nandInfoText->SetText(message);
  mainApp->CallForRender();
}

void instPage::setInstBarPerc(double ourPercent) {
  mainApp->instpage->installBar->SetVisible(true);
  mainApp->instpage->installBar->SetProgress(ourPercent);
  mainApp->CallForRender();
}

void instPage::loadMainMenu() { mainApp->LoadLayout(mainApp->mainPage); }

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

void instPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {}
} // namespace inst::ui
