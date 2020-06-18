#include <cstdlib>

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

#include "random_utf8.h"
#include "fushia.h"
#include "hoehrmann.h"



void display(const std::vector<uint8_t> & string) {
  printf("[");
  for(size_t i = 0; i < string.size(); i++) {
    printf("0x%02x", string[i]);
    if(i + 1 < string.size()) printf(",");
  }
  printf("]");
}

void brute_force_tests() {
  printf("running brute-force tests\n");
  std::random_device rd{};
  RandomUTF8 gen_1_2_3_4(rd, 1, 1, 1, 1);
  size_t total = 50000;
  for(size_t i = 0; i < total; i++) {
    if((i % (total/100)) == 0) {
      printf("\r\r\r\r%2.0f %%", double(i)/double(total)*100.0); fflush(NULL);
    }
    auto UTF8 = gen_1_2_3_4.generate(rand() % 256);
    if(active_fastvalidate::lookup2::validate(UTF8.data(), UTF8.size()) != fastvalidate::error_code::SUCCESS) {
        printf("bug brute_force_tests %.*s\n", (int) UTF8.size(), UTF8.data());
        display(UTF8);
        abort();
    }
    if(active_fastvalidate::lookup3::validate(UTF8.data(), UTF8.size()) != fastvalidate::error_code::SUCCESS) {
        printf("bug brute_force_tests %.*s\n", (int) UTF8.size(), UTF8.data());
        display(UTF8);
        abort();
    }
    if(shiftless_validate_dfa_utf8((const signed char *)UTF8.data(), UTF8.size()) != fastvalidate::error_code::SUCCESS) {
        printf("bug brute_force_tests dfa %.*s\n", (int) UTF8.size(), UTF8.data());
        display(UTF8);
        abort();
    }
    if(shiftless_validate_dfa_utf8_double((const signed char *)UTF8.data(), UTF8.size()) != fastvalidate::error_code::SUCCESS) {
        printf("bug brute_force_tests dfa double %.*s\n", (int) UTF8.size(), UTF8.data());
        display(UTF8);
        abort();
    }
    if(shiftless_validate_dfa_utf8_three((const signed char *)UTF8.data(), UTF8.size()) != fastvalidate::error_code::SUCCESS) {
        printf("bug brute_force_tests dfa three %.*s\n", (int) UTF8.size(), UTF8.data());
        display(UTF8);
        abort();
    }
    if(shiftless_validate_dfa_utf8_quad((const signed char *)UTF8.data(), UTF8.size()) != fastvalidate::error_code::SUCCESS) {
        printf("bug brute_force_tests dfa quad %.*s\n", (int) UTF8.size(), UTF8.data());
        display(UTF8);
        abort();
    }
    for(size_t flip = 0; flip  < 10000; ++flip) {
      // we are going to hack the string as long as it is UTF-8
      UTF8[rand()% UTF8.size()] ^= uint8_t(1) << (rand() % 8); // we flip exactly one bit
      auto e = fidl_validate_string(UTF8.data(), UTF8.size());      
      auto e3 = active_fastvalidate::lookup3::validate(UTF8.data(), UTF8.size());
      
      if(e3 != e) {
        printf("bug brute_force_tests lookup3 %.*s\n", (int) UTF8.size(), UTF8.data());
        if(e3 == fastvalidate::error_code::SUCCESS) {
          printf("lookup3 says it is ok!\n");
        } else {
          printf("lookup3 says it is not ok!\n");
        }
        display(UTF8);
        abort();
      }
      auto es = shiftless_validate_dfa_utf8((const signed char *)UTF8.data(), UTF8.size());
      
      if(es != e) {
        printf("bug brute_force_tests dfa %.*s\n", (int) UTF8.size(), UTF8.data());
        if(es == fastvalidate::error_code::SUCCESS) {
          printf("dfa says it is ok!\n");
        } else {
          printf("dfa says it is not ok!\n");
        }
        display(UTF8);
        abort();
      }
      auto esd = shiftless_validate_dfa_utf8_double((const signed char *)UTF8.data(), UTF8.size());
      
      if(esd != e) {
        printf("bug brute_force_tests dfa %.*s\n", (int) UTF8.size(), UTF8.data());
        if(esd == fastvalidate::error_code::SUCCESS) {
          printf("double dfa says it is ok!\n");
        } else {
          printf("double dfa says it is not ok!\n");
        }
        display(UTF8);
        abort();
      }
      auto esq = shiftless_validate_dfa_utf8_quad((const signed char *)UTF8.data(), UTF8.size());
      
      if(esq != e) {
        printf("bug brute_force_tests dfa %.*s\n", (int) UTF8.size(), UTF8.data());
        if(esq == fastvalidate::error_code::SUCCESS) {
          printf("quad dfa says it is ok!\n");
        } else {
          printf("quad dfa says it is not ok!\n");
        }
        display(UTF8);
        abort();
      }

      auto est = shiftless_validate_dfa_utf8_three((const signed char *)UTF8.data(), UTF8.size());
      
      if(est != e) {
        printf("bug brute_force_tests dfa %.*s\n", (int) UTF8.size(), UTF8.data());
        if(est == fastvalidate::error_code::SUCCESS) {
          printf("three dfa says it is ok!\n");
        } else {
          printf("three dfa says it is not ok!\n");
        }
        display(UTF8);
        abort();
      }

    }
  }
  printf("\r\r\r\r\n");
}

