
using namespace simd;



  really_inline simd8<uint8_t> check_special_cases(const simd8<uint8_t> input, const simd8<uint8_t> prev1) {
    const simd8<uint8_t> byte_1_high = prev1.shr<4>().lookup_16<uint8_t>(49, 49, 49, 49, 49, 49, 49, 49, 0, 0, 0, 0, 76, 8, 28, 58);
    const simd8<uint8_t> byte_1_low = (prev1 & 0x0F).lookup_16<uint8_t>(79, 73, 9, 9, 41, 43, 43, 43, 43, 43, 43, 43, 43, 59, 43, 43);
    const simd8<uint8_t> byte_2_high = input.shr<4>().lookup_16<uint8_t>(78, 78, 78, 78, 78, 78, 78, 78, 71, 101, 113, 113, 78, 78, 78, 78);
    return byte_1_high & byte_1_low & byte_2_high;
  }

  really_inline simd8<uint8_t> check_multibyte_lengths(simd8<uint8_t> input, simd8<uint8_t> prev_input, simd8<uint8_t> prev1) {
    simd8<uint8_t> prev2 = input.prev<2>(prev_input);
    simd8<uint8_t> prev3 = input.prev<3>(prev_input);
    // is_2_3_continuation uses one more instruction than lookup2
    simd8<bool> is_2_3_continuation = (simd8<int8_t>(input).max(simd8<int8_t>(prev1))) < int8_t(-64);
    // must_be_2_3_continuation has two fewer instructions than lookup 2
    return simd8<uint8_t>(must_be_2_3_continuation(prev2, prev3) ^ is_2_3_continuation);
  }

  //
  // Return nonzero if there are incomplete multibyte characters at the end of the block:
  // e.g. if there is a 4-byte character, but it's 3 bytes from the end.
  //
  really_inline simd8<uint8_t> is_incomplete(simd8<uint8_t> input) {
    // If the previous input's last 3 bytes match this, they're too short (they ended at EOF):
    // ... 1111____ 111_____ 11______
    static const uint8_t max_array[32] = {
      255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 0b11110000u-1, 0b11100000u-1, 0b11000000u-1
    };
    const simd8<uint8_t> max_value(&max_array[sizeof(max_array)-sizeof(simd8<uint8_t>)]);
    return input.gt_bits(max_value);
  }

  struct utf8_checker {
    // If this is nonzero, there has been a UTF-8 error.
    simd8<uint8_t> error;
    // The last input we received
    simd8<uint8_t> prev_input_block;
    // Whether the last input we received was incomplete (used for ASCII fast path)
    simd8<uint8_t> prev_incomplete;

    //
    // Check whether the current bytes are valid UTF-8.
    //
    really_inline void check_utf8_bytes(const simd8<uint8_t> input, const simd8<uint8_t> prev_input) {
      // Flip prev1...prev3 so we can easily determine if they are 2+, 3+ or 4+ lead bytes
      // (2, 3, 4-byte leads become large positive numbers instead of small negative numbers)
      simd8<uint8_t> prev1 = input.prev<1>(prev_input);
      this->error |= check_special_cases(input, prev1);
      this->error |= check_multibyte_lengths(input, prev_input, prev1);
    }

    // The only problem that can happen at EOF is that a multibyte character is too short.
    really_inline void check_eof() {
      // If the previous block had incomplete UTF-8 characters at the end, an ASCII block can't
      // possibly finish them.
      this->error |= this->prev_incomplete;
    }

    really_inline void check_next_input(simd8x64<uint8_t> input) {
      if (likely(is_ascii(input))) {
        // If the previous block had incomplete UTF-8 characters at the end, an ASCII block can't
        // possibly finish them.
        this->error |= this->prev_incomplete;
      } else {
        this->check_utf8_bytes(input.chunks[0], this->prev_input_block);
        for (int i=1; i<simd8x64<uint8_t>::NUM_CHUNKS; i++) {
          this->check_utf8_bytes(input.chunks[i], input.chunks[i-1]);
        }
        this->prev_incomplete = is_incomplete(input.chunks[simd8x64<uint8_t>::NUM_CHUNKS-1]);
        this->prev_input_block = input.chunks[simd8x64<uint8_t>::NUM_CHUNKS-1];
      }
    }

    really_inline error_code errors() {
      return this->error.any_bits_set_anywhere() ? error_code::UTF8_ERROR : error_code::SUCCESS;
    }

  }; // struct utf8_checker


  #include "validator.h"