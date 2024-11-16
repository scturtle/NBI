#include "ui/usbInstPage.hpp"
#include "ui/MainApplication.hpp"
#include "usbInstall.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "util/theme.hpp"
#include "util/util.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
extern MainApplication *mainApp;
s32 www = 0; // touchscreen variable
std::string usb_root = inst::config::appDir + "/theme";
bool usb_theme = util::themeit(usb_root); // check if we have a previous theme directory first.

std::string checked_usb = "romfs:/images/icons/check-box-outline.png";
std::string unchecked_usb = "romfs:/images/icons/checkbox-blank-outline.png";
pu::sdl2::TextureHandle::Ref checked_usb_tex = pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(checked_usb));
pu::sdl2::TextureHandle::Ref unchecked_usb_tex = pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(unchecked_usb));

void checkbox_usb() {
  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") &&
      std::filesystem::exists(inst::config::appDir + "icons_others.checkbox-checked"_theme)) {
    checked_usb = inst::config::appDir + "icons_others.checkbox-checked"_theme;
    checked_usb_tex = pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(checked_usb));
  }
  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") &&
      std::filesystem::exists(inst::config::appDir + "icons_others.checkbox-empty"_theme)) {
    unchecked_usb = inst::config::appDir + "icons_others.checkbox-empty"_theme;
    unchecked_usb_tex = pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(unchecked_usb));
  }
}

usbInstPage::usbInstPage() : Layout::Layout() {
  std::string infoRect_colour = "colour.inforect"_theme;
  std::string bg_colour = "colour.background"_theme;
  std::string tbar_colour = "colour.topbar"_theme;
  std::string bbar_colour = "colour.bottombar"_theme;
  std::string usb_top = inst::config::appDir + "bg_images.usb_top"_theme;
  std::string default_background = inst::config::appDir + "bg_images.default_background"_theme;
  std::string pageinfo_colour = "colour.pageinfo_text"_theme;
  std::string bottombar_text = "colour.bottombar_text"_theme;
  std::string background_overlay1 = "colour.background_overlay1"_theme;
  std::string background_overlay2 = "colour.background_overlay2"_theme;
  std::string focus = "colour.focus"_theme;
  std::string scrollbar = "colour.scrollbar"_theme;
  std::string waiting = inst::config::appDir + "icons_others.waiting_usb"_theme;

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
    this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR(infoRect_colour));
  else
    this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#00000080"));

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
    this->SetBackgroundColor(COLOR(bg_colour));
  else
    this->SetBackgroundColor(COLOR("#000000FF"));

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
    this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR(tbar_colour));
  else
    this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#000000FF"));

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
    this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR(bbar_colour));
  else
    this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR("#000000FF"));

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") &&
      std::filesystem::exists(usb_top))
    this->titleImage = Image::New(0, 0, pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage((usb_top))));
  else
    this->titleImage =
        Image::New(0, 0, pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage("romfs:/images/Usb.png")));

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") &&
      std::filesystem::exists(default_background))
    this->SetBackgroundImage(pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(default_background)));
  else
    this->SetBackgroundImage(pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage("romfs:/images/Background.png")));

  this->pageInfoText = TextBlock::New(10, 109, "inst.hd.top_info"_lang);
  this->pageInfoText->SetFont(pu::ui::MakeDefaultFontName(30));

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
    this->pageInfoText->SetColor(COLOR(pageinfo_colour));
  else
    this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));

  this->butText = TextBlock::New(10, 678, "inst.hd.buttons"_lang);

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
    this->butText->SetColor(COLOR(bottombar_text));
  else
    this->butText->SetColor(COLOR("#FFFFFFFF"));

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
    this->menu =
        pu::ui::elm::Menu::New(0, 156, 1280, COLOR(background_overlay1), COLOR(background_overlay2), 84, (506 / 84));
  else
    this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), COLOR("#4f4f4d33"), 84, (506 / 84));

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
    this->menu->SetItemsFocusColor(COLOR(focus));
  else
    this->menu->SetItemsFocusColor(COLOR("#4f4f4dAA"));

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
    this->menu->SetScrollbarColor(COLOR(scrollbar));
  else
    this->menu->SetScrollbarColor(COLOR("#1A1919FF"));

  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") &&
      std::filesystem::exists(waiting))
    this->infoImage = Image::New(453, 292, pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(waiting)));
  else
    this->infoImage = Image::New(
        453, 292,
        pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage("romfs:/images/icons/usb-connection-waiting.png")));

  this->Add(this->topRect);
  this->Add(this->infoRect);
  this->Add(this->botRect);
  this->Add(this->titleImage);
  this->Add(this->butText);
  this->Add(this->pageInfoText);
  this->Add(this->menu);
  this->Add(this->infoImage);
  checkbox_usb();
}

