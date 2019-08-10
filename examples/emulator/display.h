// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/graphics/common/color.h>
#include <rothko/logging/logging.h>

#include "memory.h"

namespace rothko {
namespace emulator {

// Transforms a GB shade (defined in a palette) to a Rothko Color. shades are 2 bits.
inline Color ShadeToColor(uint32_t shade) {
  switch (shade) {
    case 0: return Color{0xffffffff};   // White.
    case 1: return Color{0xbbbbbbbb};   // Light gray.
    case 2: return Color{0xff666666};   // Dark gray.
    case 3: return Color{0xff000000};   // Black.
    default: break;
  }

  NOT_REACHED();
  return {};
}

// |palette| is a palette register (bgp, obp0, obp1).
// |data| are 16 bytes, representing 64 pixels of 2 bits each.
// |out| must be able to support 8x8 pixels (64).
void TileToTexture(uint8_t palette, const void* data, Color* out);

}  // namespace emulator
}  // namespace rothko