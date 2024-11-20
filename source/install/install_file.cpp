/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "install/install_file.hpp"
#include "install/install_nsp.hpp"
#include "install/install_xci.hpp"
#include "install/sdmc_nsp.hpp"
#include "install/sdmc_xci.hpp"
#include "nx/fs.hpp"
#include "ui/MainApplication.hpp"
#include "util/config.hpp"
#include "util/file_util.hpp"
#include "util/lang.hpp"
#include "util/title_util.hpp"
#include "util/util.hpp"
#include <cstring>
#include <ctime>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

namespace inst::ui {
extern MainApplication *mainApp;
}

namespace inst {

void installNspFromFile(std::filesystem::path filePath, NcmStorageId storageId) {
  inst::util::initInstallServices();
  inst::ui::mainApp->LoadInstPage();

  bool nspInstalled = true;
  assert(storageId == NcmStorageId_SdCard || storageId == NcmStorageId_BuiltInUser);

  std::vector<int> previousClockValues;
  if (inst::config::overClock) {
    previousClockValues.push_back(inst::util::setClockSpeed(0, 1785000000)[0]);
    previousClockValues.push_back(inst::util::setClockSpeed(1, 76800000)[0]);
    previousClockValues.push_back(inst::util::setClockSpeed(2, 1600000000)[0]);
  }

  auto shortFilePath = inst::util::shortenString(filePath.filename().string(), 40, true);
  try {
    inst::ui::instPage::setTopInstInfoText("inst.info_page.top_info0"_lang + shortFilePath);

    std::unique_ptr<tin::install::Install> installTask;
    if (filePath.extension() == ".xci" || filePath.extension() == ".xcz") {
      installTask = std::make_unique<tin::install::xci::XCIInstallTask>(
          storageId, inst::config::ignoreReqVers, std::make_shared<tin::install::xci::SDMCXCI>(filePath));
    } else {
      installTask = std::make_unique<tin::install::nsp::NSPInstall>(
          storageId, inst::config::ignoreReqVers, std::make_shared<tin::install::nsp::SDMCNSP>(filePath));
    }

    inst::ui::instPage::setInstInfoText("inst.info_page.preparing"_lang);
    inst::ui::instPage::setInstBarPerc(0);

    installTask->Prepare();
    installTask->InstallTicketCert();
    installTask->Begin();

  } catch (std::exception &e) {
    inst::ui::instPage::setInstInfoText("inst.info_page.failed"_lang + shortFilePath);
    inst::ui::instPage::setInstBarPerc(0);

    inst::ui::mainApp->CreateShowDialog("inst.info_page.failed"_lang + shortFilePath + "!\n",
                                        "inst.info_page.failed_desc"_lang + "\n\n" + (std::string)e.what(),
                                        {"common.ok"_lang}, true, inst::util::LoadTexture(inst::icon::fail));
    nspInstalled = false;
  }

  if (previousClockValues.size() > 0) {
    inst::util::setClockSpeed(0, previousClockValues[0]);
    inst::util::setClockSpeed(1, previousClockValues[1]);
    inst::util::setClockSpeed(2, previousClockValues[2]);
  }

  if (nspInstalled) {
    inst::ui::instPage::setInstInfoText("inst.info_page.complete"_lang);
    inst::ui::instPage::setInstBarPerc(100);

    inst::ui::mainApp->CreateShowDialog(shortFilePath + "inst.info_page.desc1"_lang, Language::GetRandomMsg(),
                                        {"common.ok"_lang}, true, inst::util::LoadTexture(inst::icon::info));
  }

  inst::ui::mainApp->LoadMainPage();
  inst::util::deinitInstallServices();
  return;
}
} // namespace inst