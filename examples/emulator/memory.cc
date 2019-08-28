// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "memory.h"

namespace rothko {
namespace emulator {

bool SpriteIsHidden(Memory* memory, const OAMEntry& sprite) {
  // |sprite.x| is
  if (sprite.x == 0 || sprite.x >= 168)
    return true;

  // Represents whether sprites are 8x8 or 8x16 pixels
  bool big_sprite = LCDC_OBJ_SPRITE_SIZE(memory->mapped_io.lcdc);
  uint8_t y_min_threshold = big_sprite ? 0 : 8;
  if (sprite.y <= y_min_threshold || sprite.y >= 168)
    return true;
  return false;
}

}  // namespace emulator
}  // namespace rothko

