// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "gameboy.h"

namespace rothko {
namespace emulator {

namespace {

void Reset(Gameboy* gameboy) {
  *gameboy = {};
}

}  // namespace

bool Init(Game* game, Gameboy* gameboy) {
  if (!Init(game, &gameboy->textures))
    return false;

  return true;
}

// LoadCatridge ----------------------------------------------------------------------------------


void LoadCatridge(Gameboy* gameboy, Catridge&& catridge) {
  Reset(gameboy);
  gameboy->catridge = std::move(catridge);

  // Set the initial CPU values.
  // TODO(Cristian): This is only for GB for now.
  gameboy->cpu.registers.af = 0xb001;
  gameboy->cpu.registers.bc = 0x0013;
  gameboy->cpu.registers.de = 0x00d8;
  gameboy->cpu.registers.hl = 0x014d;

  gameboy->cpu.registers.pc = 0x0100;
  gameboy->cpu.registers.pc = 0xfffe;

  // Set the initial memory values.
  uint8_t* mem_ptr = (uint8_t*)&gameboy->memory;
  mem_ptr[0xff04] = 0xab;  // DIV
  mem_ptr[0xff05] = 0x00;  // TIMA
  mem_ptr[0xff06] = 0x00;  // TMA
  mem_ptr[0xff07] = 0x00;  // TAC
  mem_ptr[0xff10] = 0x80;  // NR10
  mem_ptr[0xff11] = 0xbf;  // NR11
  mem_ptr[0xff12] = 0xf3;  // NR12
  mem_ptr[0xff14] = 0xbf;  // NR14
  mem_ptr[0xff16] = 0x3f;  // NR21
  mem_ptr[0xff17] = 0x00;  // NR22
  mem_ptr[0xff19] = 0xbf;  // NR24
  mem_ptr[0xff1a] = 0x7f;  // NR30
  mem_ptr[0xff1b] = 0xff;  // NR31
  mem_ptr[0xff1c] = 0x9f;  // NR32
  mem_ptr[0xff1e] = 0xbf;  // NR33
  mem_ptr[0xff20] = 0xff;  // NR41
  mem_ptr[0xff21] = 0x00;  // NR42
  mem_ptr[0xff22] = 0x00;  // NR43
  mem_ptr[0xff23] = 0xbf;  // NR30
  mem_ptr[0xff24] = 0x77;  // NR50
  mem_ptr[0xff25] = 0xf3;  // NR51
  mem_ptr[0xff26] = 0xf1;  // NR52 GB: 0xf1, SGB: 0xf0
  mem_ptr[0xff40] = 0x91;  // LCDC
  mem_ptr[0xff42] = 0x00;  // SCY
  mem_ptr[0xff43] = 0x00;  // SCX
  mem_ptr[0xff45] = 0x00;  // LYC
  mem_ptr[0xff47] = 0xfc;  // BGP
  mem_ptr[0xff48] = 0xff;  // OBP0
  mem_ptr[0xff49] = 0xff;  // OBP1
  mem_ptr[0xff4a] = 0x00;  // WY
  mem_ptr[0xff4b] = 0x00;  // WX
  mem_ptr[0xffff] = 0x00;  // IE
}

void StepInstruction(Gameboy*);

}  // namespace emulator
}  // namespace rothko
