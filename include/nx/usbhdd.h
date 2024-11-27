#pragma once

namespace nx::hdd {
const char *rootPath(u32 index = 0);
u32 count();

bool init();
bool exit();
} // namespace nx::hdd
