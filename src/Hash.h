#ifndef __CLANG_HASH_HASH
#define __CLANG_HASH_HASH

// #include "SHA1.h"
#include "MurMurHash3.h"

struct Hash : protected MurMurHash3 {

  typedef MurMurHash3 Algorithm;

  struct Digest {
    enum { DIGEST_WORDS = Algorithm::DIGEST_WORDS };
    uint32_t Value[Algorithm::DIGEST_WORDS];
    mutable uint32_t Length; // Number of hashed bytes

    bool operator==(const Digest &Other) const {
      for (unsigned I = 0; I < DIGEST_WORDS; ++I) {
        if (Other.Value[I] != Value[I])
          return false;
      }
      return true;
    }

    bool operator!=(const Digest &Other) const { return !(*this == Other); }

    std::string asString() const {
      std::stringstream ss;
      for (unsigned I = 0; I < DIGEST_WORDS; ++I) {
        ss << std::hex << std::setfill('0') << std::setw(8) << Value[I];
      }
      return ss.str();
    }
  };

  Hash &processBlock(const void *const Start, const void *const End) {
    const uint8_t *Begin = static_cast<const uint8_t *>(Start);
    const uint8_t *Finish = static_cast<const uint8_t *>(End);
    while (Begin != Finish) {
      processByte(*Begin);
      ++Begin;
    }
    return *this;
  }

  Hash &processBytes(const void *const Data, size_t Length) {
    const uint8_t *Block = static_cast<const uint8_t *>(Data);
    processBlock(Block, Block + Length);
    return *this;
  }

  // Digest Operators
  const Digest getDigest() const {
    Hash Copy = *this;
    Digest Ret;
    Ret.Length = Copy.finalize(Ret.Value);
    return Ret;
  }

  // Stream Operators
  Hash &operator<<(uint8_t X) {
    processByte(X);
    return *this;
  }

  Hash &operator<<(uint16_t X) {
    *this << (uint8_t)(X & 0xFF) << (uint8_t)(X >> 8);
    return *this;
  }

  Hash &operator<<(uint32_t X) {
    *this << (uint16_t)(X & 0xFFFF) << (uint16_t)(X >> 16);
    return *this;
  }

  Hash &operator<<(uint64_t X) {
    *this << (uint32_t)(X & 0xFFFFFFFF) << (uint32_t)(X >> 32);
    return *this;
  }

  Hash &operator<<(int8_t X) {
    processByte(X);
    return *this;
  }

  Hash &operator<<(int16_t X) {
    *this << (int8_t)(X & 0xFF) << (int8_t)(X >> 8);
    return *this;
  }

  Hash &operator<<(int32_t X) {
    *this << (int16_t)(X & 0xFFFF) << (int16_t)(X >> 16);
    return *this;
  }

  Hash &operator<<(int64_t X) {
    *this << (int32_t)(X & 0xFFFFFFFF) << (int32_t)(X >> 32);
    return *this;
  }

  Hash &operator<<(const Digest &X) {
    m_byteCount += X.Length;
    X.Length = 0; // Include the hashed bytes only once into the hash statistic.
    return processBytes(X.Value, sizeof(X.Value));
  }

  Hash &operator<<(const Hash &Other) {
    *this << Other.getDigest();;
    return *this;
  }

  Hash &operator<<(const std::string &X) {
    processBytes(X.c_str(), X.length());
    return *this;
  }
};
#endif
