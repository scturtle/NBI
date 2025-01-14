#include "ui/MainApplication.hpp"
#include "util/lang.hpp"

namespace inst::ui {
MainApplication *mainApp;

void MainApplication::OnLoad() {
  mainApp = this;

  Language::Load();

  this->mainPage = MainPage::New();
  this->sdinstPage = sdInstPage::New();
  this->HDinstPage = HDInstPage::New();
  this->instpage = instPage::New();
  this->optionspage = optionsPage::New();
  this->mainPage->SetOnInput(std::bind(&MainPage::onInput, this->mainPage, std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3, std::placeholders::_4));
  this->sdinstPage->SetOnInput(std::bind(&sdInstPage::onInput, this->sdinstPage, std::placeholders::_1,
                                         std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  this->HDinstPage->SetOnInput(std::bind(&HDInstPage::onInput, this->HDinstPage, std::placeholders::_1,
                                         std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  this->instpage->SetOnInput(std::bind(&instPage::onInput, this->instpage, std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3, std::placeholders::_4));
  this->optionspage->SetOnInput(std::bind(&optionsPage::onInput, this->optionspage, std::placeholders::_1,
                                          std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  this->LoadLayout(this->mainPage);
}

void MainApplication::LoadMainPage() { this->LoadLayout(this->mainPage); }
void MainApplication::LoadInstPage() {
  this->instpage->pageInfoText->SetText("");
  this->instpage->installInfoText->SetText("");
  this->instpage->installBar->SetProgress(0);
  this->instpage->installBar->SetVisible(false);
  this->LoadLayout(this->instpage);
  this->CallForRender();
}

} // namespace inst::ui
