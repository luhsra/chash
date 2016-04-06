#ifndef __CLANG_HASH_MURMUR3
#define __CLANG_HASH_MURMUR3

#include <sstream>
#include <iostream>
#include <iomanip>

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.

#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else // defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------

void MurmurHash3_x64_128(const void *key, int len, uint32_t seed, void *out);

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER)

#define FORCE_INLINE __forceinline

#include <stdlib.h>

#define ROTL32(x, y) _rotl(x, y)
#define ROTL64(x, y) _rotl64(x, y)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else // defined(_MSC_VER)

#define FORCE_INLINE inline __attribute__((always_inline))

static inline uint32_t rotl32(uint32_t x, int8_t r) {
  return (x << r) | (x >> (32 - r));
}

static inline uint64_t rotl64(uint64_t x, int8_t r) {
  return (x << r) | (x >> (64 - r));
}

#define ROTL32(x, y) rotl32(x, y)
#define ROTL64(x, y) rotl64(x, y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

static FORCE_INLINE uint32_t getblock32(const uint32_t *p, int i) {
  return p[i];
}

static FORCE_INLINE uint64_t getblock64(const uint64_t *p, int i) {
  return p[i];
}

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

static FORCE_INLINE uint32_t fmix32(uint32_t h) {
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

//----------

FORCE_INLINE uint64_t fmix64(uint64_t k) {
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xff51afd7ed558ccd);
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
  k ^= k >> 33;

  return k;
}

//-----------------------------------------------------------------------------

// We use the MurMurHash3 x64 - 128 Bit Variant of the MurMur Hash
class MurMurHash3 {
public:
  enum { DIGEST_WORDS = 4, BLOCK_LENGTH = 16 };

  MurMurHash3() { reset(); }
  virtual ~MurMurHash3() {}
  MurMurHash3(const MurMurHash3 &s) { *this = s; }

  const MurMurHash3 &operator=(const MurMurHash3 &s) {
    h1 = s.h1;
    h2 = s.h2;
    memcpy(m_block, s.m_block, BLOCK_LENGTH);
    m_blockByteIndex = s.m_blockByteIndex;
    m_byteCount = s.m_byteCount;
    return *this;
  }

  MurMurHash3 &reset() {
    h1 = 0x6745230198BADCFE;
    h2 = 0xEFCDAB8910325476;
    m_blockByteIndex = 0;
    m_byteCount = 0;
    return *this;
  }

  MurMurHash3 &processByte(uint8_t octet) {
    this->m_block[this->m_blockByteIndex++] = octet;
    ++this->m_byteCount;
    if (m_blockByteIndex == BLOCK_LENGTH) {
      this->m_blockByteIndex = 0;
      processBlock();
    }
    return *this;
  }

  uint32_t finalize(uint32_t *digest) {
    //----------
    // tail
    uint64_t k1 = 0;
    uint64_t k2 = 0;
    // We use m_byteCount NOT here. On Purpose.
    switch (m_blockByteIndex & 15) {
    case 15:
      k2 ^= ((uint64_t)m_block[14]) << 48;
    case 14:
      k2 ^= ((uint64_t)m_block[13]) << 40;
    case 13:
      k2 ^= ((uint64_t)m_block[12]) << 32;
    case 12:
      k2 ^= ((uint64_t)m_block[11]) << 24;
    case 11:
      k2 ^= ((uint64_t)m_block[10]) << 16;
    case 10:
      k2 ^= ((uint64_t)m_block[9]) << 8;
    case 9:
      k2 ^= ((uint64_t)m_block[8]) << 0;
      k2 *= c2;
      k2 = ROTL64(k2, 33);
      k2 *= c1;
      h2 ^= k2;
    case 8:
      k1 ^= ((uint64_t)m_block[7]) << 56;
    case 7:
      k1 ^= ((uint64_t)m_block[6]) << 48;
    case 6:
      k1 ^= ((uint64_t)m_block[5]) << 40;
    case 5:
      k1 ^= ((uint64_t)m_block[4]) << 32;
    case 4:
      k1 ^= ((uint64_t)m_block[3]) << 24;
    case 3:
      k1 ^= ((uint64_t)m_block[2]) << 16;
    case 2:
      k1 ^= ((uint64_t)m_block[1]) << 8;
    case 1:
      k1 ^= ((uint64_t)m_block[0]) << 0;
      k1 *= c1;
      k1 = ROTL64(k1, 31);
      k1 *= c2;
      h1 ^= k1;
    };

    //----------
    // finalization
    h1 ^= m_byteCount;
    h2 ^= m_byteCount;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;

    ((uint64_t *)digest)[0] = h1;
    ((uint64_t *)digest)[1] = h2;

    return m_byteCount;
  }

protected:
  void processBlock() {
    uint64_t k1 = getblock64((uint64_t *)m_block, 0);
    uint64_t k2 = getblock64((uint64_t *)m_block, 1);

    k1 *= c1;
    k1 = ROTL64(k1, 31);
    k1 *= c2;
    h1 ^= k1;

    h1 = ROTL64(h1, 27);
    h1 += h2;
    h1 = h1 * 5 + 0x52dce729;

    k2 *= c2;
    k2 = ROTL64(k2, 33);
    k2 *= c1;
    h2 ^= k2;

    h2 = ROTL64(h2, 31);
    h2 += h1;
    h2 = h2 * 5 + 0x38495ab5;
  }

  static constexpr uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
  static constexpr uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);

  uint64_t h1, h2;
  uint8_t m_block[BLOCK_LENGTH];
  size_t m_blockByteIndex;
  size_t m_byteCount;
};

#endif
