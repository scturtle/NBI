#pragma once
#include "install/nca.hpp"
#include "nx/ncm.hpp"
#include <memory>
#include <switch.h>
#include <vector>

class NcaBodyWriter;

class NcaWriter {
public:
  NcaWriter(const NcmContentId &ncaId, std::shared_ptr<nx::ncm::ContentStorage> &contentStorage);
  virtual ~NcaWriter();

  bool isOpen() const;
  bool close();
  u64 write(const u8 *ptr, u64 sz);
  void flushHeader();

protected:
  NcmContentId m_ncaId;
  std::shared_ptr<nx::ncm::ContentStorage> m_contentStorage;
  std::vector<u8> m_buffer;
  std::shared_ptr<NcaBodyWriter> m_writer;
};
