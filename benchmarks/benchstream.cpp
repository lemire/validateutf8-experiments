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
    event_collector collector;                                                 \
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
      printf("     %8.3f  %8.3f %8.3f", insperunit, branchmissperunit * 1000,  \
             gbs);                                                             \
    } else {                                                                   \
      printf("    %8.3f ", gbs);                                               \
    }                                                                          \
  }
#define RUN(name, procedure) RUNINS(name, procedure)

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

    for (size_t k = 0; k < samples; k++) {
      size_t size = size_t(
          double(k + 1) / double(samples) * (max_size - min_size) + min_size);
      const auto UTF8 = generator.generate(size);
      size_t s{UTF8.size()};
      size_t volume{s};
      buffer.resize(s);
      printf("%zu     ", s);
      auto mem = [&UTF8, &s]() {
        memcpy(buffer.data(), UTF8.data(), s);
        return fastvalidate::error_code::SUCCESS;
      };
      RUN("memcpy", mem);

      auto fushia = [&UTF8, &s]() {
        return fidl_validate_string(UTF8.data(), s);
      };
      RUN("fushia", fushia);
      auto dfa3 = [&UTF8, &s]() {
        return shiftless_validate_dfa_utf8_three(
            (const signed char *)UTF8.data(), s);
      };
      RUN("dfa3", dfa3);
      auto lookup3avx = [&UTF8, &s]() {
        return active_fastvalidate::lookup3::validate(UTF8.data(), s);
      };
      RUN("lookup3avx", lookup3avx);
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
        "# columns match size in bytes then (ins/byte, branch miss /kb, GB/s)");
  } else {
    printf("# columns match size in bytes then (GB/s)");
  }
  printf("# the validators are memcpy, branchy, dfa, lookup3\n");

  size_t repeat = atoll(argv[1]);
  printf("# number of repetitions %zu \n", repeat);

  Benchmark bench{samples, repeat, 0, 65536};
  bench.run();

  return EXIT_SUCCESS;
}
