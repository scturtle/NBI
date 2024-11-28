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

// try to fix a temp ticket and change it t a permanent one
// https://switchbrew.org/wiki/Ticket#Certificate_chain
static void fixTicket(u8 *tikBuf) {
  u16 ECDSA = 0;
  u16 RSA_2048 = 0;
  u16 RSA_4096 = 0;

  ECDSA = (0x4 + 0x3C + 0x40 + 0x146);
  RSA_2048 = (0x4 + 0x100 + 0x3C + 0x146);
  RSA_4096 = (0x4 + 0x200 + 0x3C + 0x146);

  if (tikBuf[0x0] == 0x5 && (tikBuf[ECDSA] == 0x10 || tikBuf[ECDSA] == 0x30)) {
    tikBuf[ECDSA] = 0x0;
    tikBuf[ECDSA - 1] = 0x10; // fix broken 	Master key revision
  }

  // RSA_2048 SHA256
  else if (tikBuf[0x0] == 0x4 && (tikBuf[RSA_2048] == 0x10 || tikBuf[RSA_2048] == 0x30)) {
    tikBuf[RSA_2048] = 0x0;
    tikBuf[RSA_2048 - 1] = 0x10;
  }

  // RSA_4096 SHA256
  else if (tikBuf[0x0] == 0x3 && (tikBuf[RSA_4096] == 0x10 || tikBuf[RSA_4096] == 0x30)) {
    tikBuf[RSA_4096] = 0x0;
    tikBuf[RSA_4096 - 1] = 0x10;
  }

  // ECDSA SHA1
  else if (tikBuf[0x0] == 0x2 && (tikBuf[ECDSA] == 0x10 || tikBuf[ECDSA] == 0x30)) {
    tikBuf[ECDSA] = 0x0;
    tikBuf[ECDSA - 1] = 0x10;
  }

  // RSA_2048 SHA1
  else if (tikBuf[0x0] == 0x1 && (tikBuf[RSA_2048] == 0x10 || tikBuf[RSA_2048] == 0x30)) {
    tikBuf[RSA_2048] = 0x0;
    tikBuf[RSA_2048 - 1] = 0x10;
  }

  // RSA_4096 SHA1
  else if (tikBuf[0x0] == 0x0 && (tikBuf[RSA_4096] == 0x10 || tikBuf[RSA_4096] == 0x30)) {
    tikBuf[RSA_4096] = 0x0;
    tikBuf[RSA_4096 - 1] = 0x10;
  }
}

