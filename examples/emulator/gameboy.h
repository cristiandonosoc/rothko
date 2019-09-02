// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "audio.h"
#include "display.h"
#include "memory.h"

namespace rothko {
namespace emulator {

struct Gameboy {
  Audio audio;
  Memory memory;
  Display display;
};

}  // namespace emulator
}  // namespace rothko
