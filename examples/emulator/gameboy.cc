// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "gameboy.h"

namespace rothko {
namespace emulator {

bool Init(Game* game, Gameboy* gameboy) {
  if (!Init(game, &gameboy->textures))
    return false;

  return true;
}


void StepInstruction(Gameboy*);

}  // namespace emulator
}  // namespace rothko
