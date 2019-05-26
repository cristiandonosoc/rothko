// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// Find first set: Find the index of the first set 1 within a uint64_t.
//                 Returns 0 if non is set.
#if defined(__GNUC__) || defined(__clang__)

#define FIND_FIRST_SET(x) __builtin_ffsl(x)

#else

#define FIND_FIRST_SET(x)  \
  static_assert(false, "Find First Set not implemented in this compiler.");

#endif





