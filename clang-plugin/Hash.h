#ifndef __CLANG_HASH_HASH
#define __CLANG_HASH_HASH

// #include "SHA1.h"
#include "MurMurHash3.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Format.h"
#include <array>

namespace llvm {
struct MurMur3 : protected MurMurHash3 {
    struct Digest {
        std::array<uint8_t, 16> Bytes;

        SmallString<32> digest() const {
            SmallString<32> Str;
            raw_svector_ostream Res(Str);
            for (int i = 0; i < 16; ++i)
                Res << format("%.2x", Bytes[i]);
            return Str;
        }
    };

    /// Add the bytes in the StringRef \p Str to the hash.
    // Note that this isn't a string and so this won't include any trailing NULL
    // bytes.
    void update(StringRef Str) {
        ArrayRef<uint8_t> SVal((const uint8_t *)Str.data(), Str.size());
        update(SVal);
    }

    void update(ArrayRef<uint8_t> Data) {
        const uint8_t *Ptr = Data.data();
        unsigned long Size = Data.size();
        while (Size-- > 0) {
            processByte(*Ptr);
            Ptr++;
        }
    }

    void final(Digest &Ret)  const {
        MurMur3 Copy = *this;
        Copy.finalize((uint32_t*)Ret.Bytes.data());
    }
};

}
#endif
