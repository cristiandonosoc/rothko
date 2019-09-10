// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "audio.h"
#include "catridge.h"
#include "cpu.h"
#include "disassembler.h"
#include "display.h"
#include "memory.h"
#include "textures.h"

namespace rothko {
namespace emulator {

struct Gameboy {
  Audio audio;
  Catridge catridge;
  CPU cpu;
  Disassembler disassembler;
  Display display;
  Memory memory;
  Textures textures;

  bool initialized = false;

  bool dump_loaded = false;
  bool catridge_loaded = false;
};
bool Init(Game* game, Gameboy*);

inline bool Valid(const Gameboy& gameboy) { return gameboy.initialized; }
inline bool Loaded(const Gameboy& gameboy) { return Valid(gameboy.catridge); }

void LoadCatridge(Gameboy* gameboy, Catridge&&);

void StepInstruction(Gameboy*);

// I/O ---------------------------------------------------------------------------------------------

inline void WriteByte(Gameboy* gameboy, uint16_t address, uint8_t value) {
  gameboy->catridge.mbc.WriteByte(gameboy, address, value);
}

inline void WriteShort(Gameboy* gameboy, uint16_t address, uint16_t value) {
  gameboy->catridge.mbc.WriteShort(gameboy, address, value);
}

NO_DISCARD inline uint8_t ReadByte(Gameboy* gameboy, uint16_t address) {
  return gameboy->catridge.mbc.ReadByte(gameboy, address);
}

NO_DISCARD inline uint16_t ReadShort(Gameboy* gameboy, uint16_t address) {
  return gameboy->catridge.mbc.ReadShort(gameboy, address);
}

}  // namespace emulator
}  // namespace rothko
