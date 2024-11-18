#pragma once
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
class MainPage : public pu::ui::Layout {
public:
  MainPage();
  PU_SMART_CTOR(MainPage)
  void installMenuItem_Click();
  void usbInstallMenuItem_Click();
  void HdInstallMenuItem_Click();
  void settingsMenuItem_Click();
  void exitMenuItem_Click();
  void onInput(u64 Down, u64 Up, const u64 Held, pu::ui::TouchPoint touch_pos);

private:
  bool appletFinished;
  TextBlock::Ref topText;
  TextBlock::Ref butText;
  pu::ui::elm::Menu::Ref menu;
  pu::ui::elm::MenuItem::Ref installMenuItem;
  pu::ui::elm::MenuItem::Ref HdInstallMenuItem;
  pu::ui::elm::MenuItem::Ref settingsMenuItem;
  pu::ui::elm::MenuItem::Ref exitMenuItem;
};
} // namespace inst::ui
