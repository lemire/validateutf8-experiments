#include "event_counter.h"
#include "fushia.h"
#include "hoehrmann.h"
#include "random_utf8.h"
#include <cstdlib>
#include <fstream>
#include <streambuf>
#include <string>

#ifdef __x86_64__
#include "avx2/implementations.h"
#include "sse/implementations.h"
namespace active_fastvalidate = fastvalidate::haswell;
#elif defined(__aarch64__)
#include "neon/implementations.h"
namespace active_fastvalidate = fastvalidate::arm64;
#else
#error "Unsupported platform"
#endif
#define RUNINS(name, procedure)                                                \
  {                                                                            \
    event_aggregate all{};                                                     \
    for (size_t i = 0; i < repeat; i++) {                                      \
      collector.start();                                                       \
      if (procedure() != fastvalidate::error_code::SUCCESS) {                  \
        printf("Bug %s\n", name);                                              \
        break;                                                                 \
      }                                                                        \
      event_count allocate_count = collector.end();                            \
      all << allocate_count;                                                   \
    }                                                                          \
    double insperunit = all.best.instructions() / double(volume);              \
    double branchmissperunit = all.best.branch_misses() / double(volume);      \
    double gbs = double(volume) / all.best.elapsed_ns();                       \
    if (collector.has_events()) {                                              \
      printf("     %8.3f  %8.3f %8.3f %8.3f", insperunit, branchmissperunit * 1000,  \
             gbs);                                                             \
    } else {                                                                   \
      printf("    %8.3f ", gbs);                                               \
    }                                                                          \
  }

#define RUNINSDIFF(name, procedure1, procedure2)                               \
  {                                                                            \
    event_aggregate all1{};                                                    \
    event_aggregate all2{};                                                    \
    for (size_t i = 0; i < repeat; i++) {                                      \
      event_count allocate_count;                                              \
      collector.start();                                                       \
      if (procedure1() != fastvalidate::error_code::SUCCESS) {                 \
        printf("Bug %s\n", name);                                              \
        break;                                                                 \
      }                                                                        \
      allocate_count = collector.end();                                        \
      all1 << allocate_count;                                                  \
      collector.start();                                                       \
      if (procedure2() != fastvalidate::error_code::SUCCESS) {                 \
        printf("Bug %s\n", name);                                              \
        break;                                                                 \
      }                                                                        \
      allocate_count = collector.end();                                        \
      all2 << allocate_count;                                                  \
    }                                                                          \
    double insperunit =                                                        \
        (all1.best.instructions() - all2.best.instructions()) /                \
        double(volume);                                                        \
    double branchmissperunit =                                                 \
        (all1.best.branch_misses() - all2.best.branch_misses()) /              \
        double(volume);                                                        \
    double gbs =                                                               \
        double(volume) / (all1.best.elapsed_ns() - all2.best.elapsed_ns());    \
    if (branchmissperunit < 0) {                                               \
      branchmissperunit = 0;                                                   \
    }                                                                          \
    if (collector.has_events()) {                                              \
      printf("     %8.3f  %8.3f %8.3f", insperunit, branchmissperunit * 1000,  \
             gbs);                                                             \
    } else {                                                                   \
      printf("    %8.3f ", gbs);                                               \
    }                                                                          \
  }

#define RUN(name, procedure) RUNINS(name, procedure)

#define RUNDIFF(name, procedure1, procedure2)                                  \
  RUNINSDIFF(name, procedure1, procedure2)

std::vector<uint8_t> buffer;

class Benchmark {

  size_t samples;
  size_t repeat;
  size_t min_size;
  size_t max_size;
  std::random_device rd{};

public:
  Benchmark(const size_t _samples, const size_t _repeat, const size_t _min_size,
            const size_t _max_size)
      : samples(_samples), repeat(_repeat), min_size(_min_size),
        max_size(_max_size) {}

  void run() {

    RandomUTF8 gen_1_2(rd, 1, 1, 0, 0);
    run(gen_1_2);
  }

  void run(RandomUTF8 &generator) {
    event_collector collector;
    for (size_t k = 0; k < samples; k++) {
      size_t size = size_t(
          double(k) / double(samples - 1) * (max_size - min_size) + min_size);
      const auto UTF8 = generator.generate(2 * size);
      size_t s1{UTF8.size()};
      size_t volume{size};
      size_t s2{UTF8.size() - volume};
      while (int8_t(UTF8[s2]) < 64) {
        s2 -= 1;
      }
      volume = s1 - s2;
      buffer.resize(s1);
      printf("%zu     ", volume);
      auto mem1 = [&UTF8, &s1]() {
        memcpy(buffer.data(), UTF8.data(), s1);
        return fastvalidate::error_code::SUCCESS;
      };
      auto mem2 = [&UTF8, &s2]() {
        memcpy(buffer.data(), UTF8.data(), s2);
        return fastvalidate::error_code::SUCCESS;
      };
      RUNDIFF("memcpy", mem1, mem2);

      auto fushia1 = [&UTF8, &s1]() {
        return fidl_validate_string(UTF8.data(), s1);
      };
      auto fushia2 = [&UTF8, &s2]() {
        return fidl_validate_string(UTF8.data(), s2);
      };

      RUNDIFF("fushia", fushia1, fushia2);
      auto dfa31 = [&UTF8, &s1]() {
        return shiftless_validate_dfa_utf8_three(
            (const signed char *)UTF8.data(), s1);
      };
      auto dfa32 = [&UTF8, &s2]() {
        return shiftless_validate_dfa_utf8_three(
            (const signed char *)UTF8.data(), s2);
      };
      RUNDIFF("dfa3", dfa31, dfa32);

      auto lookup3avx1 = [&UTF8, &s1]() {
        return active_fastvalidate::lookup3::validate(UTF8.data(), s1);
      };
      auto lookup3avx2 = [&UTF8, &s2]() {
        return active_fastvalidate::lookup3::validate(UTF8.data(), s2);
      };

      RUNDIFF("lookup3avx", lookup3avx1, lookup3avx2);

      auto fushia1_ascii = [&UTF8, &s1]() {
        return fidl_validate_string_ascii(UTF8.data(), s1);
      };
      auto fushia2_ascii = [&UTF8, &s2]() {
        return fidl_validate_string_ascii(UTF8.data(), s2);
      };

      RUNDIFF("fushia_ascii", fushia1_ascii, fushia2_ascii);

      printf("\n");
    }
  }
};

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("please provide a numerical parameter which indicates the number of "
           "repetitions\n");
    return EXIT_FAILURE;
  }

  size_t samples = 100;
  event_collector collector;
  if (collector.has_events()) {
    printf(
        "# columns match size in bytes then (ins/byte, branch miss /KB, GB/s, ins/cycle)");
  } else {
    printf("# columns match size in bytes then (GB/s)");
  }
  printf("# the validators are memcpy, branchy, dfa, lookup3, branchy/ascii\n");

  size_t repeat = atoll(argv[1]);
  printf("# number of repetitions %zu \n", repeat);

  Benchmark bench{samples, repeat, 1024, 65536};
  bench.run();

  return EXIT_SUCCESS;
}
