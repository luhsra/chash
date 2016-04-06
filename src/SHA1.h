/*
 *
 * TinySHA1 - a header only implementation of the SHA1 algorithm in C++. Based
 * on the implementation in boost::uuid::details.
 *
 * SHA1 Wikipedia Page: http://en.wikipedia.org/wiki/SHA-1
 *
 * Copyright (c) 2012-22 SAURAV MOHAPATRA <mohaps@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef _TINY_SHA1_HPP_
#define _TINY_SHA1_HPP_
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

class SHA1 {
public:
  enum { DIGEST_WORDS = 5, BLOCK_LENGTH = 64 };

  SHA1() { reset(); }
  virtual ~SHA1() {}
  SHA1(const SHA1 &s) { *this = s; }

  const SHA1 &operator=(const SHA1 &s) {
    memcpy(m_digest, s.m_digest, DIGEST_WORDS * sizeof(uint32_t));
    memcpy(m_block, s.m_block, BLOCK_LENGTH);
    m_blockByteIndex = s.m_blockByteIndex;
    m_byteCount = s.m_byteCount;
    return *this;
  }

  SHA1 &reset() {
    m_digest[0] = 0x67452301;
    m_digest[1] = 0xEFCDAB89;
    m_digest[2] = 0x98BADCFE;
    m_digest[3] = 0x10325476;
    m_digest[4] = 0xC3D2E1F0;
    m_blockByteIndex = 0;
    m_byteCount = 0;
    return *this;
  }

  SHA1 &processByte(uint8_t octet) {
    this->m_block[this->m_blockByteIndex++] = octet;
    ++this->m_byteCount;
    if (m_blockByteIndex == BLOCK_LENGTH) {
      this->m_blockByteIndex = 0;
      processBlock();
    }
    return *this;
  }

  uint32_t finalize(uint32_t *digest) {
    size_t bitCount = m_byteCount * 8;
    processByte(0x80);
    if (m_blockByteIndex > 56) {
      while (m_blockByteIndex != 0) {
        processByte(0);
      }
      while (m_blockByteIndex < 56) {
        processByte(0);
      }
    } else {
      while (m_blockByteIndex < 56) {
        processByte(0);
      }
    }
    processByte(0);
    processByte(0);
    processByte(0);
    processByte(0);
    processByte(static_cast<unsigned char>((bitCount >> 24) & 0xFF));
    processByte(static_cast<unsigned char>((bitCount >> 16) & 0xFF));
    processByte(static_cast<unsigned char>((bitCount >> 8) & 0xFF));
    processByte(static_cast<unsigned char>((bitCount)&0xFF));

    memcpy(digest, m_digest, sizeof(m_digest));

    return m_byteCount;
  }

private:
  inline static uint32_t LeftRotate(uint32_t value, size_t count) {
    return (value << count) ^ (value >> (32 - count));
  }

  void processBlock() {
    uint32_t w[80];
    for (size_t i = 0; i < 16; i++) {
      w[i] = (m_block[i * 4 + 0] << 24);
      w[i] |= (m_block[i * 4 + 1] << 16);
      w[i] |= (m_block[i * 4 + 2] << 8);
      w[i] |= (m_block[i * 4 + 3]);
    }
    for (size_t i = 16; i < 80; i++) {
      w[i] = LeftRotate((w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]), 1);
    }

    uint32_t a = m_digest[0];
    uint32_t b = m_digest[1];
    uint32_t c = m_digest[2];
    uint32_t d = m_digest[3];
    uint32_t e = m_digest[4];

    for (std::size_t i = 0; i < 80; ++i) {
      uint32_t f = 0;
      uint32_t k = 0;

      if (i < 20) {
        f = (b & c) | (~b & d);
        k = 0x5A827999;
      } else if (i < 40) {
        f = b ^ c ^ d;
        k = 0x6ED9EBA1;
      } else if (i < 60) {
        f = (b & c) | (b & d) | (c & d);
        k = 0x8F1BBCDC;
      } else {
        f = b ^ c ^ d;
        k = 0xCA62C1D6;
      }
      uint32_t temp = LeftRotate(a, 5) + f + e + k + w[i];
      e = d;
      d = c;
      c = LeftRotate(b, 30);
      b = a;
      a = temp;
    }

    m_digest[0] += a;
    m_digest[1] += b;
    m_digest[2] += c;
    m_digest[3] += d;
    m_digest[4] += e;
  }

  uint32_t m_digest[DIGEST_WORDS];
  uint8_t m_block[BLOCK_LENGTH];
  size_t m_blockByteIndex;
  size_t m_byteCount;
};
#endif