void usbInstPage::drawMenuItems_withext(bool clearItems) {
  int myindex = this->menu->GetSelectedIndex(); // store index so when page redraws we can get the last item we checked.
  if (clearItems)
    this->selectedTitles = {};
  std::string text_colour = "colour.main_text"_theme;
  this->menu->ClearItems();
  for (auto &url : this->ourTitles) {
    std::string itm = inst::util::shortenString(inst::util::formatUrlString(url), 56, true);
    auto ourEntry = pu::ui::elm::MenuItem::New(itm);

    if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
      ourEntry->SetColor(COLOR(text_colour));
    else
      ourEntry->SetColor(COLOR("#FFFFFFFF"));

    ourEntry->SetIcon(unchecked_usb_tex);
    for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
      if (this->selectedTitles[i] == url) {
        ourEntry->SetIcon(checked_usb_tex);
      }
    }
    this->menu->AddItem(ourEntry);
    this->menu->SetSelectedIndex(myindex); // jump to the index we saved from above
  }
}

void usbInstPage::drawMenuItems(bool clearItems) {
  int myindex = this->menu->GetSelectedIndex(); // store index so when page redraws we can get the last item we checked.
  if (clearItems)
    this->selectedTitles = {};
  std::string text_colour = "colour.main_text"_theme;
  this->menu->ClearItems();
  for (auto &url : this->ourTitles) {

    std::string base_filename = url.substr(url.find_last_of("/") + 1); // just get the filename
    std::string::size_type const p(base_filename.find_last_of('.'));
    std::string file_without_extension = base_filename.substr(0, p); // strip of file extension
    std::string itm = inst::util::shortenString(inst::util::formatUrlString(file_without_extension), 56, true);

    // std::string itm = inst::util::shortenString(inst::util::formatUrlString(url), 56, true);
    auto ourEntry = pu::ui::elm::MenuItem::New(itm);

    if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json"))
      ourEntry->SetColor(COLOR(text_colour));
    else
      ourEntry->SetColor(COLOR("#FFFFFFFF"));
    ourEntry->SetIcon(unchecked_usb_tex);

    for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
      if (this->selectedTitles[i] == url) {
        ourEntry->SetIcon(checked_usb_tex);
      }
    }
    this->menu->AddItem(ourEntry);
    this->menu->SetSelectedIndex(myindex); // jump to the index we saved from above
  }
}

void usbInstPage::selectTitle(int selectedIndex) {
  if (this->menu->GetItems()[selectedIndex]->GetIconTexture() == checked_usb_tex) {
    for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
      if (this->selectedTitles[i] == this->ourTitles[selectedIndex])
        this->selectedTitles.erase(this->selectedTitles.begin() + i);
    }
  } else
    this->selectedTitles.push_back(this->ourTitles[selectedIndex]);
  this->drawMenuItems(false);
}

void usbInstPage::startUsb() {
  this->pageInfoText->SetText("inst.usb.top_info"_lang);
  this->butText->SetText("inst.usb.buttons"_lang);
  this->menu->SetVisible(false);
  this->menu->ClearItems();
  this->infoImage->SetVisible(true);
  mainApp->LoadLayout(mainApp->usbinstPage);
  mainApp->CallForRender();
  this->ourTitles = usbInstStuff::OnSelected();
  if (!this->ourTitles.size()) {
    mainApp->LoadLayout(mainApp->mainPage);
    return;
  } else {
    mainApp->CallForRender(); // If we re-render a few times during this process the main screen won't flicker
    this->pageInfoText->SetText("inst.usb.top_info2"_lang);
    this->butText->SetText("inst.usb.buttons2"_lang);
    this->drawMenuItems(true);
    this->menu->SetSelectedIndex(0);
    mainApp->CallForRender();
    this->infoImage->SetVisible(false);
    this->menu->SetVisible(true);
  }
  return;
}

