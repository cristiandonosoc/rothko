// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace rothko {
namespace emulator {

struct CPU;

struct Instruction {
  uint8_t ticks = 0;
  uint8_t length = 0;         // In bytes.
  union {
    uint8_t operands[2];   // Always filled in. Should be used correctly according to length.
    uint16_t operand;
  };

  // The actual bytes this instruction represents.
  // NOTE: CB instructions are two-bytes: the 0xcb prefix and then the "opcode".
  //       In our representation, the 0xcb prefix will be the first byte.
  union Opcode {
    uint16_t opcode;
    struct {
      uint8_t low;
      uint8_t high;
    };
  } opcode;
  static_assert(sizeof(Instruction::Opcode) == 2);
};
static_assert(sizeof(Instruction) == 6);

inline bool IsCBInstruction(const Instruction& i) { return i.opcode.high == 0xcb; }


const char* GetName(const Instruction& opcode);
const char* GetDescription(const Instruction& opcode);

// Conditional Ticks -------------------------------------------------------------------------------
//
// Used for comparing a condition (flag) to see how many more ticks an instruction should execute
// upon that condition being true.
//
// The ConditionalTicks masked bit has to be the same in |flags| in order to |ct.extra_ticks| to
// be returned. Returns 0 otherwise.

struct ConditionalTicks {
  uint8_t mask;
  uint8_t xnor_comparator;
  uint8_t extra_ticks;
};

#define COND_TICKS_POSITIVE_FLAG(flag, ticks) \
  { kCPUFlags##flag##Mask, kCPUFlags##flag##Mask, ticks }
#define COND_TICKS_NEGATIVE_FLAG(flag, ticks) {kCPUFlags##flag##Mask, 0, ticks}

uint8_t GetConditionalTicks(const Instruction&, uint8_t flags);

// Exposed for testing purposes.
uint8_t GetConditionalTicks(const ConditionalTicks& ct, uint8_t flags);

// We give all the possible data an instruction decoding might need, as instructions are at most
// three bytes long.
bool FetchAndDecode(Instruction*, const uint8_t data[3]);

}  // namespace emulator
}  // namespace rothko
