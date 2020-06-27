#include "event_counter.h"
#include "fushia.h"
#include "hoehrmann.h"
#include "random_utf8.h"
#include "utf8.h"
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

#define OVERHEAD()                                                             \
  {                                                                            \
    event_collector collector;                                                 \
    event_aggregate all{};                                                     \
    for (size_t i = 0; i < repeat; i++) {                                      \
      collector.start();                                                       \
      event_count allocate_count = collector.end();                            \
      all << allocate_count;                                                   \
    }                                                                          \
    overhead_ins = all.best.instructions();                                    \
    overhead_time = all.best.elapsed_ns();                                     \
  }

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
    double freq = (all.best.cycles() / all.best.elapsed_sec()) / 1000000000.0; \
    double insperunit =                                                        \
        (all.best.instructions() - overhead_ins) / double(volume);             \
    double branchmissperunit = all.best.branch_misses() / double(volume);      \
    double means = all.total.elapsed_ns() / repeat;                            \
    double margin = (means - all.best.elapsed_ns()) / means;                   \
    double gbs = double(volume) / (all.best.elapsed_ns() - overhead_time);     \
    double inspercycle = all.best.instructions() / all.best.cycles();          \
    if (collector.has_events()) {                                              \
      printf("                               %8.3f ins/byte, %8.3f branch "    \
             "miss/kbyte,   %8.3f GHz, "                                       \
             "%8.3f GB/s (%5.2f %%) %8.3f ins/cycle \n",                        \
             insperunit, branchmissperunit * 1000, freq, gbs, margin,inspercycle);         \
    } else {                                                                   \
      printf("                               %8.3f GB/s  (%5.2f %%)\n", gbs,   \
             margin * 100);                                                    \
    }                                                                          \
  }
#define RUN(name, procedure)                                                   \
  printf("%40s ", name);                                                       \
  RUNINS(name, procedure)

std::vector<uint8_t> buffer;

class Benchmark {

  size_t size;
  std::random_device rd{};

public:
  Benchmark(const size_t tsize) : size(tsize) {}

  void run() {
    printf("\n");
    printf("Running UTF8 validation benchmark.\n");
    printf("The speed is normalized by the number of input bytes.\n");
    const size_t repeat = 1000;
    RandomUTF8 gen_1byte(rd, 1, 0, 0, 0);
    RandomUTF8 gen_2bytes(rd, 0, 1, 0, 0);
    RandomUTF8 gen_1_2(rd, 1, 1, 0, 0);
    RandomUTF8 gen_1_2_3(rd, 1, 1, 1, 0);
    RandomUTF8 gen_1_2_3_4(rd, 1, 1, 1, 1);
    RandomUTF8 gen_3bytes(rd, 0, 0, 1, 0);
    RandomUTF8 gen_4bytes(rd, 0, 0, 0, 1);
    printf("Input size: (UTF8) %lu\n", size);

    puts("- ASCII characters");
    run(gen_1byte, repeat);

    puts("- 1 or 2 UTF8 bytes");
    run(gen_1_2, repeat);

    puts("- 1, 2, 3 UTF8 bytes");
    run(gen_1_2_3, repeat);

    puts("- 1, 2, 3, 4 UTF8 bytes");
    run(gen_1_2_3_4, repeat);
  }

  void run(RandomUTF8 &generator, size_t repeat) {
    double overhead_ins{};
    double overhead_time{};
    OVERHEAD();
    printf("Overhead ==> nanoseconds : %.3f, instructions %.3f\n",
           overhead_time, overhead_ins);
    const auto UTF8 = generator.generate(size);
    size_t s{UTF8.size()};

    size_t volume = UTF8.size() * sizeof(UTF8[0]);
    buffer.resize(volume);

    auto mem = [&UTF8, &s]() {
      memcpy(buffer.data(), UTF8.data(), s);
      return fastvalidate::error_code::SUCCESS;
    };
    RUN("memcpy", mem);

    auto fushia = [&UTF8, &s]() {
      return fidl_validate_string(UTF8.data(), s);
    };
    RUN("fushia", fushia);

    auto fushia_ascii = [&UTF8, &s]() {
      return fidl_validate_string_ascii(UTF8.data(), s);
    };
    RUN("fushia_ascii", fushia_ascii);

    auto fushia_ascii2 = [&UTF8, &s]() {
      return fidl_validate_string_ascii2(UTF8.data(), s);
    };
    RUN("fushia_ascii2", fushia_ascii2);
    auto fushia_ascii4 = [&UTF8, &s]() {
      return fidl_validate_string_ascii4(UTF8.data(), s);
    };
    RUN("fushia_ascii4", fushia_ascii4);



    auto utf8lib = [&UTF8, &s]() {
      return utf8::is_valid(UTF8.begin(), UTF8.begin() + s)
                 ? fastvalidate::error_code::SUCCESS
                 : fastvalidate::error_code::UTF8_ERROR;
    };
    RUN("utf8lib", utf8lib);

    auto dfa = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8((const signed char *)UTF8.data(), s);
    };
    RUN("dfa", dfa);
    auto dfa2 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_double(
          (const signed char *)UTF8.data(), s);
    };
    RUN("dfa2", dfa2);

    auto dfa3 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_three((const signed char *)UTF8.data(),
                                               s);
    };
    RUN("dfa3", dfa3);
    auto dfa4 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_quad((const signed char *)UTF8.data(),
                                              s);
    };
    RUN("dfa4", dfa4);

    auto lookup2avx = [&UTF8, &s]() {
      return active_fastvalidate::lookup2::validate(UTF8.data(), s);
    };
