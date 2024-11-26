#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "nx/ncm.hpp"
#include <switch/types.h>

namespace tin::install {
class NSPorXCI {
public:
  virtual void StreamToPlaceholder(std::shared_ptr<nx::ncm::ContentStorage> &contentStorage,
                                   NcmContentId placeholderId) = 0;
  virtual void BufferData(void *buf, off_t offset, size_t size) = 0;

  virtual void RetrieveHeader() = 0;
  virtual u64 GetDataOffset() = 0;

  virtual const void *GetFileEntryByNcaId(const NcmContentId &ncaId) = 0;
  virtual std::vector<const void *> GetFileEntriesByExtension(std::string extension) = 0;

  virtual const char *GetFileEntryName(const void *fileEntry) = 0;
  virtual const u64 GetFileEntrySize(const void *fileEntry) = 0;
  virtual const u64 GetFileEntryOffset(const void *fileEntry) = 0;
};
} // namespace tin::install
