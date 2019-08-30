// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "rothko/math/math.h"
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

  // Pre-defined colors.
  static Color Transparent() { return Color{0, 0, 0, 0}; }

  static Color Black() { return Color{0, 0, 0}; }
  static Color Blue() { return Color{0, 0, 0xff}; }
  static Color Green() { return Color{0, 0xff, 0}; }
  static Color Red() { return Color{0xff, 0, 0}; }
  static Color White() { return Color{0xff, 0xff, 0xff}; }
  static Color LightGray() { return Color{0x66, 0x66, 0x66}; }
  static Color Gray33() { return Color{0x33, 0x33, 0x33}; }
  static Color Gray66() { return Color{0x66, 0x66, 0x66}; }
  static Color Gray99() { return Color{0x99, 0x99, 0x99}; }
  static Color Graycc() { return Color{0xcc, 0xcc, 0xcc}; }
};
static_assert(sizeof(Color) == 4);

inline uint32_t ToUint32(const Color& c) { return *(uint32_t*)&c; }

inline bool IsTransparent(const Color& c) { return c.a == 0; }

// |level| = 0 -> black. |level| = 0xff -> white.
inline Color CreateGray(uint8_t level) { return Color{level, level, level}; }

inline Vec4 ToVec4(const Color& c) {
  Vec4 res;
  res.r = (float)c.r / 255.0f;
  res.g = (float)c.g / 255.0f;
  res.b = (float)c.b / 255.0f;
  res.a = (float)c.a / 255.0f;

  return res;
}

}  // namespace rothko
