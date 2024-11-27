#pragma once
#include <filesystem>
#include <switch/services/ncm_types.h>
#include <vector>

namespace inst {
void installNspFromFile(std::filesystem::path filePath, NcmStorageId storageId);
} // namespace inst
