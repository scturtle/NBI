#pragma once
#include <filesystem>
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
	class HDInstPage : public pu::ui::Layout
	{
	public:
		HDInstPage();
		PU_SMART_CTOR(HDInstPage)
			pu::ui::elm::Menu::Ref menu;
		void startInstall();
		void onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos);
		TextBlock::Ref pageInfoText;
		void drawMenuItems(bool clearItems, std::filesystem::path ourPath);
	private:
		std::vector<std::filesystem::path> ourDirectories;
		std::vector<std::filesystem::path> ourFiles;
		std::vector<std::filesystem::path> selectedTitles;
		std::filesystem::path currentDir;
		TextBlock::Ref butText;
		Rectangle::Ref topRect;
		Rectangle::Ref infoRect;
		Rectangle::Ref botRect;
		Image::Ref titleImage;
		TextBlock::Ref appVersionText;
		void followDirectory();
		void selectNsp(int selectedIndex);
		bool show_file_ext;
	};
}