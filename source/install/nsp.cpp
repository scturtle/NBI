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

// https://switchbrew.org/wiki/NCA

#include "install/nsp.hpp"
#include "util/error.hpp"
#include "util/title_util.hpp"

namespace tin::install::nsp {

NSP::NSP(std::string path) { m_file = fopen((path).c_str(), "rb"); }

NSP::~NSP() { fclose(m_file); }

void NSP::BufferData(void *buf, off_t offset, size_t size) {
  fseeko(m_file, offset, SEEK_SET);
  fread(buf, 1, size, m_file);
}

// TODO: Do verification: PFS0 magic, sizes not zero
void NSP::RetrieveHeader() {
  LOG_DEBUG("Retrieving remote NSP header...\n");

  // Retrieve the base header
  m_headerBytes.resize(sizeof(PFS0BaseHeader), 0);
  this->BufferData(m_headerBytes.data(), 0x0, sizeof(PFS0BaseHeader));

  // Retrieve the full header
  size_t remainingHeaderSize =
      this->GetBaseHeader()->numFiles * sizeof(PFS0FileEntry) + this->GetBaseHeader()->stringTableSize;
  m_headerBytes.resize(sizeof(PFS0BaseHeader) + remainingHeaderSize, 0);
  this->BufferData(m_headerBytes.data() + sizeof(PFS0BaseHeader), sizeof(PFS0BaseHeader), remainingHeaderSize);
}

const u32 NSP::GetFileEntryNum() { return this->GetBaseHeader()->numFiles; }

const void *NSP::GetFileEntry(unsigned int index) {
  size_t fileEntryOffset = sizeof(PFS0BaseHeader) + index * sizeof(PFS0FileEntry);
  return reinterpret_cast<PFS0FileEntry *>(m_headerBytes.data() + fileEntryOffset);
}

const u64 NSP::GetFileEntrySize(const void *fileEntry) { return ((PFS0FileEntry *)fileEntry)->fileSize; }

const u64 NSP::GetFileEntryOffset(const void *fileEntry) {
  return GetDataOffset() + ((PFS0FileEntry *)fileEntry)->dataOffset;
}

const char *NSP::GetFileEntryName(const void *fileEntry) {
  u64 stringTableStart = sizeof(PFS0BaseHeader) + this->GetFileEntryNum() * sizeof(PFS0FileEntry);
  return reinterpret_cast<const char *>(m_headerBytes.data() + stringTableStart +
                                        ((PFS0FileEntry *)fileEntry)->stringTableOffset);
}

const PFS0BaseHeader *NSP::GetBaseHeader() { return reinterpret_cast<PFS0BaseHeader *>(m_headerBytes.data()); }

u64 NSP::GetDataOffset() { return m_headerBytes.size(); }
} // namespace tin::install::nsp
