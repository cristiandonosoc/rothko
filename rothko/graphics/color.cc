// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/color.h"

namespace rothko {

Vec4 ToVec4(const Color& c) {
  Vec4 res;
  res.r = (float)c.r / 255.0f;
  res.g = (float)c.g / 255.0f;
  res.b = (float)c.b / 255.0f;
  res.a = (float)c.a / 255.0f;

  return res;
}

// Pre-defined colors ------------------------------------------------------------------------------

namespace colors {

Color kBlack{0, 0, 0};
Color kBlue{0, 0, 0xff};
Color kGreen{0, 0xff, 0};
Color kRed{0xff, 0, 0};
Color kWhite{0xff, 0xff, 0xff};
Color kLightGray{0x66, 0x66, 0x66};

Color kGray33{0x33, 0x33, 0x33};
Color kGray66{0x66, 0x66, 0x66};
Color kGray99{0x99, 0x99, 0x99};
Color kGraycc{0xcc, 0xcc, 0xcc};

}  // namespace colors
}  // namespace rothko
