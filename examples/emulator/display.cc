// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "display.h"

#include <rothko/logging/logging.h>

namespace rothko {
namespace emulator {

// Each pixels are defined by two bytes, where one is the "upper index" of the pixel and the second
// is the lower pixel:
//
// |7|6|5|4|3|2|1|0| Byte 1 (Least significant bit).
// |7|6|5|4|3|2|1|0| Byte 2 (Most significant bit).
//  | | | | | | | |
//  | | | | | | | |-> Pixel 0
//  | | | | | | |---> PiXel 1
//  | | | | | |-----> PiXel 2
//  | | | | |-------> PiXel 3
//  | | | |---------> PiXel 4
//  | | |-----------> PiXel 5
//  | |-------------> PiXel 6
//  |---------------> PiXel 7
void TileToTexture(const void* data, Color* out) {
  const uint8_t* ptr = (uint8_t*)data;
  for (int y = 0; y < 8; y++) {
    const uint8_t* lsb = ptr + 0;
    const uint8_t* msb = ptr + 1;

    // Bit 7 is the left-most pixel, so we iterate it backwards.
    for (int x = 7; x >= 0; x--) {
      uint8_t lsp = (*lsb >> x) & 0x1;
      uint8_t msp = (*msb >> x) & 0x1;
      uint8_t pixel = msp << 1 | lsp;
      ASSERT_MSG(pixel < 0b100, "Got pixel: 0x%x", pixel);

      switch (pixel) {
        case 0: *out = 0xffffffff; break;
        case 1: *out = 0xbbbbbbbb; break;
        case 2: *out = 0xff666666; break;
        case 3: *out = 0xff000000; break;
        default: NOT_REACHED();
      }
      out++;
    }

    ptr += 2;
  }
}

}  // namespace emulator
}  // namespace rothko
