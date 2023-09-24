#pragma once
#include <pu/Plutonium>
#include "ui/mainPage.hpp"
#include "ui/netInstPage.hpp"
#include "ui/ThemeInstPage.hpp"
#include "ui/sdInstPage.hpp"
#include "ui/HDInstPage.hpp"
#include "ui/usbInstPage.hpp"
#include "ui/instPage.hpp"
#include "ui/optionsPage.hpp"

namespace inst::ui {
	class MainApplication : public pu::ui::Application {
	public:
		using Application::Application;
		PU_SMART_CTOR(MainApplication)
			void OnLoad() override;
		MainPage::Ref mainPage;
		netInstPage::Ref netinstPage;
		ThemeInstPage::Ref ThemeinstPage;
		sdInstPage::Ref sdinstPage;
		HDInstPage::Ref HDinstPage;
		usbInstPage::Ref usbinstPage;
		instPage::Ref instpage;
		optionsPage::Ref optionspage;
	};
}