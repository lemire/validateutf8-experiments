#include "block_buf_reader.h"
#include "avx2/simd.h"


TARGET_HASWELL
namespace fastvalidate {
namespace haswell {

namespace lookup2 {
using namespace simd;
really_inline bool is_ascii(simd8x64<uint8_t> input) {
  simd8<uint8_t> bits = input.reduce([&](simd8<uint8_t> a,simd8<uint8_t> b) { return a|b; });
  return !bits.any_bits_set_anywhere(0b10000000u);
}

really_inline simd8<bool> must_be_continuation(simd8<uint8_t> prev1, simd8<uint8_t> prev2, simd8<uint8_t> prev3) {
  simd8<uint8_t> is_second_byte = prev1.saturating_sub(0b11000000u-1); // Only 11______ will be > 0
  simd8<uint8_t> is_third_byte  = prev2.saturating_sub(0b11100000u-1); // Only 111_____ will be > 0
  simd8<uint8_t> is_fourth_byte = prev3.saturating_sub(0b11110000u-1); // Only 1111____ will be > 0
  // Caller requires a bool (all 1's). All values resulting from the subtraction will be <= 64, so signed comparison is fine.
  return simd8<int8_t>(is_second_byte | is_third_byte | is_fourth_byte) > simd8<int8_t>::zero();
}
#include "generic/utf8_lookup2_algorithm.h"
}

namespace lookup3 {
using namespace simd;
really_inline bool is_ascii(simd8x64<uint8_t> input) {
  simd8<uint8_t> bits = input.reduce([&](simd8<uint8_t> a,simd8<uint8_t> b) { return a|b; });
  return !bits.any_bits_set_anywhere(0b10000000u);
}

really_inline simd8<bool> must_be_continuation(simd8<uint8_t> prev1, simd8<uint8_t> prev2, simd8<uint8_t> prev3) {
  simd8<uint8_t> is_second_byte = prev1.saturating_sub(0b11000000u-1); // Only 11______ will be > 0
  simd8<uint8_t> is_third_byte  = prev2.saturating_sub(0b11100000u-1); // Only 111_____ will be > 0
  simd8<uint8_t> is_fourth_byte = prev3.saturating_sub(0b11110000u-1); // Only 1111____ will be > 0
  // Caller requires a bool (all 1's). All values resulting from the subtraction will be <= 64, so signed comparison is fine.
  return simd8<int8_t>(is_second_byte | is_third_byte | is_fourth_byte) > simd8<int8_t>::zero();
}


really_inline simd8<bool> must_be_2_3_continuation(simd8<uint8_t> prev2, simd8<uint8_t> prev3) {
  simd8<uint8_t> is_third_byte  = prev2.saturating_sub(0b11100000u-1); // Only 111_____ will be > 0
  simd8<uint8_t> is_fourth_byte = prev3.saturating_sub(0b11110000u-1); // Only 1111____ will be > 0
  // Caller requires a bool (all 1's). All values resulting from the subtraction will be <= 64, so signed comparison is fine.
  return simd8<int8_t>(is_third_byte | is_fourth_byte) > simd8<int8_t>::zero();
}

#include "generic/utf8_lookup3_algorithm.h"
}


namespace lookup4 {
using namespace simd;
really_inline bool is_ascii(simd8x64<uint8_t> input) {
  simd8<uint8_t> bits = input.reduce_or();
  return bits.is_ascii();
}

really_inline simd8<bool> must_be_continuation(simd8<uint8_t> prev1, simd8<uint8_t> prev2, simd8<uint8_t> prev3) {
  simd8<uint8_t> is_second_byte = prev1.saturating_sub(0b11000000u-1); // Only 11______ will be > 0
  simd8<uint8_t> is_third_byte  = prev2.saturating_sub(0b11100000u-1); // Only 111_____ will be > 0
  simd8<uint8_t> is_fourth_byte = prev3.saturating_sub(0b11110000u-1); // Only 1111____ will be > 0
  // Caller requires a bool (all 1's). All values resulting from the subtraction will be <= 64, so signed comparison is fine.
  return simd8<int8_t>(is_second_byte | is_third_byte | is_fourth_byte) > simd8<int8_t>::zero();
}


really_inline simd8<bool> must_be_2_3_continuation(simd8<uint8_t> prev2, simd8<uint8_t> prev3) {
  simd8<uint8_t> is_third_byte  = prev2.saturating_sub(0b11100000u-1); // Only 111_____ will be > 0
  simd8<uint8_t> is_fourth_byte = prev3.saturating_sub(0b11110000u-1); // Only 1111____ will be > 0
  // Caller requires a bool (all 1's). All values resulting from the subtraction will be <= 64, so signed comparison is fine.
  return simd8<int8_t>(is_third_byte | is_fourth_byte) > simd8<int8_t>::zero();
}

#include "generic/utf8_lookup4_algorithm.h"
}

namespace zwegner {
#include "generic/utf8_zwegner_algorithm.h"
}

namespace range {
#include "generic/utf8_range_algorithm.h"
}


namespace basic {
#include "generic/utf8_fastvalidate_algorithm.h"
}

}
}
UNTARGET_REGION