#pragma once
#include <filesystem>
#include <vector>

extern "C" {
#include <switch/services/ncm_types.h>
}

namespace inst {
void installNspFromFile(std::filesystem::path filePath, NcmStorageId storageId);
} // namespace inst
