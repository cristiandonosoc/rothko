// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "audio.h"
#include "catridge.h"
#include "cpu.h"
#include "display.h"
#include "memory.h"

namespace rothko {
namespace emulator {

struct Gameboy {
  Audio audio;
  CPU cpu;
  Memory memory;
  Display display;
  MBCApi mbc;
};

inline bool Loaded(const Gameboy& gameboy) { return Valid(gameboy.mbc); }

void StepInstruction(Gameboy*);

// I/O ---------------------------------------------------------------------------------------------

inline void WriteByte(Gameboy* gameboy, uint16_t address, uint8_t value) {
  gameboy->mbc.WriteByte(gameboy, address, value);
}

inline void WriteShort(Gameboy* gameboy, uint16_t address, uint16_t value) {
  gameboy->mbc.WriteShort(gameboy, address, value);
}

NO_DISCARD inline uint8_t ReadByte(Gameboy* gameboy, uint16_t address) {
  return gameboy->mbc.ReadByte(gameboy, address);
}

NO_DISCARD inline uint16_t ReadShort(Gameboy* gameboy, uint16_t address) {
  return gameboy->mbc.ReadShort(gameboy, address);
}

}  // namespace emulator
}  // namespace rothko
