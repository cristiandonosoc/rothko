// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "rothko/utils/macros.h"

namespace rothko {

struct Color {
  // ABGR in memory.
  void operator=(uint32_t color) { *((uint32_t*)this) = color; }

  uint8_t r, g, b, a;
};
static_assert(sizeof(Color) == 4);

}  // namespace rothko
