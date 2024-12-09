#pragma once

#include <switch/types.h>

namespace tin::install {
struct PFS0FileEntry {
  u64 dataOffset;
  u64 fileSize;
  u32 stringTableOffset;
  u32 padding;
} NX_PACKED;

static_assert(sizeof(PFS0FileEntry) == 0x18, "PFS0FileEntry must be 0x18");

struct PFS0BaseHeader {
  u32 magic;
  u32 numFiles;
  u32 stringTableSize;
  u32 reserved;
} NX_PACKED;

static_assert(sizeof(PFS0BaseHeader) == 0x10, "PFS0BaseHeader must be 0x10");

NX_INLINE const PFS0FileEntry *pfs0GetFileEntry(const PFS0BaseHeader *header, u32 i) {
  return (const PFS0FileEntry *)((char *)(header + 1) + i * sizeof(PFS0FileEntry));
}

NX_INLINE const char *pfs0GetFileName(const PFS0BaseHeader *header, const PFS0FileEntry *entry) {
  char *stringTable = (char *)(header + 1) + header->numFiles * sizeof(PFS0FileEntry);
  return stringTable + entry->stringTableOffset;
}
} // namespace tin::install