#ifdef __x86_64__
    auto zwegneravx = [&UTF8, &s]() {
      return active_fastvalidate::zwegner::validate(UTF8.data(), s);
    };
    RUN("zwegneravx", zwegneravx);
#endif
    auto lookup3avx = [&UTF8, &s]() {
      return active_fastvalidate::lookup3::validate(UTF8.data(), s);
    };
    auto lookup4avx = [&UTF8, &s]() {
      return active_fastvalidate::lookup4::validate(UTF8.data(), s);
    };
    auto basicavx = [&UTF8, &s]() {
      return active_fastvalidate::basic::validate(UTF8.data(), s);
    };
    auto rangeavx = [&UTF8, &s]() {
      return active_fastvalidate::range::validate(UTF8.data(), s);
    };
    RUN("lookup2avx", lookup2avx);
    RUN("lookup3avx", lookup3avx);
    RUN("lookup4avx", lookup4avx);

    RUN("basicavx", basicavx);
    RUN("rangeavx", rangeavx);
  }
};

class RealDataBenchmark {

  std::vector<std::string> filenames = {"examples/hongkong.html",
                                        "examples/twitter.json"};

public:
  RealDataBenchmark() {}

  void run() {
    printf("\n");
    printf("Running UTF8 validation benchmark.\n");
    printf("The speed is normalized by the number of input bytes.\n");
    const size_t repeat = 1000;
    for (std::string filename : filenames) {
      std::ifstream in(filename);
      if (!in) {
        std::cerr << " I cannot load " << filename << std::endl;
        continue;
      }
      std::vector<char> utf8((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
      std::cout << "file: " << filename << " (" << utf8.size() / 1000 << "KB)"
                << std::endl;

      run(utf8, repeat);
    }
  }

  void run(std::vector<char> &UTF8, size_t repeat) {
    double overhead_ins{};
    double overhead_time{};
    OVERHEAD();
    printf("Overhead ==> nanoseconds : %.3f, instructions %.3f\n",
           overhead_time, overhead_ins);
    size_t s{UTF8.size()};

    size_t volume = UTF8.size() * sizeof(UTF8[0]);

    buffer.resize(volume);

    auto mem = [&UTF8, &s]() {
      memcpy(buffer.data(), UTF8.data(), s);
      return fastvalidate::error_code::SUCCESS;
    };
    RUN("memcpy", mem);

    auto fushia = [&UTF8, &s]() {
      return fidl_validate_string((const unsigned char *)UTF8.data(), s);
    };
    RUN("fushia", fushia);

    auto fushia_ascii = [&UTF8, &s]() {
      return fidl_validate_string_ascii((const unsigned char *)UTF8.data(), s);
    };
    RUN("fushia_ascii", fushia_ascii);

    auto fushia_ascii2 = [&UTF8, &s]() {
      return fidl_validate_string_ascii2((const unsigned char *)UTF8.data(), s);
    };
    RUN("fushia_ascii2", fushia_ascii2);

    auto fushia_ascii4 = [&UTF8, &s]() {
      return fidl_validate_string_ascii4((const unsigned char *)UTF8.data(), s);
    };
    RUN("fushia_ascii4", fushia_ascii4);


    auto utf8lib = [&UTF8, &s]() {
      return utf8::is_valid(UTF8.begin(), UTF8.begin() + s)
                 ? fastvalidate::error_code::SUCCESS
                 : fastvalidate::error_code::UTF8_ERROR;
    };
    RUN("utf8lib", utf8lib);

    auto dfa = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8((const signed char *)UTF8.data(), s);
    };
    RUN("dfa", dfa);
    auto dfa2 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_double(
          (const signed char *)UTF8.data(), s);
    };
    RUN("dfa2", dfa2);

    auto dfa3 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_three((const signed char *)UTF8.data(),
                                               s);
    };
    RUN("dfa3", dfa3);
    auto dfa4 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_quad((const signed char *)UTF8.data(),
                                              s);
    };
    RUN("dfa4", dfa4);

    auto lookup2avx = [&UTF8, &s]() {
      return active_fastvalidate::lookup2::validate(UTF8.data(), s);
    };
#ifdef __x86_64__
    auto zwegneravx = [&UTF8, &s]() {
      return active_fastvalidate::zwegner::validate(UTF8.data(), s);
    };
    RUN("zwegneravx", zwegneravx);
#endif
    auto lookup3avx = [&UTF8, &s]() {
      return active_fastvalidate::lookup3::validate(UTF8.data(), s);
    };
    auto lookup4avx = [&UTF8, &s]() {
      return active_fastvalidate::lookup4::validate(UTF8.data(), s);
    };
    auto basicavx = [&UTF8, &s]() {
      return active_fastvalidate::basic::validate(UTF8.data(), s);
    };
    auto rangeavx = [&UTF8, &s]() {
      return active_fastvalidate::range::validate(UTF8.data(), s);
    };
    RUN("lookup2avx", lookup2avx);
    RUN("lookup3avx", lookup3avx);
    RUN("lookup4avx", lookup4avx);

    RUN("basicavx", basicavx);
    RUN("rangeavx", rangeavx);
  }
};

int main() {
  RealDataBenchmark rdb;
  rdb.run();

  std::vector<size_t> input_size{16384};
  for (const size_t size : input_size) {
    Benchmark bench(size);
    bench.run();
  }
  return EXIT_SUCCESS;
}
