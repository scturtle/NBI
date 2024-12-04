#include "nx/nca_writer.h"
#include "install/nca.hpp"
#include "util/crypto.hpp"
#include "util/error.hpp"
#include "util/util.hpp"
#include <string.h>
#include <switch.h>
#include <zstd.h>

static void append(std::vector<u8> &buffer, const u8 *ptr, u64 sz) {
  u64 offset = buffer.size();
  buffer.resize(offset + sz);
  memcpy(buffer.data() + offset, ptr, sz);
}

class NcaBodyWriter {
public:
  NcaBodyWriter(const NcmContentId &ncaId, u64 offset, nx::ncm::ContentStorage *contentStorage)
      : m_contentStorage(contentStorage), m_ncaId(ncaId), m_offset(offset) {
    sha256ContextCreate(&m_ctx);
  }

  virtual u64 write(const u8 *ptr, u64 sz) {
    m_contentStorage->WritePlaceholder(*(NcmPlaceHolderId *)&m_ncaId, m_offset, (void *)ptr, sz);
    this->update_hash(ptr, sz);
    m_offset += sz;
    return sz;
  }

  virtual void close() {}

  void update_hash(const void *src, size_t size) { sha256ContextUpdate(&m_ctx, src, size); }

  bool verify_hash(const NcmContentId &ncaId) {
    u8 nca_hash[SHA256_HASH_SIZE] = {0};
    sha256ContextGetHash(&m_ctx, nca_hash);
    return memcmp(nca_hash, &ncaId.c, sizeof(ncaId.c)) == 0;
  }

protected:
  nx::ncm::ContentStorage *m_contentStorage;
  NcmContentId m_ncaId;
  u64 m_offset;
  Sha256Context m_ctx;
};

class NczHeader {
public:
  static const u64 MAGIC = 0x4E544345535A434E; // NTCESZCN
  static const u64 BLOCK = 0x4B434F4C425A434E; // NCZBLOCK at 0x40D0

  class Section {
  public:
    u64 offset;
    u64 size;
    u8 cryptoType;
    u8 padding1[7];
    u64 padding2;
    u8 cryptoKey[0x10];
    u8 cryptoCounter[0x10];
  } NX_PACKED;

  class SectionContext : public Section {
  public:
    Crypto::Aes128Ctr crypto;

    SectionContext(const Section &s)
        : Section(s), crypto(s.cryptoKey, Crypto::AesCtr(Crypto::swapEndian(((u64 *)&s.cryptoCounter)[0]))) {}

    void encrypt(void *p, u64 sz, u64 offset) {
      if (this->cryptoType == 3 || this->cryptoType == 4) {
        crypto.seek(offset);
        crypto.encrypt(p, p, sz);
      }
    }
  };

  const u64 size() const { return sizeof(m_magic) + sizeof(m_sectionCount) + sizeof(Section) * m_sectionCount; }

  const Section &section(u64 i) const { return m_sections[i]; }

  const u64 sectionCount() const { return m_sectionCount; }

protected:
  u64 m_magic;
  u64 m_sectionCount;
  Section m_sections[1];
} NX_PACKED;

struct BlockHeader {
  u64 magic;
  u8 version;
  u8 type;
  u8 unused;
  u8 blockSizeExponent;
  u32 numberOfBlocks;
  u64 decompressedSize;
};

struct BlockInfo {
  void init(const u8 *buffer) {
    header = *(BlockHeader *)buffer;
    assert(header.blockSizeExponent >= 14 && header.blockSizeExponent < 32);
    blockSize = size_t(1) << header.blockSizeExponent;
    for (u32 i = 0; i < header.numberOfBlocks; i++)
      compressedBlockSizeList.push_back(*(u32 *)(buffer + sizeof(BlockHeader) + i * sizeof(u32)));
  }

  u32 getCurBlockSize() { return compressedBlockSizeList[curBlockId]; }

  size_t decompressBlock(const std::vector<u8> &buffer, std::vector<u8> &dest) {
    auto curBlockSize = getCurBlockSize();
    assert(buffer.size() >= curBlockSize);
    size_t outSize;
    if (curBlockSize < blockSize) {
      outSize = ZSTD_getFrameContentSize(buffer.data(), curBlockSize);
      if (outSize == ZSTD_CONTENTSIZE_UNKNOWN || outSize == ZSTD_CONTENTSIZE_ERROR) {
        THROW_FORMAT("ZSTD_getFrameContentSize error");
        return 0;
      }
      dest.resize(outSize);
      auto ret = ZSTD_decompress(dest.data(), outSize, buffer.data(), curBlockSize);
      if (ZSTD_isError(ret)) {
        THROW_FORMAT("ZSTD_decompress error");
        return 0;
      }
    } else {
      outSize = curBlockSize;
      dest.resize(outSize);
      memcpy(dest.data(), buffer.data(), curBlockSize);
    }
    curBlockId += 1;
    return outSize;
  }

