// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/graphics/common/color.h>

namespace rothko {
namespace emulator {

// |data| are 16 bytes, representing 64 pixels of 2 bits each.
// |out| must be able to support 8x8 pixels (64).
void TileToTexture(const uint8_t* data, Color* out);

}  // namespace emulator
}  // namespace rothko
