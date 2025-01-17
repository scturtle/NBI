#pragma once

#include <cstring>
#include <vector>

#include <switch/types.h>

namespace tin::data {
class ByteBuffer {
private:
  std::vector<u8> m_buffer;

public:
  ByteBuffer(size_t reserveSize = 0) { m_buffer.resize(reserveSize); }

  size_t GetSize() { return m_buffer.size(); }
  u8 *GetData() { return m_buffer.data(); }
  void Resize(size_t size) { m_buffer.resize(size, 0); }

  template <typename T> T Read(u64 offset) {
    if (offset + sizeof(T) <= m_buffer.size())
      return *((T *)&m_buffer.data()[offset]);
    T def;
    memset(&def, 0, sizeof(T));
    return def;
  }

  template <typename T> void Write(T data, u64 offset) {
    size_t requiredSize = offset + sizeof(T);
    if (requiredSize > m_buffer.size())
      m_buffer.resize(requiredSize, 0);
    memcpy(m_buffer.data() + offset, &data, sizeof(T));
  }

  template <typename T> void Append(T data) { this->Write(data, this->GetSize()); }
};
} // namespace tin::data
