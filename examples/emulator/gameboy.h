// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "audio.h"
#include "cpu.h"
#include "display.h"
#include "memory.h"
#include "memory_bank_controllers.h"

namespace rothko {
namespace emulator {

struct Gameboy {
  Audio audio;
  CPU cpu;
  Memory memory;
  Display display;
  MBCApi mbc;
};

inline bool Valid(const Gameboy& gameboy) { return Valid(gameboy.mbc); }

}  // namespace emulator
}  // namespace rothko
