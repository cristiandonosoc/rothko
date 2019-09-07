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

inline void WriteByte(Gameboy* gameboy, uint16_t address, uint8_t value) {
  gameboy->mbc.WriteByte(gameboy, address, value);
}

inline void WriteShort(Gameboy* gameboy, uint16_t address, uint16_t value) {
  gameboy->mbc.WriteShort(gameboy, address, value);
}

NO_DISCARD
inline uint8_t Read(Gameboy* gameboy, uint16_t address) {
  return gameboy->mbc.Read(gameboy, address);
}

}  // namespace emulator
}  // namespace rothko
