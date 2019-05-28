// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

// Find first set: Find the index of the first set 1 within a uint64_t.
//                 Returns 0 if non is set.
#if defined(__GNUC__) || defined(__clang__)

inline int FindFirstSet(uint64_t x) {
  return __builtin_ffsl(x);
}

#elif defined(_MSC_VER)

#include <intrin.h>

inline int FindFirstSet(uint64_t x) {
  unsigned long index;
  char non_zero = _BitScanForward64(&index, x);
  return non_zero ? (int)index + 1 : 0;
}

#else

inline int FindFirstSet(uint64_t) {
  static_assert(false, __FUNCTION__ ": Not implemented for this compiler.");
}

#endif





