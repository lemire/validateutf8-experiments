#include "counters/bench.h"
#include "counters/event_counter.h"
#include "fushia.h"
#include "hoehrmann.h"
#include "random_utf8.h"
#include "utf8.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
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

static const bool kHasEvents = counters::has_performance_counters();
static volatile int sink = 0;
static const char *kFilter = nullptr;

static void print_section(const char *title) {
  printf("\n== %s ==\n", title);
}

static void print_table_header() {
  if (kHasEvents) {
    printf("%-18s | %8s | %10s | %6s | %7s | %7s | %7s\n",
           "name", "ins/byte", "br.miss/KB", "GHz", "GB/s", "margin%", "ins/cyc");
    printf("-------------------+----------+------------+--------+---------+"
           "---------+--------\n");
  } else {
    printf("%-18s | %8s | %7s\n", "name", "GB/s", "margin%");
    printf("-------------------+----------+--------\n");
  }
}

template <typename Func>
static void run_one(const char *name, size_t volume, Func fn) {
  if (kFilter && !strstr(name, kFilter)) return;
  if (fn() != fastvalidate::error_code::SUCCESS) {
    printf("%-18s | Bug\n", name);
    return;
  }
  auto agg = counters::bench_impl<1>([&] { sink = int(fn()); },
                                     /*min_repeat=*/10,
                                     /*min_time_ns=*/400'000'000,
                                     /*max_repeat=*/1'000'000);
  const double best_ns = agg.fastest_elapsed_ns();
  const double mean_ns = agg.elapsed_ns();
  const double gbs = double(volume) / best_ns;
  const double margin = (mean_ns - best_ns) / mean_ns * 100.0;
  if (kHasEvents) {
    const double ins_per_byte = agg.fastest_instructions() / double(volume);
    const double brmiss_per_kb = agg.fastest_branch_misses() * 1000.0 / double(volume);
    const double freq_ghz = agg.fastest_cycles() / best_ns;
    const double ins_per_cycle = agg.fastest_instructions() / agg.fastest_cycles();
    printf("%-18s | %8.3f | %10.3f | %6.3f | %7.3f | %7.2f | %7.3f\n",
           name, ins_per_byte, brmiss_per_kb, freq_ghz, gbs, margin,
           ins_per_cycle);
  } else {
    printf("%-18s | %8.3f | %7.2f\n", name, gbs, margin);
  }
}

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
    printf("Input size: %lu bytes\n", size);
    RandomUTF8 gen_1byte(rd, 1, 0, 0, 0);
    RandomUTF8 gen_2bytes(rd, 0, 1, 0, 0);
    RandomUTF8 gen_1_2(rd, 1, 1, 0, 0);
    RandomUTF8 gen_1_2_3(rd, 1, 1, 1, 0);
    RandomUTF8 gen_1_2_3_4(rd, 1, 1, 1, 1);
    RandomUTF8 gen_3bytes(rd, 0, 0, 1, 0);
    RandomUTF8 gen_4bytes(rd, 0, 0, 0, 1);

    print_section("ASCII characters");
    run(gen_1byte);

    print_section("1 or 2 UTF8 bytes");
    run(gen_1_2);

    print_section("1, 2, 3 UTF8 bytes");
    run(gen_1_2_3);

    print_section("1, 2, 3, 4 UTF8 bytes");
    run(gen_1_2_3_4);
  }

  void run(RandomUTF8 &generator) {
    print_table_header();
    const auto UTF8 = generator.generate(size);
    size_t s{UTF8.size()};

    size_t volume = UTF8.size() * sizeof(UTF8[0]);
    buffer.resize(volume);

    auto mem = [&UTF8, &s]() {
      memcpy(buffer.data(), UTF8.data(), s);
      return fastvalidate::error_code::SUCCESS;
    };
    run_one("memcpy", volume, mem);

    auto fushia = [&UTF8, &s]() {
      return fidl_validate_string(UTF8.data(), s);
    };
    run_one("fushia", volume, fushia);

    auto fushia_ascii = [&UTF8, &s]() {
      return fidl_validate_string_ascii(UTF8.data(), s);
    };
    run_one("fushia_ascii", volume, fushia_ascii);

    auto fushia_ascii2 = [&UTF8, &s]() {
      return fidl_validate_string_ascii2(UTF8.data(), s);
    };
    run_one("fushia_ascii2", volume, fushia_ascii2);
    auto fushia_ascii4 = [&UTF8, &s]() {
      return fidl_validate_string_ascii4(UTF8.data(), s);
    };
    run_one("fushia_ascii4", volume, fushia_ascii4);



    auto utf8lib = [&UTF8, &s]() {
      return utf8::is_valid(UTF8.begin(), UTF8.begin() + s)
                 ? fastvalidate::error_code::SUCCESS
                 : fastvalidate::error_code::UTF8_ERROR;
    };
    run_one("utf8lib", volume, utf8lib);

    auto dfa = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8((const signed char *)UTF8.data(), s);
    };
    run_one("dfa", volume, dfa);
    auto fdfa = [&UTF8, &s]() {
      return fast_validate_dfa_utf8((const signed char *)UTF8.data(), s);
    };
    run_one("fdfa", volume, fdfa);
    auto bdfa = [&UTF8, &s]() {
      return big_validate_dfa_utf8((const signed char *)UTF8.data(), s);
    };
    run_one("bdfa", volume, bdfa);
    auto dfa2 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_double(
          (const signed char *)UTF8.data(), s);
    };

    run_one("dfa2", volume, dfa2);

    auto dfa3 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_three((const signed char *)UTF8.data(),
                                               s);
    };
    run_one("dfa3", volume, dfa3);
    auto dfa4 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_quad((const signed char *)UTF8.data(),
                                              s);
    };
    run_one("dfa4", volume, dfa4);

    auto lookup2avx = [&UTF8, &s]() {
      return active_fastvalidate::lookup2::validate(UTF8.data(), s);
    };
