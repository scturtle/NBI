#include "install/nsp.hpp"
#include "util/error.hpp"
#include "util/title_util.hpp"

namespace tin::install::nsp {

NSP::NSP(std::string path) {
  m_file = fopen((path).c_str(), "rb");
  RetrieveHeader();
}

NSP::~NSP() { fclose(m_file); }

void NSP::BufferData(void *buf, off_t offset, size_t size) {
  fseeko(m_file, offset, SEEK_SET);
  fread(buf, 1, size, m_file);
}

void NSP::RetrieveHeader() {
  // Retrieve the base header
  m_headerBytes.resize(sizeof(PFS0BaseHeader), 0);
  this->BufferData(m_headerBytes.data(), 0x0, sizeof(PFS0BaseHeader));

  // Retrieve the full header
  size_t remainingHeaderSize = GetFileEntryNum() * sizeof(PFS0FileEntry) + this->GetBaseHeader()->stringTableSize;
  m_headerBytes.resize(sizeof(PFS0BaseHeader) + remainingHeaderSize, 0);
  this->BufferData(m_headerBytes.data() + sizeof(PFS0BaseHeader), sizeof(PFS0BaseHeader), remainingHeaderSize);
}

const PFS0BaseHeader *NSP::GetBaseHeader() { return reinterpret_cast<PFS0BaseHeader *>(m_headerBytes.data()); }

const u32 NSP::GetFileEntryNum() { return this->GetBaseHeader()->numFiles; }

const void *NSP::GetFileEntry(unsigned int index) { return pfs0GetFileEntry(this->GetBaseHeader(), index); }

const char *NSP::GetFileEntryName(const void *fileEntry) {
  return pfs0GetFileName(this->GetBaseHeader(), (PFS0FileEntry *)fileEntry);
}

const u64 NSP::GetFileEntrySize(const void *fileEntry) { return ((PFS0FileEntry *)fileEntry)->fileSize; }

const u64 NSP::GetFileEntryOffset(const void *fileEntry) {
  return m_headerBytes.size() + ((PFS0FileEntry *)fileEntry)->dataOffset;
}
} // namespace tin::install::nsp
