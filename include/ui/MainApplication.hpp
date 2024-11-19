#pragma once
#include "ui/HDInstPage.hpp"
#include "ui/instPage.hpp"
#include "ui/mainPage.hpp"
#include "ui/optionsPage.hpp"
#include "ui/sdInstPage.hpp"
#include <pu/Plutonium>

namespace inst::ui {
class MainApplication : public pu::ui::Application {
public:
  using Application::Application;
  PU_SMART_CTOR(MainApplication)
  void OnLoad() override;
  MainPage::Ref mainPage;
  sdInstPage::Ref sdinstPage;
  HDInstPage::Ref HDinstPage;
  instPage::Ref instpage;
  optionsPage::Ref optionspage;
  void LoadMainPage();
  void LoadInstPage();
};
} // namespace inst::ui
