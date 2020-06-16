#include "block_buf_reader.h"
#include "neon/simd.h"


namespace fastvalidate {
namespace arm64 {
namespace lookup2 {
using namespace simd;
really_inline bool is_ascii(simd8x64<uint8_t> input) {
    simd8<uint8_t> bits = input.reduce([&](simd8<uint8_t> a,simd8<uint8_t> b) { return a|b; });
    return bits.max() < 0b10000000u;
}

really_inline simd8<bool> must_be_continuation(simd8<uint8_t> prev1, simd8<uint8_t> prev2, simd8<uint8_t> prev3) {
    simd8<bool> is_second_byte = prev1 >= uint8_t(0b11000000u);
    simd8<bool> is_third_byte  = prev2 >= uint8_t(0b11100000u);
    simd8<bool> is_fourth_byte = prev3 >= uint8_t(0b11110000u);
    // Use ^ instead of | for is_*_byte, because ^ is commutative, and the caller is using ^ as well.
    // This will work fine because we only have to report errors for cases with 0-1 lead bytes.
    // Multiple lead bytes implies 2 overlapping multibyte characters, and if that happens, there is
    // guaranteed to be at least *one* lead byte that is part of only 1 other multibyte character.
    // The error will be detected there.
    return is_second_byte ^ is_third_byte ^ is_fourth_byte;
}

#include "generic/utf8_lookup2_algorithm.h"
}

namespace lookup3 {
using namespace simd;
really_inline bool is_ascii(simd8x64<uint8_t> input) {
    simd8<uint8_t> bits = input.reduce([&](simd8<uint8_t> a,simd8<uint8_t> b) { return a|b; });
    return bits.max() < 0b10000000u;
}

really_inline simd8<bool> must_be_continuation(simd8<uint8_t> prev1, simd8<uint8_t> prev2, simd8<uint8_t> prev3) {
    simd8<bool> is_second_byte = prev1 >= uint8_t(0b11000000u);
    simd8<bool> is_third_byte  = prev2 >= uint8_t(0b11100000u);
    simd8<bool> is_fourth_byte = prev3 >= uint8_t(0b11110000u);
    // Use ^ instead of | for is_*_byte, because ^ is commutative, and the caller is using ^ as well.
    // This will work fine because we only have to report errors for cases with 0-1 lead bytes.
    // Multiple lead bytes implies 2 overlapping multibyte characters, and if that happens, there is
    // guaranteed to be at least *one* lead byte that is part of only 1 other multibyte character.
    // The error will be detected there.
    return is_second_byte ^ is_third_byte ^ is_fourth_byte;
}


really_inline simd8<bool> must_be_2_3_continuation(simd8<uint8_t> prev2, simd8<uint8_t> prev3) {
    simd8<bool> is_third_byte  = prev2 >= uint8_t(0b11100000u);
    simd8<bool> is_fourth_byte = prev3 >= uint8_t(0b11110000u);
    return is_third_byte ^ is_fourth_byte;
}

#include "generic/utf8_lookup3_algorithm.h"
}


namespace range {
#include "generic/utf8_range_algorithm.h"
}


namespace basic {
#include "generic/utf8_fastvalidate_algorithm.h"
}

}
}