void twobytetest() {
  // this should not validate
  const char * twobyte = "\xcf\xcf\xcf";
  if(active_fastvalidate::lookup2::validate(twobyte, 3) != fastvalidate::error_code::UTF8_ERROR) {
        printf("twobytetest bug\n");
        abort();
  }
  if(active_fastvalidate::lookup3::validate(twobyte, 3) != fastvalidate::error_code::UTF8_ERROR) {
        printf("twobytetest bug\n");
        abort();
  }  
}

void test() {

  // additional tests are from autobahn websocket testsuite https://github.com/crossbario/autobahn-testsuite/tree/master/autobahntestsuite/autobahntestsuite/case
  const char *goodsequences[] = {"a", "\xc3\xb1", "\xe2\x82\xa1", "\xf0\x90\x8c\xbc", "안녕하세요, 세상",
                            "\xc2\x80", // 6.7.2
                            "\xf0\x90\x80\x80", // 6.7.4
                            "\xee\x80\x80" // 6.11.2
                            };
  const char *badsequences[] = {"\xc3\x28",         // 0
                          "\xa0\xa1",         // 1
                          "\xe2\x28\xa1",     // 2
                          "\xe2\x82\x28",     // 3
                          "\xf0\x28\x8c\xbc", // 4
                          "\xf0\x90\x28\xbc", // 5
                          "\xf0\x28\x8c\x28", // 6
                          "\xc0\x9f",         // 7
                          "\xf5\xff\xff\xff", // 8
                          "\xed\xa0\x81", // 9
                          "\xf8\x90\x80\x80\x80", //10
                          "123456789012345\xed", //11
                          "123456789012345\xf1", //12
                          "123456789012345\xc2", //13
                          "\xC2\x7F", // 14
                          "\xce", // 6.6.1
                          "\xce\xba\xe1", // 6.6.3
                          "\xce\xba\xe1\xbd", // 6.6.4
                          "\xce\xba\xe1\xbd\xb9\xcf", // 6.6.6
                          "\xce\xba\xe1\xbd\xb9\xcf\x83\xce", // 6.6.8
                          "\xce\xba\xe1\xbd\xb9\xcf\x83\xce\xbc\xce", // 6.6.10
                          "\xdf", // 6.14.6
                          "\xef\xbf", // 6.14.7
                          "\x80", 
                          "\x91\x85\x95\x9e",
                          "\x6c\x02\x8e\x18"
                        };
  for (size_t i = 0; i < 8; i++) {
    size_t len = strlen(goodsequences[i]);
    if(active_fastvalidate::lookup2::validate(goodsequences[i], len) != fastvalidate::error_code::SUCCESS) {
        printf("bug goodsequences[%zu]\n", i);
        abort();
    }
    if(active_fastvalidate::lookup3::validate(goodsequences[i], len) != fastvalidate::error_code::SUCCESS) {
        printf("bug goodsequences[%zu]\n", i);
        abort();
    }
  }
  for (size_t i = 0; i < 26; i++) {
    size_t len = strlen(badsequences[i]);
    if(active_fastvalidate::lookup2::validate(badsequences[i], len) != fastvalidate::error_code::UTF8_ERROR) {
        printf("bug lookup2 badsequences[%zu]\n", i);
        abort();
    }
    if(active_fastvalidate::lookup3::validate(badsequences[i], len) != fastvalidate::error_code::UTF8_ERROR) {
        printf("bug lookup3 badsequences[%zu]\n", i);
        abort();
    }

  }
  printf("tests ok.\n");
}
int main() {
  brute_force_tests();
  twobytetest();
  test();
  return EXIT_SUCCESS;
}
