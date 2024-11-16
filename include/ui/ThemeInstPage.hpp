#pragma once
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
class ThemeInstPage : public pu::ui::Layout {
public:
  ThemeInstPage();
  PU_SMART_CTOR(ThemeInstPage)
  void onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos);
  void startNetwork();
  static void setInstBarPerc(double ourPercent);
  pu::ui::elm::ProgressBar::Ref installBar;
  TextBlock::Ref pageInfoText;

private:
  std::vector<std::string> ourUrls;
  std::vector<std::string> modded;
  std::vector<std::string> selectedUrls;
  std::vector<std::string> alternativeNames;
  TextBlock::Ref butText;
  Rectangle::Ref topRect;
  Rectangle::Ref infoRect;
  Rectangle::Ref botRect;
  Image::Ref titleImage;
  TextBlock::Ref appVersionText;
  pu::ui::elm::Menu::Ref menu;
  Image::Ref infoImage;
  void drawMenuItems(bool clearItems);
  void selectTitle(int selectedIndex);
};
} // namespace inst::ui