  BlockHeader header;
  size_t blockSize = 0;
  size_t curBlockId = 0;
  std::vector<u32> compressedBlockSizeList;
};

class NczBodyWriter : public NcaBodyWriter {
public:
  NczBodyWriter(const NcmContentId &ncaId, u64 offset, nx::ncm::ContentStorage *contentStorage)
      : NcaBodyWriter(ncaId, offset, contentStorage) {
    buffOut = malloc(buffOutSize);
    dctx = ZSTD_createDCtx();
  }

  ~NczBodyWriter() {
    close();
    free(buffOut);
    ZSTD_freeDCtx(dctx);
  }

  void close() override {
    if (!m_isBlockCompression) {
      if (m_buffer.size()) {
        processChunk(m_buffer.data(), m_buffer.size());
        m_buffer.resize(0);
      }
      encrypt(m_deflateBuffer.data(), m_deflateBuffer.size(), m_offset);
      flush();
    } else {
      if (m_buffer.size())
        throw std::runtime_error("block decompress error");
    }
  }

  void flush() {
    if (m_deflateBuffer.size()) {
      m_contentStorage->WritePlaceholder(*(NcmPlaceHolderId *)&m_ncaId, m_offset, m_deflateBuffer.data(),
                                         m_deflateBuffer.size());
      this->update_hash(m_deflateBuffer.data(), m_deflateBuffer.size());
      m_offset += m_deflateBuffer.size();
      m_deflateBuffer.resize(0);
    }
  }

  NczHeader::SectionContext &section(u64 offset) {
    for (u64 i = 0; i < sections.size(); i++)
      if (offset >= sections[i].offset && offset < sections[i].offset + sections[i].size)
        return sections[i];
    return sections[0];
  }

  void encrypt(const void *ptr, u64 sz, u64 offset) {
    const u8 *start = (u8 *)ptr;
    const u8 *end = start + sz;
    while (start < end) {
      auto &s = section(offset);
      u64 sectionEnd = s.offset + s.size;
      u64 chunk = offset + sz > sectionEnd ? sectionEnd - offset : sz;
      s.encrypt((void *)start, chunk, offset);
      offset += chunk;
      start += chunk;
      sz -= chunk;
    }
  }

  void processChunk(const u8 *ptr, u64 sz) {
    while (sz > 0) {
      const size_t readChunkSz = std::min(sz, (u64)buffInSize);
      ZSTD_inBuffer input = {ptr, readChunkSz, 0};
      while (input.pos < input.size) {
        ZSTD_outBuffer output = {buffOut, buffOutSize, 0};
        size_t const ret = ZSTD_decompressStream(dctx, &output, &input);
        if (ZSTD_isError(ret)) {
          throw std::runtime_error("ZSTD_decompressStream error");
          return;
        }
        size_t len = output.pos;
        u8 *p = (u8 *)buffOut;
        while (len) {
          const size_t writeChunkSz = std::min(0x1000000 - m_deflateBuffer.size(), len);
          append(m_deflateBuffer, p, writeChunkSz);
          if (m_deflateBuffer.size() >= 0x1000000) {
            encrypt(m_deflateBuffer.data(), m_deflateBuffer.size(), m_offset);
            flush();
          }
          p += writeChunkSz;
          len -= writeChunkSz;
        }
      }
      sz -= readChunkSz;
      ptr += readChunkSz;
    }
  }

  u64 write(const u8 *ptr, u64 sz) override {
    if (!m_sectionsInitialized) {
      if (!m_buffer.size()) {
        append(m_buffer, ptr, sizeof(u64) * 2);
        ptr += sizeof(u64) * 2;
        sz -= sizeof(u64) * 2;
      }
      auto header = (NczHeader *)m_buffer.data();
      u64 chunk = std::min(sz, header->size() - m_buffer.size());
      append(m_buffer, ptr, chunk);
      ptr += chunk;
      sz -= chunk;
      header = (NczHeader *)m_buffer.data();
      if (m_buffer.size() == header->size()) {
        for (u64 i = 0; i < header->sectionCount(); i++)
          sections.emplace_back(header->section(i));
        m_sectionsInitialized = true;
        m_buffer.resize(0);
      }
    }

    if (!m_blockInitialized) {
      u64 chunk = std::min((size_t)sz, sizeof(BlockHeader) - m_buffer.size());
      append(m_buffer, ptr, chunk);
      ptr += chunk;
      sz -= chunk;
      if (m_buffer.size() == sizeof(BlockHeader)) {
        m_blockInitialized = true;
        auto block = (BlockHeader *)m_buffer.data();
        if (block->magic != NczHeader::BLOCK) {
          m_isBlockCompression = false;
        } else {
          m_isBlockCompression = true;
          auto listSize = block->numberOfBlocks * sizeof(u32);
          u64 chunk = std::min((size_t)sz, sizeof(BlockHeader) + listSize - m_buffer.size());
          append(m_buffer, ptr, chunk);
          ptr += chunk;
          sz -= chunk;
          if (m_buffer.size() == sizeof(BlockHeader) + listSize) {
            blockInfo.init(m_buffer.data());
            m_buffer.resize(0);
          }
        }
      }
    }

    if (m_isBlockCompression) {
      while (sz) {
        size_t curBlockSize = blockInfo.getCurBlockSize();
        auto chunk = m_buffer.size() < curBlockSize ? std::min((size_t)sz, curBlockSize - m_buffer.size()) : 0;
        append(m_buffer, ptr, chunk);
        sz -= chunk;
        ptr += chunk;
        if (m_buffer.size() >= curBlockSize) {
          blockInfo.decompressBlock(m_buffer, m_deflateBuffer);
          encrypt(m_deflateBuffer.data(), m_deflateBuffer.size(), m_offset);
          flush();
          m_buffer.erase(m_buffer.begin(), m_buffer.begin() + curBlockSize);
        }
      }
    } else {
      while (sz) {
        if (m_buffer.size() + sz >= 0x1000000) {
          u64 chunk = 0x1000000 - m_buffer.size();
          append(m_buffer, ptr, chunk);
          processChunk(m_buffer.data(), m_buffer.size());
          m_buffer.resize(0);
          sz -= chunk;
          ptr += chunk;
        } else {
          append(m_buffer, ptr, sz);
          sz = 0;
        }
      }
    }

    return sz;
  }

