#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "install/nsp_or_xci.hpp"
#include "install/pfs0.hpp"

#include "nx/ncm.hpp"
#include <switch/types.h>

namespace tin::install::nsp {
class NSP : public NSPorXCI {
protected:
  std::vector<u8> m_headerBytes;
  FILE *m_file;

public:
  NSP(std::string path);
  ~NSP();

  void RetrieveHeader() override;
  const PFS0BaseHeader *GetBaseHeader();
  u64 GetDataOffset();

  void BufferData(void *buf, off_t offset, size_t size) override;

  const u32 GetFileEntryNum() override;
  const void *GetFileEntry(unsigned int index) override;
  const char *GetFileEntryName(const void *fileEntry) override;
  const u64 GetFileEntrySize(const void *fileEntry) override;
  const u64 GetFileEntryOffset(const void *fileEntry) override;
};
} // namespace tin::install::nsp
