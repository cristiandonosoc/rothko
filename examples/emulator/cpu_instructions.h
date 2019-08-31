// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace rothko {
namespace emulator {

struct CPU;

struct Instruction {
  // NOTE(Cristian): Using a callback on each instruction is probably not the most efficient way to
  //                 go about this.
  using InstructionCallback = void(*)(CPU*, Instruction*);

  InstructionCallback* pre_execution = nullptr;
  InstructionCallback* post_execution = nullptr;

  // Most functions the ticks are only advanced when the instruction executes.
  // But for some functions (conditional jumps and some two-stage instructions), there are some
  // ticks that are run after the instruction.
  uint8_t pre_ticks = 0;
  uint8_t post_ticks = 0;

  uint8_t length;     // Length in bytes.

  // The actual bytes this instruction represents.
  // NOTE: CB instructions are two-bytes: the 0xcb prefix and then the "opcode".
  //       In our representation, the 0xcb prefix will be the first byte.
  union OpCode {
    uint16_t opcode;
    struct {
      uint8_t low;
      uint8_t high;
    };
  } opcode;
  static_assert(sizeof(Instruction::OpCode) == 2);
};
static_assert(sizeof(Instruction) == 24);

inline bool IsCBInstruction(const Instruction& i) { return i.opcode.low == 0xcb; }

void InitInstructions(CPU*);

// Conditional Ticks -------------------------------------------------------------------------------
//
// Used for comparing a condition (flag) to see how many more ticks an instruction should execute
// upon that condition being true.
// Exposed for testing purposes.

struct ConditionalTicks {
  uint8_t mask;
  uint8_t xor_comparator;
  uint8_t extra_ticks;
};

#define COND_TICKS_POSITIVE_FLAG(flag, ticks) {kCPUFlags##flag##Mask, kCPUFlags##flag##Mask, ticks}
#define COND_TICKS_NEGATIVE_FLAG(flag, ticks) {kCPUFlags##flag##Mask, 0, ticks}


}  // namespace emulator
}  // namespace rothko
