// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

namespace rothko {
namespace emulator {

// Registers ---------------------------------------------------------------------------------------

struct CPURegisters {
  // All these registers can be accessed as a 16-bit value or as two separate 8-bit value:
  // bc = 16-bit. b = top 8-bits. c = bottom 8-bit.
  uint16_t af;  // Accumulator / Flags.
  uint16_t bc;  // b and c registers.
  uint16_t de;  // d and e registers.
  uint16_t hl;  // h and l registers.

  uint16_t pc;  // Program counter.
  uint16_t sp;  // Stack pointer.
} registers;

std::string ToString(const CPURegisters&);

inline uint8_t GetTopRegister(uint16_t reg) { return (reg >> 8); }
inline uint8_t GetBottomRegister(uint16_t reg) { return reg & 0xff; }

inline uint8_t GetBit(uint8_t reg, int bit) { return (reg >> bit) & 0b1; }
inline uint8_t SetBit(uint8_t reg, int bit) { return reg | (0b1 << bit); }
inline uint8_t ClearBit(uint8_t reg, int bit) { return reg & ~(0b1 << bit); }

#define CPU_GET_A(cpu)          GetTopRegister(cpu.registers.af)
#define CPU_GET_F(cpu)          GetBottomRegister(cpu.registers.af)
#define CPU_GET_FLAGS(cpu)      CPU_GET_F(cpu)
#define CPU_GET_B(cpu)          GetTopRegister(cpu.registers.bc)
#define CPU_GET_C(cpu)          GetBottomRegister(cpu.registers.bc)
#define CPU_GET_D(cpu)          GetTopRegister(cpu.registers.de)
#define CPU_GET_E(cpu)          GetBottomRegister(cpu.registers.de)
#define CPU_GET_H(cpu)          GetTopRegister(cpu.registers.hl)
#define CPU_GET_L(cpu)          GetBottomRegister(cpu.registers.hl)

// Flag register.
//
// Bit  Name  Set Clr  Expl.
// 3-0  -     -   -    Not used (always zero)
// 4    c/cy  C   NC   Carry Flag. Carry for higher 4 bits or result.
// 5    h     -   -    Half Carry Flag (BCD). Carry for lower 4 bits of result.
// 6    n     -   -    Add/Sub-Flag (BCD). Previous instruction was addition or subtraction.
// 7    zf    Z   NZ   Zero Flag

#define CPU_FLAGS_GET_C(cpu)    GetBit(CPU_GET_FLAGS(cpu), 4)
#define CPU_FLAGS_GET_H(reg)    GetBit(CPU_GET_FLAGS(cpu), 5)
#define CPU_FLAGS_GET_N(reg)    GetBit(CPU_GET_FLAGS(cpu), 6)
#define CPU_FLAGS_GET_ZF(reg)   GetBit(CPU_GET_FLAGS(cpu), 7)

#define CPU_FLAGS_SET_C(reg)    SetBit(CPU_GET_FLAGS(cpu), 4)
#define CPU_FLAGS_SET_H(reg)    SetBit(CPU_GET_FLAGS(cpu), 5)
#define CPU_FLAGS_SET_N(reg)    SetBit(CPU_GET_FLAGS(cpu), 6)
#define CPU_FLAGS_SET_ZF(reg)   SetBit(CPU_GET_FLAGS(cpu), 7)

#define CPU_FLAGS_CLEAR_C(reg)  ClearBit(CPU_GET_FLAGS(cpu), 4)
#define CPU_FLAGS_CLEAR_H(reg)  ClearBit(CPU_GET_FLAGS(cpu), 4)
#define CPU_FLAGS_CLEAR_N(reg)  ClearBit(CPU_GET_FLAGS(cpu), 4)
#define CPU_FLAGS_CLEAR_ZF(reg) ClearBit(CPU_GET_FLAGS(cpu), 4)

// Instruction -------------------------------------------------------------------------------------

struct Instruction {
  const char* name;
  const char* description;

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

inline bool IsCBInstruction(const Instruction& i) { return i.opcode.low == 0xcb; }

struct CPU {
  CPURegisters registers;
};

}  // namespace emulator
}  // namespace rothko
