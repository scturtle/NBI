#pragma once

#include "nx/content_meta.hpp"
#include "nx/ipc/tin_ipc.h"
#include <string>

#include <switch/types.h>

namespace tin::util {
std::string GetNcaIdString(const NcmContentId &ncaId);
NcmContentId GetNcaIdFromString(std::string ncaIdStr);
u64 GetBaseTitleId(u64 titleId, NcmContentMetaType contentMetaType);
} // namespace tin::util
