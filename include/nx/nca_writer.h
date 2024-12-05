#pragma once
#include "install/nca.hpp"
#include "nx/ncm.hpp"
#include <memory>
#include <switch.h>
#include <vector>

class NcaBodyWriter;

class NcaWriter {
public:
  NcaWriter(const NcmContentId &ncaId, nx::ncm::ContentStorage *contentStorage);
  virtual ~NcaWriter();

  bool close();
  u64 write(const u8 *ptr, u64 sz);

protected:
  void flushHeader();

  NcmContentId m_ncaId;
  nx::ncm::ContentStorage *m_contentStorage;
  std::vector<u8> m_buffer;
  std::unique_ptr<NcaBodyWriter> m_writer;
};
