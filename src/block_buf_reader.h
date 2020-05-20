#ifndef FASTVALIDATE_BUF_BLOCK_READER_TARGETS_H
#define FASTVALIDATE_BUF_BLOCK_READER_TARGETS_H
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "targets.h"

namespace fastvalidate {
template<size_t STEP_SIZE>
struct buf_block_reader {
public:
  really_inline buf_block_reader(const uint8_t *_buf, size_t _len) : buf{_buf}, len{_len}, lenminusstep{len < STEP_SIZE ? 0 : len - STEP_SIZE}, idx{0} {}
  really_inline buf_block_reader(const char *_buf, size_t _len) : buf{reinterpret_cast<const uint8_t *>(_buf)}, len{_len}, lenminusstep{len < STEP_SIZE ? 0 : len - STEP_SIZE}, idx{0} {}
  really_inline size_t block_index() { return idx; }
  really_inline bool has_full_block() const {
    return idx < lenminusstep;
  }
  really_inline const uint8_t *full_block() const {
    return &buf[idx];
  }
  really_inline bool has_remainder() const {
    return idx < len;
  }
  really_inline void get_remainder(uint8_t *tmp_buf) const {
    memset(tmp_buf, 0x20, STEP_SIZE);
    memcpy(tmp_buf, buf + idx, len - idx);
  }
  really_inline void advance() {
    idx += STEP_SIZE;
  }
private:
  const uint8_t *buf;
  const ::size_t len;
  const ::size_t lenminusstep;
  ::size_t idx;
};
}
#endif