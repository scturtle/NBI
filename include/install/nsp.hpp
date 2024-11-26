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

  NSP();

public:
  virtual void RetrieveHeader() override;
  virtual const PFS0BaseHeader *GetBaseHeader();
  virtual u64 GetDataOffset() override;

  virtual const PFS0FileEntry *GetFileEntry(unsigned int index);
  virtual const PFS0FileEntry *GetFileEntryByName(std::string name);
  virtual const void *GetFileEntryByNcaId(const NcmContentId &ncaId) override;
  virtual std::vector<const void *> GetFileEntriesByExtension(std::string extension) override;

  virtual const char *GetFileEntryName(const void *fileEntry) override;
  virtual const u64 GetFileEntrySize(const void *fileEntry) override { return ((PFS0FileEntry *)fileEntry)->fileSize; }
  virtual const u64 GetFileEntryOffset(const void *fileEntry) override {
    return ((PFS0FileEntry *)fileEntry)->dataOffset;
  }
};
} // namespace tin::install::nsp