#ifdef __x86_64__
    auto zwegneravx = [&UTF8, &s]() {
      return active_fastvalidate::zwegner::validate(UTF8.data(), s);
    };
    run_one("zwegneravx", volume, zwegneravx);
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
    run_one("lookup2avx", volume, lookup2avx);
    run_one("lookup3avx", volume, lookup3avx);
    run_one("lookup4avx", volume, lookup4avx);

    run_one("basicavx", volume, basicavx);
    run_one("rangeavx", volume, rangeavx);
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
    for (std::string filename : filenames) {
      std::ifstream in(filename);
      if (!in) {
        std::cerr << " I cannot load " << filename << std::endl;
        continue;
      }
      std::vector<char> utf8((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
      char title[256];
      snprintf(title, sizeof(title), "file: %s (%zu KB)", filename.c_str(),
               utf8.size() / 1000);
      print_section(title);

      run(utf8);
    }
  }

  void run(std::vector<char> &UTF8) {
    print_table_header();
    size_t s{UTF8.size()};

    size_t volume = UTF8.size() * sizeof(UTF8[0]);

    buffer.resize(volume);

    auto mem = [&UTF8, &s]() {
      memcpy(buffer.data(), UTF8.data(), s);
      return fastvalidate::error_code::SUCCESS;
    };
    run_one("memcpy", volume, mem);

    auto fushia = [&UTF8, &s]() {
      return fidl_validate_string((const unsigned char *)UTF8.data(), s);
    };
    run_one("fushia", volume, fushia);

    auto fushia_ascii = [&UTF8, &s]() {
      return fidl_validate_string_ascii((const unsigned char *)UTF8.data(), s);
    };
    run_one("fushia_ascii", volume, fushia_ascii);

    auto fushia_ascii2 = [&UTF8, &s]() {
      return fidl_validate_string_ascii2((const unsigned char *)UTF8.data(), s);
    };
    run_one("fushia_ascii2", volume, fushia_ascii2);

    auto fushia_ascii4 = [&UTF8, &s]() {
      return fidl_validate_string_ascii4((const unsigned char *)UTF8.data(), s);
    };
    run_one("fushia_ascii4", volume, fushia_ascii4);


    auto utf8lib = [&UTF8, &s]() {
      return utf8::is_valid(UTF8.begin(), UTF8.begin() + s)
                 ? fastvalidate::error_code::SUCCESS
                 : fastvalidate::error_code::UTF8_ERROR;
    };
    run_one("utf8lib", volume, utf8lib);

    auto dfa = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8((const signed char *)UTF8.data(), s);
    };
    run_one("dfa", volume, dfa);
    auto fdfa = [&UTF8, &s]() {
      return fast_validate_dfa_utf8((const signed char *)UTF8.data(), s);
    };
    run_one("fdfa", volume, fdfa);
    auto bdfa = [&UTF8, &s]() {
      return big_validate_dfa_utf8((const signed char *)UTF8.data(), s);
    };
    run_one("bdfa", volume, bdfa);
    auto dfa2 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_double(
          (const signed char *)UTF8.data(), s);
    };
    run_one("dfa2", volume, dfa2);

    auto dfa3 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_three((const signed char *)UTF8.data(),
                                               s);
    };
    run_one("dfa3", volume, dfa3);
    auto dfa4 = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8_quad((const signed char *)UTF8.data(),
                                              s);
    };
    run_one("dfa4", volume, dfa4);

    auto lookup2avx = [&UTF8, &s]() {
      return active_fastvalidate::lookup2::validate(UTF8.data(), s);
    };
#ifdef __x86_64__
    auto zwegneravx = [&UTF8, &s]() {
      return active_fastvalidate::zwegner::validate(UTF8.data(), s);
    };
    run_one("zwegneravx", volume, zwegneravx);
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
    run_one("lookup2", volume, lookup2avx);
    run_one("lookup3", volume, lookup3avx);
    run_one("lookup4", volume, lookup4avx);

    run_one("basic", volume, basicavx);
    run_one("range", volume, rangeavx);
  }
};

static void print_usage(const char *prog) {
  fprintf(stderr,
          "usage: %s [--filter=<substring>]\n"
          "  --filter=<s>   only run validators whose name contains <s>\n",
          prog);
}

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    const char *a = argv[i];
    if (strncmp(a, "--filter=", 9) == 0) {
      kFilter = a + 9;
    } else if (strcmp(a, "--filter") == 0 && i + 1 < argc) {
      kFilter = argv[++i];
    } else if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
      print_usage(argv[0]);
      return EXIT_SUCCESS;
    } else {
      fprintf(stderr, "unknown argument: %s\n", a);
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }
  }
  if (kFilter) {
    printf("filter: only running validators matching \"%s\"\n", kFilter);
  }

  RealDataBenchmark rdb;
  rdb.run();

  std::vector<size_t> input_size{16384};
  for (const size_t size : input_size) {
    Benchmark bench(size);
    bench.run();
  }
  return EXIT_SUCCESS;
}
