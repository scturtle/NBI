#include "install/xci.hpp"
#include "error.hpp"
#include "util/title_util.hpp"

namespace tin::install::xci {

XCI::XCI(std::string path) {
  m_file = fopen((path).c_str(), "rb");
  RetrieveHeader();
}

XCI::~XCI() { fclose(m_file); }

void XCI::BufferData(void *buf, off_t offset, size_t size) {
  fseeko(m_file, offset, SEEK_SET);
  fread(buf, 1, size, m_file);
}

void XCI::RetrieveHeader() {
  // Retrieve hfs0 offset (the PartitionFsHeaderAddress at 0x130 in CardHeader)
  u64 hfs0Offset = 0xf000;

  // Retrieve main hfs0 header
  std::vector<u8> m_headerBytes;
  m_headerBytes.resize(sizeof(HFS0BaseHeader), 0);
  this->BufferData(m_headerBytes.data(), hfs0Offset, sizeof(HFS0BaseHeader));

  // Retrieve full header
  HFS0BaseHeader *header = reinterpret_cast<HFS0BaseHeader *>(m_headerBytes.data());
  if (header->magic != MAGIC_HFS0)
    THROW_FORMAT("hfs0 magic doesn't match\n");

  size_t remainingHeaderSize = header->numFiles * sizeof(HFS0FileEntry) + header->stringTableSize;
  m_headerBytes.resize(sizeof(HFS0BaseHeader) + remainingHeaderSize, 0);
  this->BufferData(m_headerBytes.data() + sizeof(HFS0BaseHeader), hfs0Offset + sizeof(HFS0BaseHeader),
                   remainingHeaderSize);

  // find secure partition
  header = reinterpret_cast<HFS0BaseHeader *>(m_headerBytes.data());
  for (unsigned int i = 0; i < header->numFiles; i++) {
    const HFS0FileEntry *entry = hfs0GetFileEntry(header, i);
    std::string entryName(hfs0GetFileName(header, entry));

    if (entryName != "secure")
      continue;

    m_secureHeaderOffset = hfs0Offset + sizeof(HFS0BaseHeader) + remainingHeaderSize + entry->dataOffset;
    m_secureHeaderBytes.resize(sizeof(HFS0BaseHeader), 0);
    this->BufferData(m_secureHeaderBytes.data(), m_secureHeaderOffset, sizeof(HFS0BaseHeader));

    if (this->GetSecureHeader()->magic != MAGIC_HFS0)
      THROW_FORMAT("hfs0 magic doesn't match\n");

    // Retrieve full header
    remainingHeaderSize = this->GetFileEntryNum() * sizeof(HFS0FileEntry) + this->GetSecureHeader()->stringTableSize;
    m_secureHeaderBytes.resize(sizeof(HFS0BaseHeader) + remainingHeaderSize, 0);
    this->BufferData(m_secureHeaderBytes.data() + sizeof(HFS0BaseHeader), m_secureHeaderOffset + sizeof(HFS0BaseHeader),
                     remainingHeaderSize);

    return;
  }
  THROW_FORMAT("couldn't optain secure hfs0 header\n");
}

const HFS0BaseHeader *XCI::GetSecureHeader() { return reinterpret_cast<HFS0BaseHeader *>(m_secureHeaderBytes.data()); }

const u32 XCI::GetFileEntryNum() { return this->GetSecureHeader()->numFiles; }

const void *XCI::GetFileEntry(unsigned int index) { return hfs0GetFileEntry(this->GetSecureHeader(), index); }

const char *XCI::GetFileEntryName(const void *fileEntry) {
  return hfs0GetFileName(this->GetSecureHeader(), (HFS0FileEntry *)fileEntry);
}

const u64 XCI::GetFileEntrySize(const void *fileEntry) { return ((HFS0FileEntry *)fileEntry)->fileSize; }

const u64 XCI::GetFileEntryOffset(const void *fileEntry) {
  return m_secureHeaderOffset + m_secureHeaderBytes.size() + ((HFS0FileEntry *)fileEntry)->dataOffset;
}
} // namespace tin::install::xci
