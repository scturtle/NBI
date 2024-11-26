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

#include "install/install.hpp"

#include "install/nca.hpp"
#include "nx/fs.hpp"
#include "nx/ncm.hpp"
#include "ui/MainApplication.hpp"
#include "util/config.hpp"
#include "util/crypto.hpp"
#include "util/error.hpp"
#include "util/file_util.hpp"
#include "util/lang.hpp"
#include "util/title_util.hpp"
#include "util/util.hpp"

#include <cstring>
#include <memory>
#include <switch.h>

namespace inst::ui {
extern MainApplication *mainApp;
}

// TODO: Check NCA files are present
// TODO: Check tik/cert is present
namespace tin::install {
Install::Install(NcmStorageId destStorageId, bool ignoreReqFirmVersion, std::shared_ptr<NSPorXCI> nsp_or_xci)
    : m_destStorageId(destStorageId), m_ignoreReqFirmVersion(ignoreReqFirmVersion), m_nsp_or_xci(nsp_or_xci),
      m_contentMeta() {
  appletSetMediaPlaybackState(true);
  m_nsp_or_xci->RetrieveHeader();
}

Install::~Install() { appletSetMediaPlaybackState(false); }

std::vector<std::tuple<nx::ncm::ContentMeta, NcmContentInfo>> Install::ReadCNMT() {
  std::vector<std::tuple<nx::ncm::ContentMeta, NcmContentInfo>> CNMTList;

  for (const void *fileEntry : m_nsp_or_xci->GetFileEntriesByExtension("cnmt.nca")) {
    std::string cnmtNcaName(m_nsp_or_xci->GetFileEntryName(fileEntry));
    NcmContentId cnmtContentId = tin::util::GetNcaIdFromString(cnmtNcaName);
    size_t cnmtNcaSize = m_nsp_or_xci->GetFileEntrySize(fileEntry);

    nx::ncm::ContentStorage contentStorage(m_destStorageId);

    LOG_DEBUG("CNMT Name: %s\n", cnmtNcaName.c_str());

    // We install the cnmt nca early to read from it later
    this->InstallNCA(cnmtContentId);
    std::string cnmtNCAFullPath = contentStorage.GetPath(cnmtContentId);

    NcmContentInfo cnmtContentInfo;
    cnmtContentInfo.content_id = cnmtContentId;
    //*(u64*)&cnmtContentInfo.size = cnmtNcaSize & 0xFFFFFFFFFFFF;
    ncmU64ToContentInfoSize(cnmtNcaSize & 0xFFFFFFFFFFFF, &cnmtContentInfo);
    cnmtContentInfo.content_type = NcmContentType_Meta;

    CNMTList.push_back({tin::util::GetContentMetaFromNCA(cnmtNCAFullPath), cnmtContentInfo});
  }
  return CNMTList;
}

void Install::InstallNCA(const NcmContentId &ncaId) {
  const void *fileEntry = m_nsp_or_xci->GetFileEntryByNcaId(ncaId);
  std::string ncaFileName = m_nsp_or_xci->GetFileEntryName(fileEntry);

  std::shared_ptr<nx::ncm::ContentStorage> contentStorage(new nx::ncm::ContentStorage(m_destStorageId));

  // Attempt to delete any leftover placeholders
  try {
    contentStorage->DeletePlaceholder(*(NcmPlaceHolderId *)&ncaId);
  } catch (...) {
  }
  // Attempt to delete leftover ncas
  try {
    contentStorage->Delete(ncaId);
  } catch (...) {
  }

  if (inst::config::validateNCAs && !m_declinedValidation) {
    tin::install::NcaHeader header;
    m_nsp_or_xci->BufferData(&header, m_nsp_or_xci->GetDataOffset() + m_nsp_or_xci->GetFileEntryOffset(fileEntry),
                             sizeof(tin::install::NcaHeader));

    Crypto::AesXtr crypto(Crypto::Keys().headerKey, false);
    crypto.decrypt(&header, &header, sizeof(tin::install::NcaHeader), 0, 0x200);
    // https://gbatemp.net/threads/nszip-nsp-compressor-decompressor-to-reduce-storage.530313/

    if (header.magic != MAGIC_NCA3)
      THROW_FORMAT("Invalid NCA magic");

    if (!Crypto::rsa2048PssVerify(&header.magic, 0x200, header.fixed_key_sig, Crypto::NCAHeaderSignature)) {
      int rc = inst::ui::mainApp->CreateShowDialog("inst.nca_verify.title"_lang, "inst.nca_verify.desc"_lang,
                                                   {"common.cancel"_lang, "inst.nca_verify.opt1"_lang}, false,
                                                   inst::util::LoadTexture(inst::icon::info));
      if (rc != 1)
        THROW_FORMAT("%s", ("inst.nca_verify.error"_lang + tin::util::GetNcaIdString(ncaId)).c_str());
      m_declinedValidation = true;
    }
  }

  m_nsp_or_xci->StreamToPlaceholder(contentStorage, ncaId);

  LOG_DEBUG("Registering placeholder...\n");

  try {
    contentStorage->Register(*(NcmPlaceHolderId *)&ncaId, ncaId);
  } catch (...) {
    LOG_DEBUG(("Failed to register " + ncaFileName + ". It may already exist.\n").c_str());
  }

  try {
    contentStorage->DeletePlaceholder(*(NcmPlaceHolderId *)&ncaId);
  } catch (...) {
  }
}

void Install::InstallTicketCert() {
  // int cal = 0;
  std::vector<const void *> tikFileEntries = m_nsp_or_xci->GetFileEntriesByExtension("tik");
  std::vector<const void *> certFileEntries = m_nsp_or_xci->GetFileEntriesByExtension("cert");

  for (size_t i = 0; i < tikFileEntries.size(); i++) {
    u64 tikSize = m_nsp_or_xci->GetFileEntrySize(tikFileEntries[i]);
    auto tikBuf = std::make_unique<u8[]>(tikSize);
    m_nsp_or_xci->BufferData(
        tikBuf.get(), m_nsp_or_xci->GetDataOffset() + m_nsp_or_xci->GetFileEntryOffset(tikFileEntries[i]), tikSize);

    u64 certSize = m_nsp_or_xci->GetFileEntrySize(certFileEntries[i]);
    auto certBuf = std::make_unique<u8[]>(certSize);
    m_nsp_or_xci->BufferData(
        certBuf.get(), m_nsp_or_xci->GetDataOffset() + m_nsp_or_xci->GetFileEntryOffset(certFileEntries[i]), certSize);

    // try to fix a temp ticket and change it t a permanent one
    // https://switchbrew.org/wiki/Ticket#Certificate_chain
    if (inst::config::fixticket) {
      u16 ECDSA = 0;
      u16 RSA_2048 = 0;
      u16 RSA_4096 = 0;

      ECDSA = (0x4 + 0x3C + 0x40 + 0x146);
      RSA_2048 = (0x4 + 0x100 + 0x3C + 0x146);
      RSA_4096 = (0x4 + 0x200 + 0x3C + 0x146);

      if (tikBuf.get()[0x0] == 0x5 && (tikBuf.get()[ECDSA] == 0x10 || tikBuf.get()[ECDSA] == 0x30)) {
        tikBuf.get()[ECDSA] = 0x0;
        tikBuf.get()[ECDSA - 1] = 0x10; // fix broken 	Master key revision
      }

      // RSA_2048 SHA256
      else if (tikBuf.get()[0x0] == 0x4 && (tikBuf.get()[RSA_2048] == 0x10 || tikBuf.get()[RSA_2048] == 0x30)) {
        tikBuf.get()[RSA_2048] = 0x0;
        tikBuf.get()[RSA_2048 - 1] = 0x10;
      }

      // RSA_4096 SHA256
      else if (tikBuf.get()[0x0] == 0x3 && (tikBuf.get()[RSA_4096] == 0x10 || tikBuf.get()[RSA_4096] == 0x30)) {
        tikBuf.get()[RSA_4096] = 0x0;
        tikBuf.get()[RSA_4096 - 1] = 0x10;
      }

      // ECDSA SHA1
      else if (tikBuf.get()[0x0] == 0x2 && (tikBuf.get()[ECDSA] == 0x10 || tikBuf.get()[ECDSA] == 0x30)) {
        tikBuf.get()[ECDSA] = 0x0;
        tikBuf.get()[ECDSA - 1] = 0x10;
      }

      // RSA_2048 SHA1
      else if (tikBuf.get()[0x0] == 0x1 && (tikBuf.get()[RSA_2048] == 0x10 || tikBuf.get()[RSA_2048] == 0x30)) {
        tikBuf.get()[RSA_2048] = 0x0;
        tikBuf.get()[RSA_2048 - 1] = 0x10;
      }

      // RSA_4096 SHA1
      else if (tikBuf.get()[0x0] == 0x0 && (tikBuf.get()[RSA_4096] == 0x10 || tikBuf.get()[RSA_4096] == 0x30)) {
        tikBuf.get()[RSA_4096] = 0x0;
        tikBuf.get()[RSA_4096 - 1] = 0x10;
      }
    }

    // Finally, let's actually import the ticket
    ASSERT_OK(esImportTicket(tikBuf.get(), tikSize, certBuf.get(), certSize), "Failed to import ticket");
  }
}

// TODO: Implement RAII on NcmContentMetaDatabase
void Install::InstallContentMetaRecords(tin::data::ByteBuffer &installContentMetaBuf, int i) {
  NcmContentMetaDatabase contentMetaDatabase;
  NcmContentMetaKey contentMetaKey = m_contentMeta[i].GetContentMetaKey();

  try {
    ASSERT_OK(ncmOpenContentMetaDatabase(&contentMetaDatabase, m_destStorageId),
              "Failed to open content meta database");
    ASSERT_OK(ncmContentMetaDatabaseSet(&contentMetaDatabase, &contentMetaKey,
                                        (NcmContentMetaHeader *)installContentMetaBuf.GetData(),
                                        installContentMetaBuf.GetSize()),
              "Failed to set content records");
    ASSERT_OK(ncmContentMetaDatabaseCommit(&contentMetaDatabase), "Failed to commit content records");
  } catch (std::runtime_error &e) {
    serviceClose(&contentMetaDatabase.s);
    THROW_FORMAT("%s", e.what());
  }

  serviceClose(&contentMetaDatabase.s);
}

void Install::InstallApplicationRecord(int i) {
  Result rc = 0;
  std::vector<ContentStorageRecord> storageRecords;
  u64 baseTitleId = tin::util::GetBaseTitleId(this->GetTitleId(i), this->GetContentMetaType(i));
  s32 contentMetaCount = 0;

  LOG_DEBUG("Base title Id: 0x%lx", baseTitleId);

  // TODO: Make custom error with result code field
  // 0x410: The record doesn't already exist
  if (R_FAILED(rc = nsCountApplicationContentMeta(baseTitleId, &contentMetaCount)) && rc != 0x410) {
    THROW_FORMAT("Failed to count application content meta");
  }
  rc = 0;

  LOG_DEBUG("Content meta count: %u\n", contentMetaCount);

  // Obtain any existing app record content meta and append it to our vector
  if (contentMetaCount > 0) {
    storageRecords.resize(contentMetaCount);
    size_t contentStorageBufSize = contentMetaCount * sizeof(ContentStorageRecord);
    auto contentStorageBuf = std::make_unique<ContentStorageRecord[]>(contentMetaCount);
    u32 entriesRead;

    ASSERT_OK(nsListApplicationRecordContentMeta(0, baseTitleId, contentStorageBuf.get(), contentStorageBufSize,
                                                 &entriesRead),
              "Failed to list application record content meta");

    if ((s32)entriesRead != contentMetaCount) {
      THROW_FORMAT("Mismatch between entries read and content meta count");
    }

    memcpy(storageRecords.data(), contentStorageBuf.get(), contentStorageBufSize);
  }

  // Add our new content meta
  ContentStorageRecord storageRecord;
  storageRecord.metaRecord = m_contentMeta[i].GetContentMetaKey();
  storageRecord.storageId = m_destStorageId;
  storageRecords.push_back(storageRecord);

  // Replace the existing application records with our own
  try {
    nsDeleteApplicationRecord(baseTitleId);
  } catch (...) {
  }

  LOG_DEBUG("Pushing application record...\n");
  ASSERT_OK(nsPushApplicationRecord(baseTitleId, 0x3, storageRecords.data(),
                                    storageRecords.size() * sizeof(ContentStorageRecord)),
            "Failed to push application record");
}

// Validate and obtain all data needed for install
void Install::Prepare() {
  tin::data::ByteBuffer cnmtBuf;

  std::vector<std::tuple<nx::ncm::ContentMeta, NcmContentInfo>> tupelList = this->ReadCNMT();

  for (size_t i = 0; i < tupelList.size(); i++) {
    std::tuple<nx::ncm::ContentMeta, NcmContentInfo> cnmtTuple = tupelList[i];

    m_contentMeta.push_back(std::get<0>(cnmtTuple));
    NcmContentInfo cnmtContentRecord = std::get<1>(cnmtTuple);

    nx::ncm::ContentStorage contentStorage(m_destStorageId);

    if (!contentStorage.Has(cnmtContentRecord.content_id)) {
      LOG_DEBUG("Installing CNMT NCA...\n");
      this->InstallNCA(cnmtContentRecord.content_id);
    } else {
      LOG_DEBUG("CNMT NCA already installed. Proceeding...\n");
    }

    // Parse data and create install content meta
    if (m_ignoreReqFirmVersion)
      LOG_DEBUG("WARNING: Required system firmware version is being IGNORED!\n");

    tin::data::ByteBuffer installContentMetaBuf;
    m_contentMeta[i].GetInstallContentMeta(installContentMetaBuf, cnmtContentRecord, m_ignoreReqFirmVersion);

    this->InstallContentMetaRecords(installContentMetaBuf, i);
    this->InstallApplicationRecord(i);
  }
}

void Install::Begin() {
  for (nx::ncm::ContentMeta contentMeta : m_contentMeta) {
    LOG_DEBUG("Installing NCAs...\n");
    for (auto &record : contentMeta.GetContentInfos()) {
      LOG_DEBUG("Installing from %s\n", tin::util::GetNcaIdString(record.content_id).c_str());
      this->InstallNCA(record.content_id);
    }
  }
}

u64 Install::GetTitleId(int i) { return m_contentMeta[i].GetContentMetaKey().id; }

NcmContentMetaType Install::GetContentMetaType(int i) {
  return static_cast<NcmContentMetaType>(m_contentMeta[i].GetContentMetaKey().type);
}
} // namespace tin::install
