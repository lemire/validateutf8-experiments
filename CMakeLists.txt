cmake_minimum_required(VERSION 3.14)
project(validateutf8-experiments CXX)

include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _is_multi_config AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
      Debug Release RelWithDebInfo MinSizeRel)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  add_compile_options(-Wall -Wextra)
  check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
  if(COMPILER_SUPPORTS_MARCH_NATIVE)
    add_compile_options(-march=native)
  endif()
elseif(MSVC)
  # MSVC lacks per-function target attributes; enable AVX2 at file scope so
  # the AVX2/SSE intrinsic paths in src/avx2 and src/sse can compile.
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_compile_options(/arch:AVX2)
  endif()
endif()

unset(CPM_DIRECTORY CACHE)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake")

CPMAddPackage("gh:lemire/counters#v3.2.0")

# --- Targets ---------------------------------------------------------------
add_library(random_utf8 STATIC benchmarks/random_utf8.cpp)
target_include_directories(random_utf8 PUBLIC benchmarks)

add_executable(unit tests/unit.cpp)
target_include_directories(unit PRIVATE src benchmarks)
target_link_libraries(unit PRIVATE random_utf8)

add_executable(benchmark benchmarks/benchmark.cpp)
target_include_directories(benchmark PRIVATE src benchmarks)
target_include_directories(benchmark SYSTEM PRIVATE dependencies/utf8_v2_3_4/source)
target_link_libraries(benchmark PRIVATE random_utf8 counters::counters)

add_executable(benchstream benchmarks/benchstream.cpp)
target_include_directories(benchstream PRIVATE src benchmarks)
target_link_libraries(benchstream PRIVATE random_utf8 counters::counters)

enable_testing()
add_test(
  NAME unit
  COMMAND unit
)
set_tests_properties(unit PROPERTIES
  TIMEOUT 300
  FAIL_REGULAR_EXPRESSION "bug|Error|FAILED"
)
