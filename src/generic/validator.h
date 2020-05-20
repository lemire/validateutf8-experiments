  template<class checker>
  error_code validate(const char * input, size_t length) {
    checker c{};
    buf_block_reader<64> reader(input, length);
    while (reader.has_full_block()) {
      simd::simd8x64<uint8_t> in(reader.full_block());
      c.check_next_input(in);
      reader.advance();
    }

    if (likely(reader.has_remainder())) {
      uint8_t block[64];
      reader.get_remainder(block);
      simd::simd8x64<uint8_t> in(reader.full_block());
      c.check_next_input(in);
      reader.advance();
    }
    return c.errors();
  }

  error_code validate(const char * input, size_t length) {
    return validate<utf8_checker>(input,length);
  }

  error_code validate(const uint8_t * input, size_t length) {
    return validate<utf8_checker>(reinterpret_cast<const char *>(input),length);
  }