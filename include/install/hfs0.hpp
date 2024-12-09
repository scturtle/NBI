#pragma once

#include <switch/types.h>

#define MAGIC_HFS0 0x30534648

namespace tin::install {
struct HFS0FileEntry {
  u64 dataOffset;
  u64 fileSize;
  u32 stringTableOffset;
  u32 hashedSize;
  u64 padding;
  unsigned char hash[0x20];
} NX_PACKED;

static_assert(sizeof(HFS0FileEntry) == 0x40, "HFS0FileEntry must be 0x18");

struct HFS0BaseHeader {
  u32 magic;
  u32 numFiles;
  u32 stringTableSize;
  u32 reserved;
} NX_PACKED;

static_assert(sizeof(HFS0BaseHeader) == 0x10, "HFS0BaseHeader must be 0x10");

NX_INLINE const HFS0FileEntry *hfs0GetFileEntry(const HFS0BaseHeader *header, u32 i) {
  return (const HFS0FileEntry *)((char *)(header + 1) + i * sizeof(HFS0FileEntry));
}

NX_INLINE const char *hfs0GetFileName(const HFS0BaseHeader *header, const HFS0FileEntry *entry) {
  char *stringTable = (char *)(header + 1) + header->numFiles * sizeof(HFS0FileEntry);
  return stringTable + entry->stringTableOffset;
}
} // namespace tin::install
