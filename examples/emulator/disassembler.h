// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include "cpu_instructions.h"

namespace rothko {
namespace emulator {

struct Memory;

struct Disassembler {
  Instruction instructions[64*1024];
};

void Disassemble(const Memory&, Disassembler*, uint16_t entry_point = 0x100);

// Returns the index of first valid instruction before the given |address|. -1 if none.
int PrevInstructionIndex(const Disassembler&, uint16_t address);

// Returns the index of first valid instruction after the given |address|. -1 if none.
int NextInstructionIndex(const Disassembler&, uint64_t address);

}  // namespace emulator
}  // namespace rothko
