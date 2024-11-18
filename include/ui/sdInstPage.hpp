#pragma once
#include <filesystem>
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
class sdInstPage : public pu::ui::Layout {
public:
  sdInstPage();
  PU_SMART_CTOR(sdInstPage)
  pu::ui::elm::Menu::Ref menu;
  void onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos);
  void drawMenuItems(std::filesystem::path ourPath);

private:
  std::vector<std::filesystem::path> ourDirectories;
  std::vector<std::filesystem::path> ourFiles;
  std::vector<std::filesystem::path> selectedTitles;
  std::filesystem::path currentDir;
  TextBlock::Ref topText;
  TextBlock::Ref butText;
  void select();
};
} // namespace inst::ui
