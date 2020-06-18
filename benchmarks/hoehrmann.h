#ifndef HOEHRMANN_H
#define HOEHRMANN_H
// credit: @hoehrmann regarding the transcoding functions.

// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

// Note: The validation functions are original and inspired by
// the work by Hoehrmann.
#include <cstdint>

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1
#define SHIFTLESS_UTF8_REJECT 16

static const uint8_t utf8d[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 00..1f
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20..3f
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 40..5f
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60..7f
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   9,   9,   9,   9,   9,   9,
    9,   9,   9,   9,   9,   9,   9,   9,   9,   9, // 80..9f
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7, // a0..bf
    8,   8,   2,   2,   2,   2,   2,   2,   2,   2,   2,
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2, // c0..df
    0xa, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
    0x3, 0x3, 0x4, 0x3, 0x3, // e0..ef
    0xb, 0x6, 0x6, 0x6, 0x5, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8 // f0..ff
};
static const uint8_t utf8d_transition[] = {
    0x0, 0x1, 0x2, 0x3, 0x5, 0x8, 0x7, 0x1, 0x1, 0x1, 0x4,
    0x6, 0x1, 0x1, 0x1, 0x1, // s0..s0
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   0,   1,   1,   1,   1,
    1,   0,   1,   0,   1,   1,   1,   1,   1,   1, // s1..s2
    1,   2,   1,   1,   1,   1,   1,   2,   1,   2,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   2,   1,   1,   1,   1,   1,   1,   1,   1, // s3..s4
    1,   2,   1,   1,   1,   1,   1,   1,   1,   2,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   3,   1,   3,   1,   1,   1,   1,   1,   1, // s5..s6
    1,   3,   1,   1,   1,   1,   1,   3,   1,   3,   1,
    1,   1,   1,   1,   1,   1,   3,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, // s7..s8
};

static const uint8_t shifted_utf8d_transition[] = {
    0x0,  0x10, 0x20, 0x30, 0x50, 0x80, 0x70, 0x10, 0x10, 0x10, 0x40, 0x60,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0,  0x10, 0x10,
    0x10, 0x10, 0x10, 0x0,  0x10, 0x0,  0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x10, 0x20, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0x10, 0x30, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30,
    0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};

static uint32_t inline decode(uint32_t *state, uint32_t *codep, uint32_t byte) {
  uint32_t type = utf8d[byte];
  *codep = (*state != UTF8_ACCEPT) ? (byte & 0x3fu) | (*codep << 6)
                                   : (0xff >> type) & (byte);
  *state = utf8d_transition[16 * *state + type];
  return *state;
}

static uint32_t inline shiftless_decode(uint32_t *state, uint32_t *codep,
                                        uint32_t byte) {
  uint32_t type = utf8d[byte];
  *codep = (*state != UTF8_ACCEPT) ? (byte & 0x3fu) | (*codep << 6)
                                   : (0xff >> type) & (byte);
  *state = shifted_utf8d_transition[*state + type];
  return *state;
}

static uint32_t inline updatestate(uint32_t *state, uint32_t byte) {
  uint32_t type = utf8d[byte];
  *state = utf8d_transition[16 * *state + type];
  return *state;
}

static uint32_t inline shiftless_updatestate(uint32_t *state, uint32_t byte) {
  uint32_t type = utf8d[byte];
  *state = shifted_utf8d_transition[*state + type];
  return *state;
}

static inline fastvalidate::error_code  is_ascii(const signed char *c, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (c[i] < 0)
      return fastvalidate::error_code::UTF8_ERROR;
  }
  return fastvalidate::error_code::SUCCESS;
}


static inline fastvalidate::error_code  validate_dfa_utf8(const signed char *c, size_t len) {
  const unsigned char *cu = (const unsigned char *)c;
  uint32_t state = 0;
  for (size_t i = 0; i < len; i++) {
    uint32_t byteval = (uint32_t)cu[i];
    if (updatestate(&state, byteval) == UTF8_REJECT)
      return fastvalidate::error_code::UTF8_ERROR;
  }
  return fastvalidate::error_code::SUCCESS;
}

static inline fastvalidate::error_code  shiftless_validate_dfa_utf8(const signed char *c, size_t len) {
  const unsigned char *cu = (const unsigned char *)c;
  uint32_t state = 0;
  for (size_t i = 0; i < len; i++) {
    uint32_t byteval = (uint32_t)cu[i];
    if (shiftless_updatestate(&state, byteval) == SHIFTLESS_UTF8_REJECT)
      return fastvalidate::error_code::UTF8_ERROR;
  }
  if (shiftless_updatestate(&state, 0) == SHIFTLESS_UTF8_REJECT)
    return fastvalidate::error_code::UTF8_ERROR;
  return fastvalidate::error_code::SUCCESS;
}

static inline bool is_leading(int8_t c) {
  return c >= -64; // this covers ASCII + other leading bytes
}

static inline fastvalidate::error_code  shiftless_validate_dfa_utf8_double(const signed char *c, size_t len) {
  if(len < 32) {// if too small, do not bother
    return shiftless_validate_dfa_utf8(c, len);
  }
  // go to the middle:
  size_t middle_point = len / 2;
  // backtrack until we find a leading byte
  while(!is_leading(c[middle_point])) {
    if(middle_point == 0) {
      // we have determined that there is an error
      return fastvalidate::error_code::UTF8_ERROR;
    } 
    middle_point -= 1; 
  }
  uint32_t state1 = 0;
  uint32_t state2 = 0;
  const unsigned char *cu1 = (const unsigned char *)c;
  const unsigned char *cu2 = (const unsigned char *)(c + middle_point);

  for (size_t i = 0; i < middle_point; i++) {
    uint32_t byteval1 = (uint32_t)cu1[i];
    uint32_t byteval2 = (uint32_t)cu2[i];
    if ((shiftless_updatestate(&state1, byteval1) == SHIFTLESS_UTF8_REJECT) 
       || (shiftless_updatestate(&state2, byteval2) == SHIFTLESS_UTF8_REJECT)) {
      return fastvalidate::error_code::UTF8_ERROR;
    }
  }
  if(shiftless_updatestate(&state1, 0) == SHIFTLESS_UTF8_REJECT) {
    return fastvalidate::error_code::UTF8_ERROR;
  }
  for (size_t i = middle_point; i < len - middle_point; i++) {
    uint32_t byteval2 = (uint32_t)cu2[i];
    if (shiftless_updatestate(&state2, byteval2) == SHIFTLESS_UTF8_REJECT) {
      return fastvalidate::error_code::UTF8_ERROR;
    }
  }
  if(shiftless_updatestate(&state2, 0) == SHIFTLESS_UTF8_REJECT) {
    return fastvalidate::error_code::UTF8_ERROR;
  }
  return fastvalidate::error_code::SUCCESS;
}
#endif