  size_t const buffInSize = ZSTD_DStreamInSize();
  size_t const buffOutSize = ZSTD_DStreamOutSize();

  void *buffOut = NULL;
  ZSTD_DCtx *dctx = NULL;

  std::vector<u8> m_buffer;
  std::vector<u8> m_deflateBuffer;

  bool m_sectionsInitialized = false;
  bool m_blockInitialized = false;
  bool m_isBlockCompression = false;

  std::vector<NczHeader::SectionContext> sections;
  BlockInfo blockInfo;
};

NcaWriter::NcaWriter(const NcmContentId &ncaId, nx::ncm::ContentStorage *contentStorage)
    : m_ncaId(ncaId), m_contentStorage(contentStorage), m_writer(NULL) {}

NcaWriter::~NcaWriter() { close(); }

bool NcaWriter::close() {
  if (m_writer) {
    m_writer->close();
    if (not m_writer->verify_hash(m_ncaId))
      inst::util::msg("error", "nca verification failed");
    m_writer = NULL;
  } else if (m_buffer.size()) {
    if (isOpen())
      flushHeader();
    m_buffer.resize(0);
  }
  m_contentStorage = NULL;
  return true;
}

bool NcaWriter::isOpen() const { return (bool)m_contentStorage; }

u64 NcaWriter::write(const u8 *ptr, u64 sz) {
  if (m_buffer.size() < NCA_HEADER_SIZE) {
    u64 remainder = std::min((size_t)sz, NCA_HEADER_SIZE - m_buffer.size());
    append(m_buffer, ptr, remainder);
    ptr += remainder;
    sz -= remainder;
    if (m_buffer.size() == NCA_HEADER_SIZE) {
      flushHeader();
    }
  }

  if (sz) {
    if (!m_writer) {
      if (sz >= sizeof(NczHeader::MAGIC)) {
        if (*(u64 *)ptr == NczHeader::MAGIC) {
          m_writer = std::make_shared<NczBodyWriter>(m_ncaId, m_buffer.size(), m_contentStorage);
        } else {
          m_writer = std::make_shared<NcaBodyWriter>(m_ncaId, m_buffer.size(), m_contentStorage);
        }
        m_writer->update_hash(m_buffer.data(), m_buffer.size());
      } else {
        THROW_FORMAT("not enough data to read ncz header");
      }
    }

    if (m_writer) {
      m_writer->write(ptr, sz);
    } else {
      THROW_FORMAT("null writer");
    }
  }

  return sz;
}

void NcaWriter::flushHeader() {
  tin::install::NcaHeader header;
  memcpy(&header, m_buffer.data(), sizeof(header));
  Crypto::AesXtr decryptor(Crypto::Keys().headerKey, false);
  Crypto::AesXtr encryptor(Crypto::Keys().headerKey, true);
  decryptor.decrypt(&header, &header, sizeof(header), 0, 0x200);

  if (header.magic == MAGIC_NCA3) {
    if (isOpen()) {
      m_contentStorage->CreatePlaceholder(m_ncaId, *(NcmPlaceHolderId *)&m_ncaId, header.nca_size);
    }
  } else {
    THROW_FORMAT("Invalid NCA magic");
  }

  if (header.distribution == 1) {
    header.distribution = 0;
  }
  encryptor.encrypt(m_buffer.data(), &header, sizeof(header), 0, 0x200);

  if (isOpen()) {
    m_contentStorage->WritePlaceholder(*(NcmPlaceHolderId *)&m_ncaId, 0, m_buffer.data(), m_buffer.size());
  }
}