void usbInstPage::startInstall() {
  int dialogResult = -1;

  std::string install = "romfs:/images/icons/install.png";
  if (usb_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") &&
      std::filesystem::exists(inst::config::appDir + "icons_others.install"_theme)) {
    install = inst::config::appDir + "icons_others.install"_theme;
  }

  if (this->selectedTitles.size() == 1) {
    dialogResult = mainApp->CreateShowDialog(
        "inst.target.desc0"_lang + ":\n\n" +
            inst::util::shortenString(std::filesystem::path(this->selectedTitles[0]).filename().string(), 32, true) +
            "\n\n" + "inst.target.desc1"_lang,
        "\n\n\n\n\n\n\n" + "common.cancel_desc"_lang, {"inst.target.opt0"_lang, "inst.target.opt1"_lang}, false,
        pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(install)));
  }
  // else dialogResult = mainApp->CreateShowDialog("inst.target.desc00"_lang +
  // std::to_string(this->selectedTitles.size()) + "inst.target.desc01"_lang, "common.cancel_desc"_lang, {
  // "inst.target.opt0"_lang, "inst.target.opt1"_lang }, false);
  else
    dialogResult = mainApp->CreateShowDialog(
        "inst.target.desc00"_lang + std::to_string(this->selectedTitles.size()) + "inst.target.desc01"_lang,
        "\n" + "common.cancel_desc"_lang, {"inst.target.opt0"_lang, "inst.target.opt1"_lang}, false,
        pu::sdl2::TextureHandle::New(pu::ui::render::LoadImage(install)));
  if (dialogResult == -1)
    return;
  usbInstStuff::installTitleUsb(this->selectedTitles, dialogResult);
  return;
}

void usbInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {
  if (Down & HidNpadButton_B) {
    mainApp->LoadLayout(mainApp->mainPage);
  }

  HidTouchScreenState state = {0};

  if (hidGetTouchScreenStates(&state, 1)) {

    if ((Down & HidNpadButton_A) || (state.count != www)) {
      www = state.count;

      if (www != 1) {
        int var = this->menu->GetItems().size();
        auto s = std::to_string(var);
        if (s == "0") {
          // do nothing here because there's no items in the list, that way the app won't freeze
        } else {
          this->selectTitle(this->menu->GetSelectedIndex());
          if (this->menu->GetItems().size() == 1 && this->selectedTitles.size() == 1) {
            this->startInstall();
          }
        }
      }
    }
  }

  if ((Down & HidNpadButton_Y)) {
    if (this->selectedTitles.size() == this->menu->GetItems().size())
      this->drawMenuItems(true);
    else {
      for (long unsigned int i = 0; i < this->menu->GetItems().size(); i++) {
        if (this->menu->GetItems()[i]->GetIconTexture() == checked_usb_tex)
          continue;
        else
          this->selectTitle(i);
      }
      this->drawMenuItems(false);
    }
  }

  if (Down & HidNpadButton_Plus) {
    int var = this->menu->GetItems().size();
    auto s = std::to_string(var);

    if (s != "0") {
      if (this->selectedTitles.size() == 0) {
        this->selectTitle(this->menu->GetSelectedIndex());
        this->startInstall();
        return;
      }
      this->startInstall();
    }
  }

  if (Down & HidNpadButton_ZL)
    this->menu->SetSelectedIndex(std::max(0, this->menu->GetSelectedIndex() - 6));

  if (Down & HidNpadButton_ZR)
    this->menu->SetSelectedIndex(std::min((s32)this->menu->GetItems().size() - 1, this->menu->GetSelectedIndex() + 6));

  // goto top of list
  if (Down & HidNpadButton_L) {
    int x = this->menu->GetItems().size() - 1;
    this->menu->SetSelectedIndex(std::max(0, this->menu->GetSelectedIndex() - x));
  }

  // goto bottom of list
  if (Down & HidNpadButton_R) {
    int x = this->menu->GetItems().size() - 1;
    this->menu->SetSelectedIndex(std::min((s32)this->menu->GetItems().size() - 1, this->menu->GetSelectedIndex() + x));
  }

  // don't show file extensions
  if (Down & HidNpadButton_Left) {
    this->drawMenuItems(true);
  }

  // show file extensions
  if (Down & HidNpadButton_Right) {
    this->drawMenuItems_withext(true);
  }
}
} // namespace inst::ui
