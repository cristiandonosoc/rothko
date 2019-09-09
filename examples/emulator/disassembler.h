// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <map>

#include "cpu_instructions.h"

namespace rothko {
namespace emulator {

struct Memory;

struct DisassembledInstruction {
  uint16_t address = 0;
  Instruction instruction;
};

struct Disassembler {
  std::map<uint64_t, DisassembledInstruction> instructions;
};
inline bool Valid(const Disassembler& d) { return !d.instructions.empty(); }

void Disassemble(const Memory&, Disassembler*, uint16_t entry_point = 0x100);

}  // namespace emulator
}  // namespace rothko
