#pragma once

#include <memory>
#include <vector>

#include "nx/ncm.hpp"
#include <switch/types.h>

namespace tin::install {
class NSPorXCI {
public:
  void StreamToPlaceholder(nx::ncm::ContentStorage &contentStorage, NcmContentId placeholderId);
  virtual void BufferData(void *buf, off_t offset, size_t size) = 0;

  virtual const u32 GetFileEntryNum() = 0;
  virtual const void *GetFileEntry(unsigned int index) = 0;
  const void *GetFileEntryByName(std::string name);
  const void *GetFileEntryByNcaId(const NcmContentId &ncaId);
  std::vector<const void *> GetFileEntriesByExtension(std::string extension);

  virtual const char *GetFileEntryName(const void *fileEntry) = 0;
  virtual const u64 GetFileEntrySize(const void *fileEntry) = 0;
  virtual const u64 GetFileEntryOffset(const void *fileEntry) = 0;
};
} // namespace tin::install
