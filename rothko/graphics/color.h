// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "rothko/utils/macros.h"

namespace rothko {

struct Color {
  // ABGR in memory.
  Color() : r(0), g(0), b(0), a(0xff) {}
  Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) : r(r), g(g), b(b), a(a) {}
  Color(uint32_t color) { *((uint32_t*)this) = color; }

  DEFAULT_COPY_AND_ASSIGN(Color);
  DEFAULT_MOVE_AND_ASSIGN(Color);

  void operator=(uint32_t color) { *((uint32_t*)this) = color; }

  uint8_t r, g, b, a;
};
static_assert(sizeof(Color) == 4);

inline uint32_t ToUint32(const Color& c) { return *(uint32_t*)&c; }

// Premade colors ----------------------------------------------------------------------------------

namespace colors {

extern Color kBlack;
extern Color kBlue;
extern Color kGreen;
extern Color kRed;
extern Color kWhite;
extern Color kLightGray;

}  // namespace colors

}  // namespace rothko
