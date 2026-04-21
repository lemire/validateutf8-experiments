#ifndef FASTVALIDATE_TARGETS_H
#define FASTVALIDATE_TARGETS_H
#ifndef likely
#if defined(__GNUC__) || defined(__clang__)
#define likely(x) __builtin_expect(!!(x), 1)
#else
#define likely(x) (x)
#endif
#endif
#ifndef unlikely
#if defined(__GNUC__) || defined(__clang__)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define unlikely(x) (x)
#endif
#endif
namespace fastvalidate {
enum class error_code { SUCCESS, UTF8_ERROR };
}
// this is almost standard?
#undef STRINGIFY_IMPLEMENTATION_
#undef STRINGIFY
#define STRINGIFY_IMPLEMENTATION_(a) #a
#define STRINGIFY(a) STRINGIFY_IMPLEMENTATION_(a)
#if defined(_MSC_VER) && !defined(__clang__)
#define really_inline __forceinline
#else
#define really_inline inline __attribute__((always_inline, unused))
#endif
#ifdef __clang__
// clang does not have GCC push pop
// warning: clang attribute push can't be used within a namespace in clang up
// til 8.0 so TARGET_REGION and UNTARGET_REGION must be *outside* of a
// namespace.
#define TARGET_REGION(T)                                                       \
  _Pragma(STRINGIFY(                                                           \
      clang attribute push(__attribute__((target(T))), apply_to = function)))
#define UNTARGET_REGION _Pragma("clang attribute pop")
#elif defined(__GNUC__)
// GCC is easier
#define TARGET_REGION(T)                                                       \
  _Pragma("GCC push_options") _Pragma(STRINGIFY(GCC target(T)))
#define UNTARGET_REGION _Pragma("GCC pop_options")
#else
// MSVC has no per-function target pragma; rely on /arch:AVX2 at file scope.
#define TARGET_REGION(T)
#define UNTARGET_REGION
#endif // clang then gcc then MSVC

// under GCC and CLANG, we use these two macros
#define TARGET_HASWELL TARGET_REGION("avx2,bmi,lzcnt")
#define TARGET_WESTMERE TARGET_REGION("sse4.2")

#endif