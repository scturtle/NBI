#pragma once

extern "C" {
#include <switch/services/fs.h>
}

#include <tuple>
#include <vector>

#include "nx/ncm.hpp"

#include "data/byte_buffer.hpp"
#include "install/nsp_or_xci.hpp"
#include "nx/content_meta.hpp"

namespace tin::install {
class Install {
protected:
  const NcmStorageId m_destStorageId;
  bool m_declinedValidation = false;

  const std::shared_ptr<NSPorXCI> m_nsp_or_xci;

  std::vector<nx::ncm::ContentMeta> m_contentMeta;

  void InstallContentMetaRecords(tin::data::ByteBuffer &installContentMetaBuf, int i);
  void InstallApplicationRecord(int i);
  void InstallNCA(const NcmContentId &ncaId);

public:
  Install(NcmStorageId destStorageId, std::shared_ptr<NSPorXCI> nsp_or_xci);
  ~Install();

  void install();
};
} // namespace tin::install
