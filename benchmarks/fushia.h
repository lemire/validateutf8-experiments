// adapted from
// https://fuchsia.googlesource.com/fuchsia/+/master/zircon/system/ulib/fidl/validate_string.cc#7
// see the source of licensing.
#include "targets.h"
#include <cstdint>

fastvalidate::error_code fidl_validate_string(const unsigned char *data,
                                              uint64_t size) {
  uint64_t pos = 0;
  uint64_t next_pos = 0;
  uint32_t code_point = 0;
  while (pos < size) {
    // The following comparison relies on treating the byte as if it was an
    // unsigned 8-bit value. However, both signed and unsigned char are allowed
    // in the C++ spec, with x64 choosing signed, and arm64 choosing unsigned.
    // We therefore force the byte to be treated as unsigned, since we cannot
    // rely on the default.
    unsigned char byte = data[pos];
    if (byte < 0b10000000) {
      pos++;
      continue;
    } else if ((byte & 0b11100000) == 0b11000000) {
      next_pos = pos + 2;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point = (byte & 0b00011111) << 6 | (data[pos + 1] & 0b00111111);
      if (code_point < 0x80 || 0x7ff < code_point) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else if ((byte & 0b11110000) == 0b11100000) {
      next_pos = pos + 3;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 2] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point = (byte & 0b00001111) << 12 |
                   (data[pos + 1] & 0b00111111) << 6 |
                   (data[pos + 2] & 0b00111111);
      if (code_point < 0x800 || 0xffff < code_point ||
          (0xd7ff < code_point && code_point < 0xe000)) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else if ((byte & 0b11111000) == 0b11110000) { // 0b11110000
      next_pos = pos + 4;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 2] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 3] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point =
          (byte & 0b00000111) << 18 | (data[pos + 1] & 0b00111111) << 12 |
          (data[pos + 2] & 0b00111111) << 6 | (data[pos + 3] & 0b00111111);
      if (code_point < 0xffff || 0x10ffff < code_point) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else {
      // we may have a continuation
      return fastvalidate::error_code::UTF8_ERROR;
    }
    pos = next_pos;
  }
  return fastvalidate::error_code::SUCCESS;
}

fastvalidate::error_code fidl_validate_string_ascii4(const unsigned char *data,
                                              uint64_t size) {
  uint64_t pos = 0;
  uint64_t next_pos = 0;
  uint32_t code_point = 0;
  while (pos < size) {

    // check of the next 8 bytes are ascii.
    next_pos = pos + 32;
    if (next_pos <= size) { // if it is safe to read 8 more bytes, check that they are ascii 
        uint64_t v1;
        memcpy(&v1, data + pos, sizeof(uint64_t));
        uint64_t v2;
        memcpy(&v2, data + pos + sizeof(uint64_t), sizeof(uint64_t));
        uint64_t v3;
        memcpy(&v3, data + pos + 2 * sizeof(uint64_t), sizeof(uint64_t));
        uint64_t v4;
        memcpy(&v4, data + pos + 3 * sizeof(uint64_t), sizeof(uint64_t));
        uint64_t v{ v1 | v2 | v3 | v4};
        if((v & 0x8080808080808080)  == 0) {
          pos = next_pos;
          continue;
        }
    }
    unsigned char byte = data[pos];

    if (byte < 0b10000000) {
      pos++;
      continue;
    } else if ((byte & 0b11100000) == 0b11000000) {
      next_pos = pos + 2;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point = (byte & 0b00011111) << 6 | (data[pos + 1] & 0b00111111);
      if (code_point < 0x80 || 0x7ff < code_point) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else if ((byte & 0b11110000) == 0b11100000) {
      next_pos = pos + 3;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 2] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point = (byte & 0b00001111) << 12 |
                   (data[pos + 1] & 0b00111111) << 6 |
                   (data[pos + 2] & 0b00111111);
      if (code_point < 0x800 || 0xffff < code_point ||
          (0xd7ff < code_point && code_point < 0xe000)) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else if ((byte & 0b11111000) == 0b11110000) { // 0b11110000
      next_pos = pos + 4;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 2] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 3] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point =
          (byte & 0b00000111) << 18 | (data[pos + 1] & 0b00111111) << 12 |
          (data[pos + 2] & 0b00111111) << 6 | (data[pos + 3] & 0b00111111);
      if (code_point < 0xffff || 0x10ffff < code_point) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else {
      // we may have a continuation
      return fastvalidate::error_code::UTF8_ERROR;
    }
    pos = next_pos;
  }
  return fastvalidate::error_code::SUCCESS;
}

