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
                          "\xef\xbf" // 6.14.7
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
  for (size_t i = 0; i < 23; i++) {
    size_t len = strlen(badsequences[i]);
    if(active_fastvalidate::lookup2::validate(badsequences[i], len) != fastvalidate::error_code::UTF8_ERROR) {
        printf("bug badsequences[%zu]\n", i);
        abort();
    }
    if(active_fastvalidate::lookup3::validate(badsequences[i], len) != fastvalidate::error_code::UTF8_ERROR) {
        printf("bug badsequences[%zu]\n", i);
        abort();
    }

  }
  printf("tests ok.\n");
}
int main() {
  twobytetest();
  test();
  return EXIT_SUCCESS;
}
