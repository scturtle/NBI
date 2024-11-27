#include "install/nsp_or_xci.hpp"
#include "nx/nca_writer.h"
#include "ui/instPage.hpp"
#include "util/error.hpp"
#include "util/lang.hpp"
#include "util/title_util.hpp"
#include <memory>
#include <sstream>

namespace tin::install {

std::vector<const void *> NSPorXCI::GetFileEntriesByExtension(std::string extension) {
  std::vector<const void *> entryList;

  for (unsigned int i = 0; i < this->GetFileEntryNum(); i++) {
    const void *fileEntry = this->GetFileEntry(i);
    std::string name(this->GetFileEntryName(fileEntry));
    auto foundExtension = name.substr(name.find(".") + 1);

    // fix cert filename extension becoming corrupted when xcz/nsz is installing certs.
    std::string cert("cert");
    std::size_t found = name.find(cert);
    if (found != std::string::npos) {
      int pos = 0;
      std::string mystr = name;
      pos = mystr.find_last_of('.');
      mystr = mystr.substr(5, pos);
      foundExtension = mystr.substr(mystr.find(".") + 1);
    }

    if (foundExtension == extension)
      entryList.push_back(fileEntry);
  }

  return entryList;
}

const void *NSPorXCI::GetFileEntryByName(std::string name) {
  for (unsigned int i = 0; i < this->GetFileEntryNum(); i++) {
    const void *fileEntry = this->GetFileEntry(i);
    std::string foundName(this->GetFileEntryName(fileEntry));
    if (foundName == name)
      return fileEntry;
  }
  return nullptr;
}

const void *NSPorXCI::GetFileEntryByNcaId(const NcmContentId &ncaId) {
  const void *fileEntry = nullptr;
  std::string ncaIdStr = tin::util::GetNcaIdString(ncaId);

  if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".nca")) == nullptr)
    if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".cnmt.nca")) == nullptr)
      if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".ncz")) == nullptr)
        if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".cnmt.ncz")) == nullptr)
          return nullptr;

  return fileEntry;
}

void NSPorXCI::StreamToPlaceholder(std::shared_ptr<nx::ncm::ContentStorage> &contentStorage, NcmContentId ncaId) {
  const void *fileEntry = this->GetFileEntryByNcaId(ncaId);
  std::string ncaFileName = this->GetFileEntryName(fileEntry);

  LOG_DEBUG("Retrieving %s\n", ncaFileName.c_str());
  size_t ncaSize = this->GetFileEntrySize(fileEntry);

  NcaWriter writer(ncaId, contentStorage);

  float progress;

  u64 fileStart = this->GetFileEntryOffset(fileEntry);
  u64 fileOff = 0;
  size_t readSize = 0x400000; // 4MB buff
  auto readBuffer = std::make_unique<u8[]>(readSize);

  try {
    // inst::ui::instPage::setInstInfoText("inst.info_page.top_info0"_lang + ncaFileName + "...");
    inst::ui::instPage::setInstBarPerc(0);
    while (fileOff < ncaSize) {
      progress = (float)fileOff / (float)ncaSize;

      if (fileOff % (0x400000 * 3) == 0) {
        LOG_DEBUG("> Progress: %lu/%lu MB (%d%s)\r", (fileOff / 1000000), (ncaSize / 1000000), (int)(progress * 100.0),
                  "%");
        inst::ui::instPage::setInstBarPerc((double)(progress * 100.0));
        //
        std::stringstream x;
        x << (int)(progress * 100.0);
        inst::ui::instPage::setInstInfoText("inst.info_page.top_info0"_lang + ncaFileName + " " + x.str() + "%");
      }

      if (fileOff + readSize >= ncaSize)
        readSize = ncaSize - fileOff;

      this->BufferData(readBuffer.get(), fileOff + fileStart, readSize);
      writer.write(readBuffer.get(), readSize);

      fileOff += readSize;
    }
    inst::ui::instPage::setInstBarPerc(100);
  } catch (std::exception &e) {
    LOG_DEBUG("something went wrong: %s\n", e.what());
  }

  writer.close();
}
} // namespace tin::install