namespace tin::install {
Install::Install(NcmStorageId destStorageId, std::shared_ptr<NSPorXCI> nsp_or_xci)
    : m_destStorageId(destStorageId), m_nsp_or_xci(nsp_or_xci) {
  appletSetMediaPlaybackState(true);
}

Install::~Install() { appletSetMediaPlaybackState(false); }

void Install::InstallNCA(const NcmContentId &ncaId) {
  const void *fileEntry = m_nsp_or_xci->GetFileEntryByNcaId(ncaId);
  std::string ncaFileName = m_nsp_or_xci->GetFileEntryName(fileEntry);

  nx::ncm::ContentStorage contentStorage(m_destStorageId);

  // Attempt to delete any leftover placeholders
  try {
    contentStorage.DeletePlaceholder(*(NcmPlaceHolderId *)&ncaId);
  } catch (...) {
  }
  // Attempt to delete leftover ncas
  try {
    contentStorage.Delete(ncaId);
  } catch (...) {
  }

  if (inst::config::validateNCAs && !m_declinedValidation) {
    tin::install::NcaHeader header;
    m_nsp_or_xci->BufferData(&header, m_nsp_or_xci->GetFileEntryOffset(fileEntry), sizeof(tin::install::NcaHeader));

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

  // registering placeholder
  try {
    contentStorage.Register(*(NcmPlaceHolderId *)&ncaId, ncaId);
  } catch (...) {
    // already exist
  }

  try {
    contentStorage.DeletePlaceholder(*(NcmPlaceHolderId *)&ncaId);
  } catch (...) {
  }
}

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
  auto titleId = m_contentMeta[i].GetContentMetaKey().id;
  auto contentMetaType = static_cast<NcmContentMetaType>(m_contentMeta[i].GetContentMetaKey().type);
  u64 baseTitleId = tin::util::GetBaseTitleId(titleId, contentMetaType);
  s32 contentMetaCount = 0;

  // TODO: Make custom error with result code field
  // 0x410: The record doesn't already exist
  if (R_FAILED(rc = nsCountApplicationContentMeta(baseTitleId, &contentMetaCount)) && rc != 0x410) {
    THROW_FORMAT("Failed to count application content meta");
  }
  rc = 0;

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

void Install::install() {
  auto cnmtncas = m_nsp_or_xci->GetFileEntriesByExtension("cnmt.nca");

  for (size_t i = 0; i < cnmtncas.size(); i++) {
    auto fileEntry = cnmtncas[i];
    std::string cnmtNcaName(m_nsp_or_xci->GetFileEntryName(fileEntry));
    size_t cnmtNcaSize = m_nsp_or_xci->GetFileEntrySize(fileEntry);

    // We install the cnmt nca early to read from it later
    NcmContentId cnmtContentId = tin::util::GetNcaIdFromString(cnmtNcaName);
    this->InstallNCA(cnmtContentId);

    nx::ncm::ContentStorage contentStorage(m_destStorageId);
    std::string cnmtNCAFullPath = contentStorage.GetPath(cnmtContentId);

    nx::ncm::ContentMeta contentMeta = tin::util::GetContentMetaFromNCA(cnmtNCAFullPath);
    m_contentMeta.push_back(contentMeta);

    NcmContentInfo cnmtContentInfo;
    cnmtContentInfo.content_id = cnmtContentId;
    ncmU64ToContentInfoSize(cnmtNcaSize & 0xFFFFFFFFFFFF, &cnmtContentInfo);
    cnmtContentInfo.content_type = NcmContentType_Meta;

    // Parse data and create install content meta
    tin::data::ByteBuffer installContentMetaBuf;
    m_contentMeta[i].GetInstallContentMeta(installContentMetaBuf, cnmtContentInfo, inst::config::ignoreReqVers);

    this->InstallContentMetaRecords(installContentMetaBuf, i);
    this->InstallApplicationRecord(i);
  }

  // install tickets and certs
  auto tikFileEntries = m_nsp_or_xci->GetFileEntriesByExtension("tik");
  auto certFileEntries = m_nsp_or_xci->GetFileEntriesByExtension("cert");

  for (size_t i = 0; i < tikFileEntries.size(); i++) {
    u64 tikSize = m_nsp_or_xci->GetFileEntrySize(tikFileEntries[i]);
    auto tikBuf = std::make_unique<u8[]>(tikSize);
    m_nsp_or_xci->BufferData(tikBuf.get(), m_nsp_or_xci->GetFileEntryOffset(tikFileEntries[i]), tikSize);

    u64 certSize = m_nsp_or_xci->GetFileEntrySize(certFileEntries[i]);
    auto certBuf = std::make_unique<u8[]>(certSize);
    m_nsp_or_xci->BufferData(certBuf.get(), m_nsp_or_xci->GetFileEntryOffset(certFileEntries[i]), certSize);

    if (inst::config::fixticket)
      fixTicket(tikBuf.get());

    // Finally, let's actually import the ticket
    ASSERT_OK(esImportTicket(tikBuf.get(), tikSize, certBuf.get(), certSize), "Failed to import ticket");
  }

  // install NCAs
  for (nx::ncm::ContentMeta contentMeta : m_contentMeta) {
    for (auto &record : contentMeta.GetContentInfos()) {
      this->InstallNCA(record.content_id);
    }
  }
}

} // namespace tin::install
