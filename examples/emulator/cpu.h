// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/math/math.h>
#include <stdint.h>

#include <string>

#include "cpu_instructions.h"

namespace rothko {
namespace emulator {

struct CPU;
struct Gameboy;

// Registers ---------------------------------------------------------------------------------------

#define CREATE_CPU_REGISTER_UNION(reg1, reg2) \
  union {                                     \
    uint16_t reg1##reg2;                      \
    struct {                                  \
      uint8_t reg2;                           \
      uint8_t reg1;                           \
    };                                        \
  };

struct CPURegisters {
  // All these registers can be accessed as a 16-bit value or as two separate 8-bit value:
  // bc = 16-bit. b = top 8-bits. c = bottom 8-bit.
  CREATE_CPU_REGISTER_UNION(a, f);      // Accumulator / Flags.
  CREATE_CPU_REGISTER_UNION(b, c);      // b and c registers.
  CREATE_CPU_REGISTER_UNION(d, e);      // d and e registers.
  CREATE_CPU_REGISTER_UNION(h, l);      // h and l registers.

  uint16_t pc;  // Program counter.
  uint16_t sp;  // Stack pointer.
};

std::string ToString(const CPURegisters&);

inline uint8_t GetTopRegister(uint16_t reg) { return (reg >> 8); }
inline uint8_t GetBottomRegister(uint16_t reg) { return reg & 0xff; }

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

// Instruction -------------------------------------------------------------------------------------

struct CPU {
  CPURegisters registers;

  bool stopped = false;
};

inline uint8_t CPUFlagsGetC(const CPU& cpu) { return GetBit(cpu.registers.f, 4); }
inline uint8_t CPUFlagsGetH(const CPU& cpu) { return GetBit(cpu.registers.f, 5); }
inline uint8_t CPUFlagsGetN(const CPU& cpu) { return GetBit(cpu.registers.f, 6); }
inline uint8_t CPUFlagsGetZ(const CPU& cpu) { return GetBit(cpu.registers.f, 7); }

inline uint8_t _Set(CPU* cpu, uint8_t bit) {
  SetBit(&cpu->registers.f, bit);
  return cpu->registers.f;
}

inline uint8_t _Clear(CPU* cpu, uint8_t bit) {
  ClearBit(&cpu->registers.f, bit);
  return cpu->registers.f;
}

inline uint8_t CPUFlagsSetC(CPU* cpu) { return _Set(cpu, 4); }
inline uint8_t CPUFlagsSetC(CPU* cpu, uint8_t v) { return v ? _Set(cpu, 4) : _Clear(cpu, 4); }

inline uint8_t CPUFlagsSetH(CPU* cpu) { return _Set(cpu, 5); }
inline uint8_t CPUFlagsSetH(CPU* cpu, uint8_t v) { return v ? _Set(cpu, 5) : _Clear(cpu, 5); }

inline uint8_t CPUFlagsSetN(CPU* cpu) { return _Set(cpu, 6); }
inline uint8_t CPUFlagsSetN(CPU* cpu, uint8_t v) { return v ? _Set(cpu, 6) : _Clear(cpu, 6); }

inline uint8_t CPUFlagsSetZ(CPU* cpu) { return _Set(cpu, 7); }
inline uint8_t CPUFlagsSetZ(CPU* cpu, uint8_t v) { return v ? _Set(cpu, 7) : _Clear(cpu, 7); }

inline uint8_t CPUFlagsClearC(CPU* cpu) { return _Clear(cpu, 4); }
inline uint8_t CPUFlagsClearH(CPU* cpu) { return _Clear(cpu, 5); }
inline uint8_t CPUFlagsClearN(CPU* cpu) { return _Clear(cpu, 6); }
inline uint8_t CPUFlagsClearZ(CPU* cpu) { return _Clear(cpu, 7); }

void Init(CPU*);

uint8_t StepCPU(Gameboy*);

}  // namespace emulator
}  // namespace rothko
