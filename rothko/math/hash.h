// Copyright 2020, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "rothko/utils/macros.h"

namespace rothko {


// Call for running the hash on compile time.
#define HASH_STRING32(str) ::rothko::CompileTimeHash<FNA1a32Hash(str)>::kValue

#define HashString32(str) ::rothko::FNA1a32Hash(str)

// Template specialization to ensure compile-time evaluation.
template <uint32_t HASH>
struct CompileTimeHash {
  static constexpr uint32_t kValue = HASH;
};

// FNV-1a (32-bit) ---------------------------------------------------------------------------------

namespace {

constexpr uint32_t kFNV1a32Hash = 0x811c9dc5;
constexpr uint32_t kFNV1a32Prime = 0x1000193;

}  // namespace

inline constexpr uint32_t FNA1a32Hash(const char* str, const uint32_t value = kFNV1a32Hash) {
  if (str[0] == 0)
    return value;
  return FNA1a32Hash(str + 1, (uint32_t)((value ^ uint32_t(str[0])) * (uint64_t)kFNV1a32Prime));
}

}  // namespace rothko