fastvalidate::error_code fidl_validate_string_ascii2(const unsigned char *data,
                                              uint64_t size) {
  uint64_t pos = 0;
  uint64_t next_pos = 0;
  uint32_t code_point = 0;
  while (pos < size) {

    // check of the next 8 bytes are ascii.
    next_pos = pos + 16;
    if (next_pos <= size) { // if it is safe to read 8 more bytes, check that they are ascii 
        uint64_t v1;
        memcpy(&v1, data + pos, sizeof(uint64_t));
        uint64_t v2;
        memcpy(&v2, data + pos + sizeof(uint64_t), sizeof(uint64_t));
        uint64_t v{ v1 | v2};
        if((v & 0x8080808080808080)  == 0) {
          pos = next_pos;
          continue;
        }
    }
    unsigned char byte = data[pos];

    if (byte < 0b10000000) {
      pos++;
      continue;
    } else if ((byte & 0b11100000) == 0b11000000) {
      next_pos = pos + 2;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point = (byte & 0b00011111) << 6 | (data[pos + 1] & 0b00111111);
      if (code_point < 0x80 || 0x7ff < code_point) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else if ((byte & 0b11110000) == 0b11100000) {
      next_pos = pos + 3;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 2] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point = (byte & 0b00001111) << 12 |
                   (data[pos + 1] & 0b00111111) << 6 |
                   (data[pos + 2] & 0b00111111);
      if (code_point < 0x800 || 0xffff < code_point ||
          (0xd7ff < code_point && code_point < 0xe000)) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else if ((byte & 0b11111000) == 0b11110000) { // 0b11110000
      next_pos = pos + 4;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 2] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 3] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point =
          (byte & 0b00000111) << 18 | (data[pos + 1] & 0b00111111) << 12 |
          (data[pos + 2] & 0b00111111) << 6 | (data[pos + 3] & 0b00111111);
      if (code_point < 0xffff || 0x10ffff < code_point) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else {
      // we may have a continuation
      return fastvalidate::error_code::UTF8_ERROR;
    }
    pos = next_pos;
  }
  return fastvalidate::error_code::SUCCESS;
}
fastvalidate::error_code fidl_validate_string_ascii(const unsigned char *data,
                                              uint64_t size) {
  uint64_t pos = 0;
  uint64_t next_pos = 0;
  uint32_t code_point = 0;
  while (pos < size) {

    // check of the next 8 bytes are ascii.
    next_pos = pos + 8;
    if (next_pos <= size) { // if it is safe to read 8 more bytes, check that they are ascii 
        uint64_t v;
        memcpy(&v, data + pos, sizeof(uint64_t));
        if((v & 0x8080808080808080)  == 0) {
          pos = next_pos;
          continue;
        }
    }
    unsigned char byte = data[pos];

    if (byte < 0b10000000) {
      pos++;
      continue;
    } else if ((byte & 0b11100000) == 0b11000000) {
      next_pos = pos + 2;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point = (byte & 0b00011111) << 6 | (data[pos + 1] & 0b00111111);
      if (code_point < 0x80 || 0x7ff < code_point) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else if ((byte & 0b11110000) == 0b11100000) {
      next_pos = pos + 3;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 2] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point = (byte & 0b00001111) << 12 |
                   (data[pos + 1] & 0b00111111) << 6 |
                   (data[pos + 2] & 0b00111111);
      if (code_point < 0x800 || 0xffff < code_point ||
          (0xd7ff < code_point && code_point < 0xe000)) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else if ((byte & 0b11111000) == 0b11110000) { // 0b11110000
      next_pos = pos + 4;
      if (next_pos > size) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 1] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 2] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      if ((data[pos + 3] & 0b11000000) != 0b10000000) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
      // range check
      code_point =
          (byte & 0b00000111) << 18 | (data[pos + 1] & 0b00111111) << 12 |
          (data[pos + 2] & 0b00111111) << 6 | (data[pos + 3] & 0b00111111);
      if (code_point < 0xffff || 0x10ffff < code_point) {
        return fastvalidate::error_code::UTF8_ERROR;
      }
    } else {
      // we may have a continuation
      return fastvalidate::error_code::UTF8_ERROR;
    }
    pos = next_pos;
  }
  return fastvalidate::error_code::SUCCESS;
}
