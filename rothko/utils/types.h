// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace rothko {

constexpr uint64_t kKilobyte = 1024;
constexpr uint64_t kMegabyte = 1024 * 1024;
constexpr uint64_t kGigabyte = 1024 * 1024 * 1024;

#define KILOBYTES(count) (count * kKilobyte)
#define MEGABYTES(count) (count * kMegabyte)
#define GIGABYTES(count) (count * kGigabyte)

inline float ToKilobytes(uint64_t bytes) { return (float)bytes / kKilobyte; }
inline float ToMegabytes(uint64_t bytes) { return (float)bytes / kMegabyte; }
inline float ToGigabytes(uint64_t bytes) { return (float)bytes / kGigabyte; }

}  // namespace rothko
