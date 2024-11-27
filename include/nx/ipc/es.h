#pragma once

#include <switch/services/ncm.h>

Result esInitialize();
void esExit();

Result esImportTicket(void const *tikBuf, size_t tikSize, void const *certBuf, size_t certSize); // 1
