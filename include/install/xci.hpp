#pragma once

#include <functional>
#include <vector>

#include "install/hfs0.hpp"
#include "install/nsp_or_xci.hpp"

#include "nx/ncm.hpp"
#include <memory>
#include <switch/types.h>

namespace tin::install::xci {
class XCI : public NSPorXCI {
protected:
  u64 m_secureHeaderOffset;
  std::vector<u8> m_secureHeaderBytes;
  FILE *m_file;

public:
  XCI(std::string path);
  ~XCI();

  void RetrieveHeader();
  const HFS0BaseHeader *GetSecureHeader();

  void BufferData(void *buf, off_t offset, size_t size) override;

  const u32 GetFileEntryNum() override;
  const void *GetFileEntry(unsigned int index) override;
  const char *GetFileEntryName(const void *fileEntry) override;
  const u64 GetFileEntrySize(const void *fileEntry) override;
  const u64 GetFileEntryOffset(const void *fileEntry) override;
};
} // namespace tin::install::xci
