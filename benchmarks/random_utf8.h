#pragma once

#include <cstdint>
#include <cstddef>
#include <random>

class RandomUTF8 final {
public:
    RandomUTF8(std::random_device& rd,
                int prob_1byte,
                int prob_2bytes,
                int prob_3bytes,
                int prob_4bytes);

    std::vector<uint8_t> generate(size_t output_bytes);
    std::vector<uint8_t> generate(size_t output_bytes, long seed);
private:
    uint32_t generate();

    std::mt19937 gen;
    std::discrete_distribution<> bytes_count;
    std::uniform_int_distribution<uint8_t> val_7bit{0x00, 0x7f}; // 0b0xxxxxxx
    std::uniform_int_distribution<uint8_t> val_6bit{0x00, 0x3f}; // 0b10xxxxxx
    std::uniform_int_distribution<uint8_t> val_5bit{0x00, 0x1f}; // 0b110xxxxx
    std::uniform_int_distribution<uint8_t> val_4bit{0x00, 0x0f}; // 0b1110xxxx
    std::uniform_int_distribution<uint8_t> val_3bit{0x00, 0x07}; // 0b11110xxx
};
