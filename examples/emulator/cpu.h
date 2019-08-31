// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

#include "cpu_instructions.h"

namespace rothko {
namespace emulator {

struct CPU;

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
};

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

constexpr uint8_t kCPUFlagsCMask = (1 << 4);
constexpr uint8_t kCPUFlagsCIndex = 4;

constexpr uint8_t kCPUFlagsHMask = (1 << 5);
constexpr uint8_t kCPUFlagsHIndex = 5;

constexpr uint8_t kCPUFlagsNMask = (1 << 6);
constexpr uint8_t kCPUFlagsNIndex = 6;

constexpr uint8_t kCPUFlagsZMask = (1 << 7);
constexpr uint8_t kCPUFlagsZIndex = 7;

#define CPU_FLAGS_GET_C(cpu)    GetBit(CPU_GET_FLAGS(cpu), 4)
#define CPU_FLAGS_GET_H(cpu)    GetBit(CPU_GET_FLAGS(cpu), 5)
#define CPU_FLAGS_GET_N(cpu)    GetBit(CPU_GET_FLAGS(cpu), 6)
#define CPU_FLAGS_GET_Z(cpu)    GetBit(CPU_GET_FLAGS(cpu), 7)

#define CPU_FLAGS_SET_C(cpu)    SetBit(CPU_GET_FLAGS(cpu), 4)
#define CPU_FLAGS_SET_H(cpu)    SetBit(CPU_GET_FLAGS(cpu), 5)
#define CPU_FLAGS_SET_N(cpu)    SetBit(CPU_GET_FLAGS(cpu), 6)
#define CPU_FLAGS_SET_Z(cpu)    SetBit(CPU_GET_FLAGS(cpu), 7)

#define CPU_FLAGS_CLEAR_C(cpu)  ClearBit(CPU_GET_FLAGS(cpu), 4)
#define CPU_FLAGS_CLEAR_H(cpu)  ClearBit(CPU_GET_FLAGS(cpu), 4)
#define CPU_FLAGS_CLEAR_N(cpu)  ClearBit(CPU_GET_FLAGS(cpu), 4)
#define CPU_FLAGS_CLEAR_Z(cpu)  ClearBit(CPU_GET_FLAGS(cpu), 4)

// Instruction -------------------------------------------------------------------------------------

struct CPU {
  CPURegisters registers;

  Instruction instructions[0xff + 1];
  Instruction cb_instructions[0xff + 1];
};

void Init(CPU*);

}  // namespace emulator
}  // namespace rothko
