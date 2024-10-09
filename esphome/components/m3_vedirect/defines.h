#pragma once

#include <stddef.h>
#include <string.h>

namespace esphome {
namespace m3_vedirect {

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef signed short int16_t;
typedef unsigned int u_int32_t;
typedef signed int int32_t;

// maximum amount of time (millis) without receiving data
// after which we consider the vedirect link disconnected
#define VEDIRECT_TIMEOUT_MILLIS 5000

// Helper to get the number of elements in static arrays
#define ARRAY_COUNT(_array) (sizeof(_array) / sizeof(_array[0]))

// Helpers for unordered_map with const char* key
#if __cpp_constexpr >= 201304L
#define _RELAXEDCONSTEXPR constexpr
#else
#define _RELAXEDCONSTEXPR
#endif
#define _HASH_SHIFT 16u
#define _HASH_MUL 23456789u

struct cstring_hash {
  _RELAXEDCONSTEXPR size_t operator()(const char *s) const {
    size_t h = 0;
    for (; *s; ++s) {
      h = h * _HASH_MUL + static_cast<unsigned char>(*s);
      h ^= h >> _HASH_SHIFT;
    }
    return h *= _HASH_MUL;
  }
};

struct cstring_eq {
  _RELAXEDCONSTEXPR
  bool operator()(const char *__x, const char *__y) const { return !strcmp(__x, __y); }
};

}  // namespace m3_vedirect
}  // namespace esphome