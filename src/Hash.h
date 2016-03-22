#ifndef __CLANG_HASH_HASH
#define __CLANG_HASH_HASH

// #include "SHA1.h"
#include "MurMurHash3.h"


struct Hash : protected MurMurHash3 {

    typedef MurMurHash3 algorithm;

    struct digest {
        enum { DIGEST_WORDS = algorithm::DIGEST_WORDS };
        uint32_t value[algorithm::DIGEST_WORDS];
        uint32_t length;

        bool operator==(const digest &other) const {
            for (unsigned i = 0; i < DIGEST_WORDS; i++) {
                if (other.value[i] != value[i])
                    return false;
            }
            return true;
        }
        bool operator!=(const digest &other) const {
            return !(*this == other);
        }


        std::string getHexDigest() const {
            std::stringstream ss;
            for (unsigned i = 0; i < DIGEST_WORDS; i++) {
                ss << std::hex << std::setfill('0') << std::setw(8)
                   << value[i];
            }
            return ss.str();
        }
    };

    Hash& processBlock(const void* const start, const void* const end) {
        const uint8_t* begin = static_cast<const uint8_t*>(start);
        const uint8_t* finish = static_cast<const uint8_t*>(end);
        while(begin != finish) {
            processByte(*begin);
            begin++;
        }
        return *this;
    }

    Hash& processBytes(const void* const data, size_t len) {
        const uint8_t* block = static_cast<const uint8_t*>(data);
        processBlock(block, block + len);
        return *this;
    }

    // Digest Operators
    const digest getDigest() const {
        Hash copy = *this;
        digest ret;
        ret.length = copy.finalize(ret.value);
        return ret;
    }

    // Stream Operators
    Hash& operator<<(uint8_t x) {
        processByte(x);
        return *this;
    }

    Hash& operator<<(uint16_t x) {
        *this << (uint8_t) (x & 0xFF) << (uint8_t)(x >> 8);
        return *this;
    }

    Hash& operator<<(uint32_t x) {
        *this << (uint16_t)(x & 0xFFFF) << (uint16_t) (x >> 16);
        return *this;
    }

    Hash& operator<<(uint64_t x) {
        *this << (uint32_t)(x & 0xFFFFFFFF) << (uint32_t)(x >> 32);
        return *this;
    }

    Hash& operator<<(int8_t x) {
        processByte(x);
        return *this;
    }

    Hash& operator<<(int16_t x) {
        *this << (int8_t) (x & 0xFF) << (int8_t)(x >> 8);
        return *this;
    }
    Hash& operator<<(int32_t x) {
        *this << (int16_t)(x & 0xFFFF) << (int16_t) (x >> 16);
        return *this;
    }
    Hash& operator<<(int64_t x) {
        *this << (int32_t)(x & 0xFFFFFFFF) << (int32_t)(x >> 32);
        return *this;
    }

    Hash& operator<<(const digest & x) {
        m_byteCount += x.length;
        return  processBytes(x.value, sizeof(x.value));
    }

    Hash& operator<<(const Hash & other) {
        digest digest = other.getDigest();
        *this << digest;
        return *this;
    }

    Hash& operator<<(const std::string &x) {
        processBytes(x.c_str(), x.length());
        return *this;
    }
};
#endif
