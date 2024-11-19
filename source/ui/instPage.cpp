#include "ui/instPage.hpp"
#include "ui/MainApplication.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "util/util.hpp"
#include <filesystem>

namespace inst::ui {
extern MainApplication *mainApp;

instPage::instPage() : Layout::Layout() {

  this->SetBackgroundColor(COLOR("#000000FF"));

  this->pageInfoText = TextBlock::New(10, 14, "");
  this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));

  this->installInfoText = TextBlock::New(10, 74, "");
  this->installInfoText->SetColor(COLOR("#FFFFFFFF"));

  this->installBar = pu::ui::elm::ProgressBar::New(10, 134, 1900, 35, 100.0f);
  this->installBar->SetBackgroundColor(COLOR("#000000FF"));
  this->installBar->SetProgressColor(COLOR("#565759FF"));

  this->Add(this->pageInfoText);
  this->Add(this->installInfoText);
  this->Add(this->installBar);
}

void instPage::setTopInstInfoText(std::string ourText) {
  mainApp->instpage->pageInfoText->SetText(ourText);
  mainApp->CallForRender();
}

void instPage::setInstInfoText(std::string ourText) {
  mainApp->instpage->installInfoText->SetText(ourText);
  mainApp->CallForRender();
}

void instPage::setInstBarPerc(double ourPercent) {
  mainApp->instpage->installBar->SetVisible(true);
  mainApp->instpage->installBar->SetProgress(ourPercent);
  mainApp->CallForRender();
}

void instPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {}
} // namespace inst::ui
