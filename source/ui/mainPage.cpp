#include "ui/mainPage.hpp"
#include "HDInstall.hpp"
#include "data/buffered_placeholder_writer.hpp"
#include "nx/usbhdd.h"
#include "ui/MainApplication.hpp"
#include "usbhsfs.h"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "util/util.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <switch.h>
#include <thread>

namespace inst::ui {
extern MainApplication *mainApp;
bool appletFinished = false;

void showSpaceInfo() {
  FsFileSystem nandFS;
  fsOpenBisFileSystem(&nandFS, FsBisPartitionId_User, "");
  fsdevMountDevice("user", nandFS);
  auto [sysFree, sysUsed, sysTotal] = inst::util::getSpaceInfo("user:/");
  fsdevUnmountDevice("user");

  auto [sdFree, sdUsed, sdTotal] = inst::util::getSpaceInfo("sdmc:/");

  auto toGB = [](double size) -> std::string {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << (size / 1024 / 1024 / 1024);
    return ss.str();
  };

  auto toPercent = [](double a, double b) -> std::string {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << (b == 0.0 ? 0 : a / b * 100);
    return ss.str();
  };

  std::string Info = "usage.system_size"_lang + toGB(sysTotal) + "usage.gb"_lang + "usage.freespace"_lang +
                     toGB(sysFree) + "usage.gb"_lang + "usage.percent_used"_lang + toPercent(sysUsed, sysTotal) +
                     "usage.percent"_lang + "usage.sd_size"_lang + toGB(sdTotal) + "usage.gb"_lang +
                     "usage.sd_space"_lang + toGB(sdFree) + "usage.gb"_lang + "usage.sd_used"_lang +
                     toPercent(sdUsed, sdTotal) + "usage.percent_symbol"_lang;

  inst::ui::mainApp->CreateShowDialog("usage.space_info"_lang, Info, {"common.ok"_lang}, true,
                                      inst::util::LoadTexture("romfs:/images/icons/drive.png"));
}

void mainMenuThread() {
  bool menuLoaded = mainApp->IsShown();
  if (!appletFinished && appletGetAppletType() == AppletType_LibraryApplet) {
    tin::data::NUM_BUFFER_SEGMENTS = 2;
    if (menuLoaded) {
      inst::ui::appletFinished = true;
      mainApp->CreateShowDialog("main.applet.title"_lang, "main.applet.desc"_lang, {"common.ok"_lang}, true,
                                inst::util::LoadTexture(inst::icon::info));
    }
  } else if (!appletFinished) {
    inst::ui::appletFinished = true;
    tin::data::NUM_BUFFER_SEGMENTS = 128;
  }
}

MainPage::MainPage() : Layout::Layout() {
  this->SetBackgroundColor(COLOR("#000000FF"));

  this->topText = TextBlock::New(10, 14, "NBI v" + inst::config::appVersion);
  this->topText->SetColor(COLOR("#FFFFFFFF"));
  this->butText = TextBlock::New(10, 1028, "main.buttons"_lang);
  this->butText->SetColor(COLOR("#FFFFFFFF"));

  this->menu = pu::ui::elm::Menu::New(0, 61, 1920, COLOR("#FFFFFF00"), COLOR("#4f4f4d33"), 84, 11);
  this->menu->SetItemsFocusColor(COLOR("#4f4f4dAA"));
  // this->menu->SetScrollbarColor(COLOR("#1A1919FF"));
  this->menu->SetItemAlphaIncrementSteps(1);
  this->menu->SetShadowBaseAlpha(0);

  this->installMenuItem = pu::ui::elm::MenuItem::New("main.menu.sd"_lang);
  this->installMenuItem->SetColor(COLOR("#FFFFFFFF"));
  this->installMenuItem->SetIcon(inst::util::LoadTexture("romfs:/images/icons/micro-sd.png"));

  this->HdInstallMenuItem = pu::ui::elm::MenuItem::New("main.menu.hdd"_lang);
  this->HdInstallMenuItem->SetColor(COLOR("#FFFFFFFF"));
  this->HdInstallMenuItem->SetIcon(inst::util::LoadTexture("romfs:/images/icons/usb-hd.png"));

  this->settingsMenuItem = pu::ui::elm::MenuItem::New("main.menu.set"_lang);
  this->settingsMenuItem->SetColor(COLOR("#FFFFFFFF"));
  this->settingsMenuItem->SetIcon(inst::util::LoadTexture("romfs:/images/icons/settings.png"));

  this->exitMenuItem = pu::ui::elm::MenuItem::New("main.menu.exit"_lang);
  this->exitMenuItem->SetColor(COLOR("#FFFFFFFF"));
  this->exitMenuItem->SetIcon(inst::util::LoadTexture("romfs:/images/icons/exit-run.png"));

  this->Add(this->topText);
  this->Add(this->butText);
  this->menu->AddItem(this->installMenuItem);
  this->menu->AddItem(this->HdInstallMenuItem);
  this->menu->AddItem(this->settingsMenuItem);
  this->menu->AddItem(this->exitMenuItem);
  this->Add(this->menu);
  this->AddRenderCallback(mainMenuThread);
}

void MainPage::installMenuItem_Click() {
  mainApp->sdinstPage->drawMenuItems("sdmc:/");
  mainApp->sdinstPage->menu->SetSelectedIndex(0);
  mainApp->LoadLayout(mainApp->sdinstPage);
}

void MainPage::HdInstallMenuItem_Click() {
  if (nx::hdd::count() && nx::hdd::rootPath()) {
    mainApp->HDinstPage->drawMenuItems(nx::hdd::rootPath());
    mainApp->HDinstPage->menu->SetSelectedIndex(0);
    mainApp->LoadLayout(mainApp->HDinstPage);
  } else {
    std::string drive = "romfs:/images/icons/drive.png";
    inst::ui::mainApp->CreateShowDialog("main.hdd.title"_lang, "main.hdd.notfound"_lang, {"common.ok"_lang}, true,
                                        inst::util::LoadTexture(drive));
  }
}

void MainPage::exitMenuItem_Click() {
  mainApp->FadeOut();
  mainApp->Close();
}

void MainPage::settingsMenuItem_Click() { mainApp->LoadLayout(mainApp->optionspage); }

void MainPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {

  if (((Down & HidNpadButton_Plus) || (Down & HidNpadButton_Minus) ||
       ((Held & HidNpadButton_L) && (Down & HidNpadButton_R)) ||
       ((Down & HidNpadButton_L) && (Held & HidNpadButton_R))) &&
      mainApp->IsShown()) {
    mainApp->FadeOut();
    mainApp->Close();
  }

  HidTouchScreenState state = {0};

  if (hidGetTouchScreenStates(&state, 1)) {

    static s32 prev_touchcount = 0;
    if ((Down & HidNpadButton_A) || (state.count != prev_touchcount)) {
      prev_touchcount = state.count;

      if (prev_touchcount != 1) {
        int menuindex = this->menu->GetSelectedIndex();
        switch (menuindex) {
        case 0:
          this->installMenuItem_Click();
          break;
        case 1:
          MainPage::HdInstallMenuItem_Click();
          break;
        case 2:
          MainPage::settingsMenuItem_Click();
          break;
        case 3:
          MainPage::exitMenuItem_Click();
          break;
        default:
          break;
        }
      }
    }
  }

  if (Down & HidNpadButton_Y) {
    showSpaceInfo();
  }
}
} // namespace inst::ui
