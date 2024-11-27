#include "util/title_util.hpp"

#include "util/error.hpp"
#include <machine/endian.h>

namespace tin::util {
std::string GetNcaIdString(const NcmContentId &ncaId) {
  char ncaIdStr[FS_MAX_PATH] = {0};
  u64 ncaIdLower = __bswap64(*(u64 *)ncaId.c);
  u64 ncaIdUpper = __bswap64(*(u64 *)(ncaId.c + 0x8));
  snprintf(ncaIdStr, FS_MAX_PATH, "%016lx%016lx", ncaIdLower, ncaIdUpper);
  return std::string(ncaIdStr);
}

NcmContentId GetNcaIdFromString(std::string ncaIdStr) {
  NcmContentId ncaId = {0};
  char lowerU64[17] = {0};
  char upperU64[17] = {0};
  memcpy(lowerU64, ncaIdStr.c_str(), 16);
  memcpy(upperU64, ncaIdStr.c_str() + 16, 16);

  *(u64 *)ncaId.c = __bswap64(strtoul(lowerU64, NULL, 16));
  *(u64 *)(ncaId.c + 8) = __bswap64(strtoul(upperU64, NULL, 16));

  return ncaId;
}

u64 GetBaseTitleId(u64 titleId, NcmContentMetaType contentMetaType) {
  switch (contentMetaType) {
  case NcmContentMetaType_Patch:
    return titleId ^ 0x800;

  case NcmContentMetaType_AddOnContent:
    return (titleId ^ 0x1000) & ~0xFFF;

  default:
    return titleId;
  }
}

} // namespace tin::util
