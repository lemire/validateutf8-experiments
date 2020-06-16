#include <cstdlib>
#include <string>
#include <fstream>
#include <streambuf>
#include "event_counter.h"
#include "random_utf8.h"
#include "fushia.h"
#include "hoehrmann.h"

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

#define RUNINS(name, procedure)                                                      \
  {                                                                            \
    event_collector collector;                                                 \
    event_aggregate all{};                                                     \
    for (size_t i = 0; i < repeat; i++) {                                      \
      collector.start();                                                       \
      if(procedure() != fastvalidate::error_code::SUCCESS) {printf("Bug %s\n",name);break;}                                                             \
      event_count allocate_count = collector.end();                            \
      all << allocate_count;                                                   \
    }                                                                          \
    double freq = (all.best.cycles() / all.best.elapsed_sec()) / 1000000000.0; \
    double insperunit = all.best.instructions() / double(volume);              \
    double branchmissperunit = all.best.branch_misses() / double(volume);              \
    double gbs = double(volume) / all.best.elapsed_ns();                       \
    if (collector.has_events()) {                                              \
      printf("                               %8.3f ins/byte, %8.3f %% branch miss/byte,   %8.3f GHz, "      \
             "%8.3f GB/s \n",                                                  \
             insperunit, branchmissperunit * 100 ,freq, gbs);                                           \
    } else {                                                                   \
      printf("                               %8.3f GB/s \n", gbs);             \
    }                                                                          \
  }
#define RUN(name, procedure)                                                   \
  printf("%40s ", name);\
  RUNINS(name,procedure)

class Benchmark {

  size_t size;
  std::random_device rd{};

public:
  Benchmark(const size_t tsize) : size(tsize) {}

  void run() {
    printf("\n");
    printf("Running UTF8 validation benchmark.\n");
    printf("The speed is normalized by the number of input bytes.\n");
    const size_t repeat = 10;
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

    const auto UTF8 = generator.generate(size);
    size_t s{UTF8.size()};

    size_t volume = UTF8.size() * sizeof(UTF8[0]);
    auto fushia = [&UTF8, &s]() {
      return fidl_validate_string(UTF8.data(), s);
    };
    RUN("fushia", fushia);


    auto dfa = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8((const signed char*) UTF8.data(), s);
    };
    RUN("dfa", dfa);
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
    auto basicavx = [&UTF8, &s]() {
      return active_fastvalidate::basic::validate(UTF8.data(), s);
    };
    auto rangeavx = [&UTF8, &s]() {
      return active_fastvalidate::range::validate(UTF8.data(), s);
    };
    RUN("lookup2avx", lookup2avx);
    RUN("lookup3avx", lookup3avx);
    RUN("basicavx", basicavx);
    RUN("rangeavx", rangeavx);
  }
};



class RealDataBenchmark {

  std::vector<std::string> filenames = {"examples/hongkong.html","examples/twitter.json"};



public:
  RealDataBenchmark() {}

  void run() {
    printf("\n");
    printf("Running UTF8 validation benchmark.\n");
    printf("The speed is normalized by the number of input bytes.\n");
    const size_t repeat = 10;
    for(std::string filename : filenames) {
      std::ifstream in(filename);
      if(!in) {
        std::cerr << " I cannot load " << filename << std::endl;
        continue;
      }
      std::vector<char> utf8((std::istreambuf_iterator<char>(in)),
                 std::istreambuf_iterator<char>());
      std::cout << "file: " << filename << " (" << utf8.size() / 1000 << "KB)" << std::endl;

      run(utf8,repeat);

    }
  }


  void run(std::vector<char>& UTF8, size_t repeat) {

    size_t s{UTF8.size()};

    size_t volume = UTF8.size() * sizeof(UTF8[0]);
    auto fushia = [&UTF8, &s]() {
      return fidl_validate_string((const unsigned char*)UTF8.data(), s);
    };
    RUN("fushia", fushia);


    auto dfa = [&UTF8, &s]() {
      return shiftless_validate_dfa_utf8((const signed char*) UTF8.data(), s);
    };
    RUN("dfa", dfa);
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
    auto basicavx = [&UTF8, &s]() {
      return active_fastvalidate::basic::validate(UTF8.data(), s);
    };
    auto rangeavx = [&UTF8, &s]() {
      return active_fastvalidate::range::validate(UTF8.data(), s);
    };
    RUN("lookup2avx", lookup2avx);
    RUN("lookup3avx", lookup3avx);
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
}
