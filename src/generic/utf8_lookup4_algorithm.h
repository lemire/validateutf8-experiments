
using namespace simd;


  // ----------------------------------------------------------------------------
  // lookup4 goal:
  // keep all UTF-8 validation decisions in SIMD-friendly bitmasks so every lane
  // can be checked with a small fixed sequence of LUTs and logical operations.
  // ----------------------------------------------------------------------------


  // This helper computes error bits by looking at byte pairs (prev1, input)
  // with three LUTs. Each lane carries a bitmask of possible UTF-8 violations.
  really_inline simd8<uint8_t> check_special_cases(const simd8<uint8_t> input, const simd8<uint8_t> prev1) {
   // Error bits tracked in each lane:
   // Bit 0 = Too Short (lead/ASCII followed by lead/ASCII while continuation expected)
   // Bit 1 = Too Long (ASCII followed by continuation)
   // Bit 2 = Overlong 3-byte sequence
   // Bit 3 = Too Large scalar value (part of > U+10FFFF detection)
   // Bit 4 = Surrogate range encoded in UTF-8
   // Bit 5 = Overlong 2-byte sequence
   // Bit 6 = Boundary-case marker for OVERLONG_4 / TOO_LARGE_1000
   // Bit 7 = Two continuation bytes in a row
    constexpr const uint8_t TOO_SHORT   = 1<<0; // 11______ 0_______
                                                // 11______ 11______
    constexpr const uint8_t TOO_LONG    = 1<<1; // 0_______ 10______
    constexpr const uint8_t OVERLONG_3  = 1<<2; // 11100000 100_____
    constexpr const uint8_t SURROGATE   = 1<<4; // 11101101 101_____
    constexpr const uint8_t OVERLONG_2  = 1<<5; // 1100000_ 10______
    constexpr const uint8_t TWO_CONTS   = 1<<7; // 10______ 10______
    constexpr const uint8_t TOO_LARGE   = 1<<3; // 11110100 1001____
                                                // 11110100 101_____
                                                // 11110101 1001____
                                                // 11110101 101_____
                                                // 1111011_ 1001____
                                                // 1111011_ 101_____
                                                // 11111___ 1001____
                                                // 11111___ 101_____
    constexpr const uint8_t TOO_LARGE_1000 = 1<<6;
                                                // 11110101 1000____
                                                // 1111011_ 1000____
                                                // 11111___ 1000____
    constexpr const uint8_t OVERLONG_4  = 1<<6; // 11110000 1000____

    // LUT #1: classify prev1 by high nibble.
    // This gives coarse context: ASCII, continuation, 2-byte lead, 3-byte lead, 4-byte lead.
    const simd8<uint8_t> byte_1_high = prev1.shr<4>().lookup_16<uint8_t>(
      // 0_______ ________ <ASCII in byte 1>
      TOO_LONG, TOO_LONG, TOO_LONG, TOO_LONG,
      TOO_LONG, TOO_LONG, TOO_LONG, TOO_LONG,
      // 10______ ________ <continuation in byte 1>
      TWO_CONTS, TWO_CONTS, TWO_CONTS, TWO_CONTS,
      // 1100____ ________ <two byte lead in byte 1>
      TOO_SHORT | OVERLONG_2,
      // 1101____ ________ <two byte lead in byte 1>
      TOO_SHORT,
      // 1110____ ________ <three byte lead in byte 1>
      TOO_SHORT | OVERLONG_3 | SURROGATE,
      // 1111____ ________ <four+ byte lead in byte 1>
      TOO_SHORT | TOO_LARGE | TOO_LARGE_1000 | OVERLONG_4
    );

    // "Carry" bits are the bits that survive into low-nibble refinement.
    constexpr const uint8_t CARRY = TOO_SHORT | TOO_LONG | TWO_CONTS; // These all have ____ in byte 1 .

    // LUT #2: refine prev1 with low nibble for boundary values
    // (e.g., E0/ED/F0/F4) that determine overlong/surrogate/too-large behavior.
    const simd8<uint8_t> byte_1_low = (prev1 & 0x0F).lookup_16<uint8_t>(
      // ____0000 ________
      CARRY | OVERLONG_3 | OVERLONG_2 | OVERLONG_4,
      // ____0001 ________
      CARRY | OVERLONG_2,
      // ____001_ ________
      CARRY,
      CARRY,

      // ____0100 ________
      CARRY | TOO_LARGE,
      // ____0101 ________
      CARRY | TOO_LARGE | TOO_LARGE_1000,
      // ____011_ ________
      CARRY | TOO_LARGE | TOO_LARGE_1000,
      CARRY | TOO_LARGE | TOO_LARGE_1000,

      // ____1___ ________
      CARRY | TOO_LARGE | TOO_LARGE_1000,
      CARRY | TOO_LARGE | TOO_LARGE_1000,
      CARRY | TOO_LARGE | TOO_LARGE_1000,
      CARRY | TOO_LARGE | TOO_LARGE_1000,
      CARRY | TOO_LARGE | TOO_LARGE_1000,
      // ____1101 ________
      CARRY | TOO_LARGE | TOO_LARGE_1000 | SURROGATE,
      CARRY | TOO_LARGE | TOO_LARGE_1000,
      CARRY | TOO_LARGE | TOO_LARGE_1000
    );

    // LUT #3: classify current byte by high nibble.
    // This separates ASCII, continuation subranges 8/9/A/B, and lead bytes.
    const simd8<uint8_t> byte_2_high = input.shr<4>().lookup_16<uint8_t>(
      // ________ 0_______ <ASCII in byte 2>
      TOO_SHORT, TOO_SHORT, TOO_SHORT, TOO_SHORT,
      TOO_SHORT, TOO_SHORT, TOO_SHORT, TOO_SHORT,

      // ________ 1000____
      TOO_LONG | OVERLONG_2 | TWO_CONTS | OVERLONG_3 | TOO_LARGE_1000 | OVERLONG_4,
      // ________ 1001____
      TOO_LONG | OVERLONG_2 | TWO_CONTS | OVERLONG_3 | TOO_LARGE,
      // ________ 101_____
      TOO_LONG | OVERLONG_2 | TWO_CONTS | SURROGATE  | TOO_LARGE,
      TOO_LONG | OVERLONG_2 | TWO_CONTS | SURROGATE  | TOO_LARGE,

      // ________ 11______
      TOO_SHORT, TOO_SHORT, TOO_SHORT, TOO_SHORT
    );

    // Keep only error bits consistent with all three viewpoints.
    // If a bit survives this AND, we have enough evidence for that error class.
    return (byte_1_high & byte_1_low & byte_2_high);
  }

  // Validate continuation-length obligations and combine with special-case bits.
  really_inline simd8<uint8_t> check_multibyte_lengths(simd8<uint8_t> input, simd8<uint8_t> prev_input, simd8<uint8_t> sc) {
    // Create N-2 and N-3 aligned vectors to infer mandatory continuation slots.
    simd8<uint8_t> prev2 = input.prev<2>(prev_input);
    simd8<uint8_t> prev3 = input.prev<3>(prev_input);

    // Helper emits mask for bytes that must be continuation bytes.
    simd8<uint8_t> must23 = simd8<uint8_t>(must_be_2_3_continuation(prev2, prev3));

    // Keep only the continuation marker bit (bit 7).
    simd8<uint8_t> must23_80 = must23 & uint8_t(0x80);

    // XOR highlights mismatches between expected continuation status and
    // special-case validity mask. Any nonzero lane means invalid UTF-8.
    return must23_80 ^ sc;
  }

  //
  // Return nonzero if there are incomplete multibyte characters at the end of the block:
  // e.g. if there is a 4-byte character, but it's 3 bytes from the end.
  //
  really_inline simd8<uint8_t> is_incomplete(simd8<uint8_t> input) {
    // Tail threshold table: only the last few lanes are constrained;
    // all earlier lanes are 255 (no incomplete-byte signal there).
    // If the previous input's last 3 bytes match this, they're too short (they ended at EOF):
    // ... 1111____ 111_____ 11______
    static const uint8_t max_array[32] = {
      255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 0b11110000u-1, 0b11100000u-1, 0b11000000u-1
    };

    // Load the tail window corresponding to current SIMD width.
    const simd8<uint8_t> max_value(&max_array[sizeof(max_array)-sizeof(simd8<uint8_t>)]);

    // Any byte exceeding lane threshold indicates an incomplete lead at block end.
    return input.gt_bits(max_value);
  }

  // Stateful UTF-8 checker across 64-byte blocks.
  // It stores just enough context to make block-boundary validation correct.
  struct utf8_checker {
    // Global error accumulator; any nonzero bit means invalid UTF-8 encountered.
    simd8<uint8_t> error;

    // Last 16-byte chunk from the previous block (context for input.prev<k>). 
    simd8<uint8_t> prev_input_block;

    // Marker of whether previous block ended in an incomplete sequence.
    // Needed so an ASCII-only block does not accidentally "hide" prior incompleteness.
    simd8<uint8_t> prev_incomplete;

    //
    // Check whether the current bytes are valid UTF-8.
    //
    really_inline void check_utf8_bytes(const simd8<uint8_t> input, const simd8<uint8_t> prev_input) {
      // Align previous byte with each lane of input.
      simd8<uint8_t> prev1 = input.prev<1>(prev_input);

      // Compute pairwise special-case flags.
      simd8<uint8_t> sc = check_special_cases(input, prev1);

      // Merge length/special-case validation into global error state.
      this->error |= check_multibyte_lengths(input, prev_input, sc);
    }

    // The only problem that can happen at EOF is that a multibyte character is too short.
    really_inline void check_eof() {
      // If the previous block had incomplete UTF-8 characters at the end, an ASCII block can't
      // possibly finish them.
      this->error |= this->prev_incomplete;
    }

    // Process one 64-byte block split into SIMD chunks.
    really_inline void check_next_input(simd8x64<uint8_t> input) {
      // Fast path: pure ASCII needs no internal UTF-8 table checks.
      // Boundary state still matters (previous block may have ended mid-sequence).
      if (likely(is_ascii(input))) {
        // If the previous block had incomplete UTF-8 characters at the end, an ASCII block can't
        // possibly finish them.
        this->error |= this->prev_incomplete;
      } else {
        // First chunk uses saved previous-block tail as context.
        this->check_utf8_bytes(input.chunks[0], this->prev_input_block);

        // Remaining chunks use the immediate previous chunk as context.
        for (int i=1; i<simd8x64<uint8_t>::NUM_CHUNKS; i++) {
          this->check_utf8_bytes(input.chunks[i], input.chunks[i-1]);
        }

        // Save boundary state for next block.
        this->prev_incomplete = is_incomplete(input.chunks[simd8x64<uint8_t>::NUM_CHUNKS-1]);
        this->prev_input_block = input.chunks[simd8x64<uint8_t>::NUM_CHUNKS-1];
      }
    }

    // Return final status in public API form.
    really_inline error_code errors() {
      return this->error.any_bits_set_anywhere() ? error_code::UTF8_ERROR : error_code::SUCCESS;
    }

  }; // struct utf8_checker


  #include "validator.h"