// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "cpu_operations.h"

#include "cpu.h"

namespace rothko {
namespace emulator {

void TestBit(CPU* cpu, uint8_t word, int bit) {
  uint8_t mask = (uint8_t)(1 << bit);

  // Z=* N=0 H=1 C=/
  CPUFlagsSetZ(cpu, (word & mask) == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsSetH(cpu);
}

// Z=/ N=/ H=/ C=/
uint8_t SetBit(uint8_t* target, int bit) {
  uint8_t mask = (uint8_t)(1 << bit);
  *target |= mask;
}

// Z=/ N=/ H=/ C=/
uint8_t ClearBit(uint8_t* target, int bit) {
  uint8_t mask = (uint8_t) ~(1 << bit);
  *target &= mask;
}

void RotateLeftThroughCarry(CPU* cpu, uint8_t* target, int count, int carry) {
  int carry_out = (*target >> 7) & 0x1;
  uint8_t value_shifted = *target << count;
  *target = value_shifted | carry;

  // Z=* N=0 H=0 C=*
  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

void RotateLeftAndCarry(CPU* cpu, uint8_t* target, int count) {
  int carry_out = (*target >> 7) & 0x1;
  uint8_t value_shifted = *target << count;
  *target = value_shifted | carry_out;

  // Z=* N=0 H=0 C=*
  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

void RotateRightThroughCarry(CPU* cpu, uint8_t* target, int count, int carry) {
  int carry_out = *target & 0x1;
  uint8_t value_shifted = *target >> count;
  *target = value_shifted | (carry << 7);


  // Z=* N=0 H=0 C=*
  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

void RotateRightAndCarry(CPU* cpu, uint8_t* target, int count) {
  uint8_t carry_out = *target & 0x1;
  uint8_t value_shifted = *target >> count;
  *target = value_shifted | (carry_out << 7);

  // Z=* N=0 H=0 C=*
  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

void ShiftLeft(CPU* cpu, uint8_t* target, int count) {
  int carry_out = (*target >> 7) & 0x1;
  *target = (uint8_t)(*target << count);

  // Z=* N=0 H=0 C=*
  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

void ShiftRightLogic(CPU* cpu, uint8_t* target, int count) {
  int carry_out = *target & 0x1;
  *target = *target >> count;

  // Z=* N=0 H=0 C=*
  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

void ShiftRightArithmetic(CPU* cpu, uint8_t* target, int count) {
  int carry_out = *target & 0x1;
  uint8_t msb = *target & 0x80;
  *target = (*target >> count) | msb;

  // Z=* N=0 H=0 C=*
  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

// Z=* N=0 H=0 C=0
uint8_t SwapNibbles(CPU* cpu, uint8_t* target) {
  uint8_t highNibble = (uint8_t)((*target >> 4) & 0x0f);
  uint8_t lowNibble = (uint8_t)((*target << 4) & 0xf0);
  *target = (uint8_t)(highNibble | lowNibble);

  *target == 0 ? CPUFlagsSetZ(cpu) : CPUFlagsClearZ(cpu);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsClearC(cpu);
}

void SBC(CPU* cpu, uint8_t* substractee, uint8_t substractor, uint8_t extra_sub) {
  // Need more range for flags
  int A = *substractee;
  int B = substractor;
  int C = extra_sub;

  *substractee -= substractor;
  *substractee -= extra_sub;

  // Z=* N=1 H=* C=*
  uint8_t set_z = (*substractee == 0);
  CPUFlagsSetZ(cpu, set_z);

  CPUFlagsSetN(cpu);

  uint8_t set_h = (A & 0x0F) < ((B & 0x0F) + C);
  CPUFlagsSetH(cpu, set_h);

  uint8_t set_c = A < (B + C);
  CPUFlagsSetC(cpu, set_c);
}

}  // namespace emulator
}  // namespace rothko
