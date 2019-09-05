// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "cpu_operations.h"

#include "gameboy.h"

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

// Execute -----------------------------------------------------------------------------------------

namespace {

void ExecuteCBInstruction(Gameboy* gameboy, const Instruction& instruction) {
  CPU* cpu = &gameboy->cpu;

  switch (instruction.opcode.high) {
    // RLC B: Rotate B left with carry
    case 0x00: RotateLeftAndCarry(cpu, &cpu->registers.b); break;
    // RLC C: Rotate C left with carry
    case 0x01: RotateLeftAndCarry(cpu, &cpu->registers.c); break;
    // RLC D: Rotate D left with carry
    case 0x02: RotateLeftAndCarry(cpu, &cpu->registers.d); break;
    // RLC E: Rotate E left with carry
    case 0x03: RotateLeftAndCarry(cpu, &cpu->registers.e); break;
    // RLC H: Rotate H left with carry
    case 0x04: RotateLeftAndCarry(cpu, &cpu->registers.h); break;
    // RLC L: Rotate L left with carry
    case 0x05: RotateLeftAndCarry(cpu, &cpu->registers.l); break;
    // RLC (HL): Rotate value pointed by HL left with carry
    case 0x06: {
      uint16_t address = gameboy->cpu.registers.hl;
      uint8_t val = gameboy->mbc.Read(gameboy, address);
      RotateLeftAndCarry(cpu, &val);
      gameboy->mbc.WriteByte(gameboy, address, val);
      break;
    }
    // RLC A: Rotate A left with carry
    case 0x07: RotateLeftAndCarry(cpu, &cpu->registers.a); break;

    // RRC B: Rotate B right with carry
    case 0x08: RotateRightAndCarry(cpu, &cpu->registers.b); break;
    // RRC C: Rotate C right with carry
    case 0x09: RotateRightAndCarry(cpu, &cpu->registers.c); break;
    // RRC D: Rotate D right with carry
    case 0x0A: RotateRightAndCarry(cpu, &cpu->registers.d); break;
    // RRC E: Rotate E right with carry
    case 0x0B: RotateRightAndCarry(cpu, &cpu->registers.e); break;
    // RRC H: Rotate H right with carry
    case 0x0C: RotateRightAndCarry(cpu, &cpu->registers.h); break;
    // RRC L: Rotate L right with carry
    case 0x0D: RotateRightAndCarry(cpu, &cpu->registers.l); break;
    // RRC (HL): Rotate value pointed by HL right with carry
    case 0x0E: {
      uint16_t address = gameboy->cpu.registers.hl;
      uint8_t val = gameboy->mbc.Read(gameboy, address);
      RotateRightAndCarry(cpu, &val);
      gameboy->mbc.WriteByte(gameboy, address, val);
      break;
    }
    // RRC A: Rotate A right with carry
    case 0x0F: RotateRightAndCarry(cpu, &cpu->registers.a); break;

    // RL B: Rotate B left
    case 0x10: RotateLeftThroughCarry(cpu, &cpu->registers.b, 1, CPUFlagsGetC(*cpu)); break;
    // RL C: Rotate C left
    case 0x11: RotateLeftThroughCarry(cpu, &cpu->registers.c, 1, CPUFlagsGetC(*cpu)); break;
    // RL D: Rotate D left
    case 0x12: RotateLeftThroughCarry(cpu, &cpu->registers.d, 1, CPUFlagsGetC(*cpu)); break;
    // RL E: Rotate E left
    case 0x13: RotateLeftThroughCarry(cpu, &cpu->registers.e, 1, CPUFlagsGetC(*cpu)); break;
    // RL H: Rotate H left
    case 0x14: RotateLeftThroughCarry(cpu, &cpu->registers.h, 1, CPUFlagsGetC(*cpu)); break;
    // RL L: Rotate L left
    case 0x15: RotateLeftThroughCarry(cpu, &cpu->registers.l, 1, CPUFlagsGetC(*cpu)); break;
    // RL (HL): Rotate value pointed by HL left
    case 0x16: {
      uint16_t address = gameboy->cpu.registers.hl;
      uint8_t val = gameboy->mbc.Read(gameboy, address);
      RotateLeftThroughCarry(cpu, &val, 1, CPUFlagsGetC(*cpu));
      gameboy->mbc.WriteByte(gameboy, address, val);
      break;
    }
    // RL A: Rotate A left
    case 0x17: RotateLeftThroughCarry(cpu, &cpu->registers.a, 1, CPUFlagsGetC(*cpu)); break;

    // RR B: Rotate B right
    case 0x18: RotateRightThroughCarry(cpu, &cpu->registers.b, 1, CPUFlagsGetC(*cpu)); break;
    // RR C: Rotate C right
    case 0x19: RotateRightThroughCarry(cpu, &cpu->registers.c, 1, CPUFlagsGetC(*cpu)); break;
    // RR D: Rotate D right
    case 0x1A: RotateRightThroughCarry(cpu, &cpu->registers.d, 1, CPUFlagsGetC(*cpu)); break;
    // RR E: Rotate E right
    case 0x1B: RotateRightThroughCarry(cpu, &cpu->registers.e, 1, CPUFlagsGetC(*cpu)); break;
    // RR H: Rotate H right
    case 0x1C: RotateRightThroughCarry(cpu, &cpu->registers.h, 1, CPUFlagsGetC(*cpu)); break;
    // RR L: Rotate L right
    case 0x1D: RotateRightThroughCarry(cpu, &cpu->registers.l, 1, CPUFlagsGetC(*cpu)); break;
    // RR (HL): Rotate value pointed by HL right
    case 0x1E: {
      uint16_t address = gameboy->cpu.registers.hl;
      uint8_t val = gameboy->mbc.Read(gameboy, address);
      RotateRightThroughCarry(cpu, &val, 1, CPUFlagsGetC(*cpu));
      gameboy->mbc.WriteByte(gameboy, address, val);
      break;
    }
    // RR A: Rotate A right
    case 0x1F: RotateRightThroughCarry(cpu, &cpu->registers.a, 1, CPUFlagsGetC(*cpu)); break;

    // SLA B: Shift B left preserving sign
    case 0x20: ShiftLeft(cpu, &cpu->registers.b); break;
    // SLA C: Shift C left preserving sign
    case 0x21: ShiftLeft(cpu, &cpu->registers.c); break;
    // SLA D: Shift D left preserving sign
    case 0x22: ShiftLeft(cpu, &cpu->registers.d); break;
    // SLA E: Shift E left preserving sign
    case 0x23: ShiftLeft(cpu, &cpu->registers.e); break;
    // SLA H: Shift H left preserving sign
    case 0x24: ShiftLeft(cpu, &cpu->registers.h); break;
    // SLA L: Shift L left preserving sign
    case 0x25: ShiftLeft(cpu, &cpu->registers.l); break;
    // SLA (HL): Shift value pointed by HL left preserving sign
    case 0x26: {
      uint16_t address = gameboy->cpu.registers.hl;
      uint8_t val = gameboy->mbc.Read(gameboy, address);
      ShiftLeft(cpu, &val);
      gameboy->mbc.WriteByte(gameboy, address, val);
      break;
    }
    // SLA A: Shift A left preserving sign
    case 0x27: ShiftLeft(cpu, &cpu->registers.a); break;

    // SRA B: Shift B right preserving sign
    case 0x28: ShiftRightArithmetic(cpu, &cpu->registers.b); break;
    // SRA C: Shift C right preserving sign
    case 0x29: ShiftRightArithmetic(cpu, &cpu->registers.c); break;
    // SRA D: Shift D right preserving sign
    case 0x2A: ShiftRightArithmetic(cpu, &cpu->registers.d); break;
    // SRA E: Shift E right preserving sign
    case 0x2B: ShiftRightArithmetic(cpu, &cpu->registers.e); break;
    // SRA H: Shift H right preserving sign
    case 0x2C: ShiftRightArithmetic(cpu, &cpu->registers.h); break;
    // SRA L: Shift L right preserving sign
    case 0x2D: ShiftRightArithmetic(cpu, &cpu->registers.l); break;
    // SRA (HL): Shift value pointed by HL right preserving sign
    case 0x2E: {
      uint16_t address = gameboy->cpu.registers.hl;
      uint8_t val = gameboy->mbc.Read(gameboy, address);
      ShiftRightArithmetic(cpu, &val);
      gameboy->mbc.WriteByte(gameboy, address, val);
      break;
    }
    // SRA A: Shift A right preserving sign
    case 0x2F: ShiftRightArithmetic(cpu, &cpu->registers.a); break;

    // SWAP B: Swap nybbles in B
    case 0x30: SwapNibbles(cpu, &cpu->registers.b); break;
    // SWAP C: Swap nybbles in C
    case 0x31: SwapNibbles(cpu, &cpu->registers.c); break;
    // SWAP D: Swap nybbles in D
    case 0x32: SwapNibbles(cpu, &cpu->registers.d); break;
    // SWAP E: Swap nybbles in E
    case 0x33: SwapNibbles(cpu, &cpu->registers.e); break;
    // SWAP H: Swap nybbles in H
    case 0x34: SwapNibbles(cpu, &cpu->registers.h); break;
    // SWAP L: Swap nybbles in L
    case 0x35: SwapNibbles(cpu, &cpu->registers.l); break;
    // SWAP (HL): Swap nybbles in value pointed by HL
    case 0x36: {
      uint16_t address = gameboy->cpu.registers.hl;
      uint8_t val = gameboy->mbc.Read(gameboy, address);
      SwapNibbles(cpu, &val);
      gameboy->mbc.WriteByte(gameboy, address, val);
      break;
    }
    // SWAP A: Swap nybbles in A
    case 0x37: SwapNibbles(cpu, &cpu->registers.a); break;

    // SRL B: Shift B right
    case 0x38: ShiftRightLogic(cpu, &cpu->registers.b); break;
    // SRL C: Shift C right
    case 0x39: ShiftRightLogic(cpu, &cpu->registers.c); break;
    // SRL D: Shift D right
    case 0x3A: ShiftRightLogic(cpu, &cpu->registers.d); break;
    // SRL E: Shift E right
    case 0x3B: ShiftRightLogic(cpu, &cpu->registers.e); break;
    // SRL H: Shift H right
    case 0x3C: ShiftRightLogic(cpu, &cpu->registers.h); break;
    // SRL L: Shift L right
    case 0x3D: ShiftRightLogic(cpu, &cpu->registers.l); break;
    // SRL (HL): Shift value pointed by HL right
    case 0x3E: {
      uint16_t address = gameboy->cpu.registers.hl;
      uint8_t val = gameboy->mbc.Read(gameboy, address);
      ShiftRightLogic(cpu, &val);
      gameboy->mbc.WriteByte(gameboy, address, val);
      break;
    }
    // SRL A: Shift A right
    case 0x3F: ShiftRightLogic(cpu, &cpu->registers.a); break;

    // BIT 0,B: Test bit 0 of B
    case 0x40: TestBit(cpu, cpu->registers.b, 0); break;
    // BIT 0,C: Test bit 0 of C
    case 0x41: TestBit(cpu, cpu->registers.c, 0); break;
    // BIT 0,D: Test bit 0 of D
    case 0x42: TestBit(cpu, cpu->registers.d, 0); break;
    // BIT 0,E: Test bit 0 of E
    case 0x43: TestBit(cpu, cpu->registers.e, 0); break;
    // BIT 0,H: Test bit 0 of H
    case 0x44: TestBit(cpu, cpu->registers.h, 0); break;
    // BIT 0,L: Test bit 0 of L
    case 0x45: TestBit(cpu, cpu->registers.l, 0); break;
    // BIT 0,(HL): Test bit 0 of value pointed by HL
    case 0x46: TestBit(cpu, gameboy->mbc.Read(gameboy, cpu->registers.hl), 0); break;
    // BIT 0,A: Test bit 0 of A
    case 0x47: TestBit(cpu, cpu->registers.a, 0); break;

    // BIT 1,B: Test bit 1 of B
    case 0x48: TestBit(cpu, cpu->registers.b, 1); break;
    // BIT 1,C: Test bit 1 of C
    case 0x49: TestBit(cpu, cpu->registers.c, 1); break;
    // BIT 1,D: Test bit 1 of D
    case 0x4A: TestBit(cpu, cpu->registers.d, 1); break;
    // BIT 1,E: Test bit 1 of E
    case 0x4B: TestBit(cpu, cpu->registers.e, 1); break;
    // BIT 1,H: Test bit 1 of H
    case 0x4C: TestBit(cpu, cpu->registers.h, 1); break;
    // BIT 1,L: Test bit 1 of L
    case 0x4D: TestBit(cpu, cpu->registers.l, 1); break;
    // BIT 1,(HL): Test bit 1 of value pointed by HL
    case 0x4E: TestBit(cpu, gameboy->mbc.Read(gameboy, cpu->registers.hl), 1); break;
    // BIT 1,A: Test bit 1 of A
    case 0x4F: TestBit(cpu, cpu->registers.a, 1); break;

    // BIT 2,B: Test bit 2 of B
    case 0x50: TestBit(cpu, cpu->registers.b, 2); break;
    // BIT 2,C: Test bit 2 of C
    case 0x51: TestBit(cpu, cpu->registers.c, 2); break;
    // BIT 2,D: Test bit 2 of D
    case 0x52: TestBit(cpu, cpu->registers.d, 2); break;
    // BIT 2,E: Test bit 2 of E
    case 0x53: TestBit(cpu, cpu->registers.e, 2); break;
    // BIT 2,H: Test bit 2 of H
    case 0x54: TestBit(cpu, cpu->registers.h, 2); break;
    // BIT 2,L: Test bit 2 of L
    case 0x55: TestBit(cpu, cpu->registers.l, 2); break;
    // BIT 2,(HL): Test bit 2 of value pointed by HL
    case 0x56: TestBit(cpu, gameboy->mbc.Read(gameboy, cpu->registers.hl), 2); break;
    // BIT 2,A: Test bit 2 of A
    case 0x57: TestBit(cpu, cpu->registers.a, 2); break;

    // BIT 3,B: Test bit 3 of B
    case 0x58: TestBit(cpu, cpu->registers.b, 3); break;
    // BIT 3,C: Test bit 3 of C
    case 0x59: TestBit(cpu, cpu->registers.c, 3); break;
    // BIT 3,D: Test bit 3 of D
    case 0x5A: TestBit(cpu, cpu->registers.d, 3); break;
    // BIT 3,E: Test bit 3 of E
    case 0x5B: TestBit(cpu, cpu->registers.e, 3); break;
    // BIT 3,H: Test bit 3 of H
    case 0x5C: TestBit(cpu, cpu->registers.h, 3); break;
    // BIT 3,L: Test bit 3 of L
    case 0x5D: TestBit(cpu, cpu->registers.l, 3); break;
    // BIT 3,(HL): Test bit 3 of value pointed by HL
    case 0x5E: TestBit(cpu, gameboy->mbc.Read(gameboy, cpu->registers.hl), 3); break;
    // BIT 3,A: Test bit 3 of A
    case 0x5F: TestBit(cpu, cpu->registers.a, 3); break;

    // BIT 4,B: Test bit 4 of B
    case 0x60: TestBit(cpu, cpu->registers.b, 4); break;
    // BIT 4,C: Test bit 4 of C
    case 0x61: TestBit(cpu, cpu->registers.c, 4); break;
    // BIT 4,D: Test bit 4 of D
    case 0x62: TestBit(cpu, cpu->registers.d, 4); break;
    // BIT 4,E: Test bit 4 of E
    case 0x63: TestBit(cpu, cpu->registers.e, 4); break;
    // BIT 4,H: Test bit 4 of H
    case 0x64: TestBit(cpu, cpu->registers.h, 4); break;
    // BIT 4,L: Test bit 4 of L
    case 0x65: TestBit(cpu, cpu->registers.l, 4); break;
    // BIT 4,(HL): Test bit 4 of value pointed by HL
    case 0x66: TestBit(cpu, gameboy->mbc.Read(gameboy, cpu->registers.hl), 4); break;
    // BIT 4,A: Test bit 4 of A
    case 0x67: TestBit(cpu, cpu->registers.a, 4); break;

    // BIT 5,B: Test bit 5 of B
    case 0x68: TestBit(cpu, cpu->registers.b, 5); break;
    // BIT 5,C: Test bit 5 of C
    case 0x69: TestBit(cpu, cpu->registers.c, 5); break;
    // BIT 5,D: Test bit 5 of D
    case 0x6A: TestBit(cpu, cpu->registers.d, 5); break;
    // BIT 5,E: Test bit 5 of E
    case 0x6B: TestBit(cpu, cpu->registers.e, 5); break;
    // BIT 5,H: Test bit 5 of H
    case 0x6C: TestBit(cpu, cpu->registers.h, 5); break;
    // BIT 5,L: Test bit 5 of L
    case 0x6D: TestBit(cpu, cpu->registers.l, 5); break;
    // BIT 5,(HL): Test bit 5 of value pointed by HL
    case 0x6E: TestBit(cpu, gameboy->mbc.Read(gameboy, cpu->registers.hl), 5); break;
    // BIT 5,A: Test bit 5 of A
    case 0x6F: TestBit(cpu, cpu->registers.a, 5); break;

    // BIT 6,B: Test bit 6 of B
    case 0x70: TestBit(cpu, cpu->registers.b, 6); break;
    // BIT 6,C: Test bit 6 of C
    case 0x71: TestBit(cpu, cpu->registers.c, 6); break;
    // BIT 6,D: Test bit 6 of D
    case 0x72: TestBit(cpu, cpu->registers.d, 6); break;
    // BIT 6,E: Test bit 6 of E
    case 0x73: TestBit(cpu, cpu->registers.e, 6); break;
    // BIT 6,H: Test bit 6 of H
    case 0x74: TestBit(cpu, cpu->registers.h, 6); break;
    // BIT 6,L: Test bit 6 of L
    case 0x75: TestBit(cpu, cpu->registers.l, 6); break;
    // BIT 6,(HL): Test bit 6 of value pointed by HL
    case 0x76: TestBit(cpu, gameboy->mbc.Read(gameboy, cpu->registers.hl), 6); break;
    // BIT 6,A: Test bit 6 of A
    case 0x77: TestBit(cpu, cpu->registers.a, 6); break;

    // BIT 7,B: Test bit 7 of B
    case 0x78: TestBit(cpu, cpu->registers.b, 7); break;
    // BIT 7,C: Test bit 7 of C
    case 0x79: TestBit(cpu, cpu->registers.c, 7); break;
    // BIT 7,D: Test bit 7 of D
    case 0x7A: TestBit(cpu, cpu->registers.d, 7); break;
    // BIT 7,E: Test bit 7 of E
    case 0x7B: TestBit(cpu, cpu->registers.e, 7); break;
    // BIT 7,H: Test bit 7 of H
    case 0x7C: TestBit(cpu, cpu->registers.h, 7); break;
    // BIT 7,L: Test bit 7 of L
    case 0x7D: TestBit(cpu, cpu->registers.l, 7); break;
    // BIT 7,(HL): Test bit 7 of value pointed by HL
    case 0x7E: TestBit(cpu, gameboy->mbc.Read(gameboy, cpu->registers.hl), 7); break;
    // BIT 7,A: Test bit 7 of A
    case 0x7F: TestBit(cpu, cpu->registers.a, 7); break;

    // RES 0,B: Clear (reset) bit 0 of B
    case 0x80: ClearBit(&cpu->registers.b, 0); break;
    // RES 0,C: Clear (reset) bit 0 of C
    case 0x81: ClearBit(&cpu->registers.c, 0); break;
    // RES 0,D: Clear (reset) bit 0 of D
    case 0x82: ClearBit(&cpu->registers.d, 0); break;
    // RES 0,E: Clear (reset) bit 0 of E
    case 0x83: ClearBit(&cpu->registers.e, 0); break;
    // RES 0,H: Clear (reset) bit 0 of H
    case 0x84: ClearBit(&cpu->registers.h, 0); break;
    // RES 0,L: Clear (reset) bit 0 of L
    case 0x85: ClearBit(&cpu->registers.l, 0); break;
    // RES 0,(HL): Clear (reset) bit 0 of value pointed by HL
    case 0x86: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      ClearBit(&val, 0);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // RES 0,A: Clear (reset) bit 0 of A
    case 0x87: ClearBit(&cpu->registers.a, 0); break;

    // RES 1,B: Clear (reset) bit 1 of B
    case 0x88: ClearBit(&cpu->registers.b, 1); break;
    // RES 1,C: Clear (reset) bit 1 of C
    case 0x89: ClearBit(&cpu->registers.c, 1); break;
    // RES 1,D: Clear (reset) bit 1 of D
    case 0x8A: ClearBit(&cpu->registers.d, 1); break;
    // RES 1,E: Clear (reset) bit 1 of E
    case 0x8B: ClearBit(&cpu->registers.e, 1); break;
    // RES 1,H: Clear (reset) bit 1 of H
    case 0x8C: ClearBit(&cpu->registers.h, 1); break;
    // RES 1,L: Clear (reset) bit 1 of L
    case 0x8D: ClearBit(&cpu->registers.l, 1); break;
    // RES 1,(HL): Clear (reset) bit 1 of value pointed by HL
    case 0x8E: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      ClearBit(&val, 1);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // RES 1,A: Clear (reset) bit 1 of A
    case 0x8F: ClearBit(&cpu->registers.a, 1); break;

    // RES 2,B: Clear (reset) bit 2 of B
    case 0x90: ClearBit(&cpu->registers.b, 2); break;
    // RES 2,C: Clear (reset) bit 2 of C
    case 0x91: ClearBit(&cpu->registers.c, 2); break;
    // RES 2,D: Clear (reset) bit 2 of D
    case 0x92: ClearBit(&cpu->registers.d, 2); break;
    // RES 2,E: Clear (reset) bit 2 of E
    case 0x93: ClearBit(&cpu->registers.e, 2); break;
    // RES 2,H: Clear (reset) bit 2 of H
    case 0x94: ClearBit(&cpu->registers.h, 2); break;
    // RES 2,L: Clear (reset) bit 2 of L
    case 0x95: ClearBit(&cpu->registers.l, 2); break;
    // RES 2,(HL): Clear (reset) bit 2 of value pointed by HL
    case 0x96: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      ClearBit(&val, 2);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // RES 2,A: Clear (reset) bit 2 of A
    case 0x97: ClearBit(&cpu->registers.a, 2); break;

    // RES 3,B: Clear (reset) bit 3 of B
    case 0x98: ClearBit(&cpu->registers.b, 3); break;
    // RES 3,C: Clear (reset) bit 3 of C
    case 0x99: ClearBit(&cpu->registers.c, 3); break;
    // RES 3,D: Clear (reset) bit 3 of D
    case 0x9A: ClearBit(&cpu->registers.d, 3); break;
    // RES 3,E: Clear (reset) bit 3 of E
    case 0x9B: ClearBit(&cpu->registers.e, 3); break;
    // RES 3,H: Clear (reset) bit 3 of H
    case 0x9C: ClearBit(&cpu->registers.h, 3); break;
    // RES 3,L: Clear (reset) bit 3 of L
    case 0x9D: ClearBit(&cpu->registers.l, 3); break;
    // RES 3,(HL): Clear (reset) bit 3 of value pointed by HL
    case 0x9E: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      ClearBit(&val, 3);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // RES 3,A: Clear (reset) bit 3 of A
    case 0x9F: ClearBit(&cpu->registers.a, 3); break;

    // RES 4,B: Clear (reset) bit 4 of B
    case 0xA0: ClearBit(&cpu->registers.b, 4); break;
    // RES 4,C: Clear (reset) bit 4 of C
    case 0xA1: ClearBit(&cpu->registers.c, 4); break;
    // RES 4,D: Clear (reset) bit 4 of D
    case 0xA2: ClearBit(&cpu->registers.d, 4); break;
    // RES 4,E: Clear (reset) bit 4 of E
    case 0xA3: ClearBit(&cpu->registers.e, 4); break;
    // RES 4,H: Clear (reset) bit 4 of H
    case 0xA4: ClearBit(&cpu->registers.h, 4); break;
    // RES 4,L: Clear (reset) bit 4 of L
    case 0xA5: ClearBit(&cpu->registers.l, 4); break;
    // RES 4,(HL): Clear (reset) bit 4 of value pointed by HL
    case 0xA6: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      ClearBit(&val, 4);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // RES 4,A: Clear (reset) bit 4 of A
    case 0xA7: ClearBit(&cpu->registers.a, 4); break;

    // RES 5,B: Clear (reset) bit 5 of B
    case 0xA8: ClearBit(&cpu->registers.b, 5); break;
    // RES 5,C: Clear (reset) bit 5 of C
    case 0xA9: ClearBit(&cpu->registers.c, 5); break;
    // RES 5,D: Clear (reset) bit 5 of D
    case 0xAA: ClearBit(&cpu->registers.d, 5); break;
    // RES 5,E: Clear (reset) bit 5 of E
    case 0xAB: ClearBit(&cpu->registers.e, 5); break;
    // RES 5,H: Clear (reset) bit 5 of H
    case 0xAC: ClearBit(&cpu->registers.h, 5); break;
    // RES 5,L: Clear (reset) bit 5 of L
    case 0xAD: ClearBit(&cpu->registers.l, 5); break;
    // RES 5,(HL): Clear (reset) bit 5 of value pointed by HL
    case 0xAE: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      ClearBit(&val, 5);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // RES 5,A: Clear (reset) bit 5 of A
    case 0xAF: ClearBit(&cpu->registers.a, 5); break;

    // RES 6,B: Clear (reset) bit 6 of B
    case 0xB0: ClearBit(&cpu->registers.b, 6); break;
    // RES 6,C: Clear (reset) bit 6 of C
    case 0xB1: ClearBit(&cpu->registers.c, 6); break;
    // RES 6,D: Clear (reset) bit 6 of D
    case 0xB2: ClearBit(&cpu->registers.d, 6); break;
    // RES 6,E: Clear (reset) bit 6 of E
    case 0xB3: ClearBit(&cpu->registers.e, 6); break;
    // RES 6,H: Clear (reset) bit 6 of H
    case 0xB4: ClearBit(&cpu->registers.h, 6); break;
    // RES 6,L: Clear (reset) bit 6 of L
    case 0xB5: ClearBit(&cpu->registers.l, 6); break;
    // RES 6,(HL): Clear (reset) bit 6 of value pointed by HL
    case 0xB6: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      ClearBit(&val, 6);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // RES 6,A: Clear (reset) bit 6 of A
    case 0xB7: ClearBit(&cpu->registers.a, 6); break;

    // RES 7,B: Clear (reset) bit 7 of B
    case 0xB8: ClearBit(&cpu->registers.b, 7); break;
    // RES 7,C: Clear (reset) bit 7 of C
    case 0xB9: ClearBit(&cpu->registers.c, 7); break;
    // RES 7,D: Clear (reset) bit 7 of D
    case 0xBA: ClearBit(&cpu->registers.d, 7); break;
    // RES 7,E: Clear (reset) bit 7 of E
    case 0xBB: ClearBit(&cpu->registers.e, 7); break;
    // RES 7,H: Clear (reset) bit 7 of H
    case 0xBC: ClearBit(&cpu->registers.h, 7); break;
    // RES 7,L: Clear (reset) bit 7 of L
    case 0xBD: ClearBit(&cpu->registers.l, 7); break;
    // RES 7,(HL): Clear (reset) bit 7 of value pointed by HL
    case 0xBE: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      ClearBit(&val, 7);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // RES 7,A: Clear (reset) bit 7 of A
    case 0xBF: ClearBit(&cpu->registers.a, 7); break;

    // SET 0,B: Set bit 0 of B
    case 0xC0: SetBit(&cpu->registers.b, 0); break;
    // SET 0,C: Set bit 0 of C
    case 0xC1: SetBit(&cpu->registers.c, 0); break;
    // SET 0,D: Set bit 0 of D
    case 0xC2: SetBit(&cpu->registers.d, 0); break;
    // SET 0,E: Set bit 0 of E
    case 0xC3: SetBit(&cpu->registers.e, 0); break;
    // SET 0,H: Set bit 0 of H
    case 0xC4: SetBit(&cpu->registers.h, 0); break;
    // SET 0,L: Set bit 0 of L
    case 0xC5: SetBit(&cpu->registers.l, 0); break;
    // SET 0,(HL): Set bit 0 of value pointed by HL
    case 0xC6: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      SetBit(&val, 0);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // SET 0,A: Set bit 0 of A
    case 0xC7: SetBit(&cpu->registers.a, 0); break;

    // SET 1,B: Set bit 1 of B
    case 0xC8: SetBit(&cpu->registers.b, 1); break;
    // SET 1,C: Set bit 1 of C
    case 0xC9: SetBit(&cpu->registers.c, 1); break;
    // SET 1,D: Set bit 1 of D
    case 0xCA: SetBit(&cpu->registers.d, 1); break;
    // SET 1,E: Set bit 1 of E
    case 0xCB: SetBit(&cpu->registers.e, 1); break;
    // SET 1,H: Set bit 1 of H
    case 0xCC: SetBit(&cpu->registers.h, 1); break;
    // SET 1,L: Set bit 1 of L
    case 0xCD: SetBit(&cpu->registers.l, 1); break;
    // SET 1,(HL): Set bit 1 of value pointed by HL
    case 0xCE: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      SetBit(&val, 1);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // SET 1,A: Set bit 1 of A
    case 0xCF: SetBit(&cpu->registers.a, 1); break;

    // SET 2,B: Set bit 2 of B
    case 0xD0: SetBit(&cpu->registers.b, 2); break;
    // SET 2,C: Set bit 2 of C
    case 0xD1: SetBit(&cpu->registers.c, 2); break;
    // SET 2,D: Set bit 2 of D
    case 0xD2: SetBit(&cpu->registers.d, 2); break;
    // SET 2,E: Set bit 2 of E
    case 0xD3: SetBit(&cpu->registers.e, 2); break;
    // SET 2,H: Set bit 2 of H
    case 0xD4: SetBit(&cpu->registers.h, 2); break;
    // SET 2,L: Set bit 2 of L
    case 0xD5: SetBit(&cpu->registers.l, 2); break;
    // SET 2,(HL): Set bit 2 of value pointed by HL
    case 0xD6: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      SetBit(&val, 2);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // SET 2,A: Set bit 2 of A
    case 0xD7: SetBit(&cpu->registers.a, 2); break;

    // SET 3,B: Set bit 3 of B
    case 0xD8: SetBit(&cpu->registers.b, 3); break;
    // SET 3,C: Set bit 3 of C
    case 0xD9: SetBit(&cpu->registers.c, 3); break;
    // SET 3,D: Set bit 3 of D
    case 0xDA: SetBit(&cpu->registers.d, 3); break;
    // SET 3,E: Set bit 3 of E
    case 0xDB: SetBit(&cpu->registers.e, 3); break;
    // SET 3,H: Set bit 3 of H
    case 0xDC: SetBit(&cpu->registers.h, 3); break;
    // SET 3,L: Set bit 3 of L
    case 0xDD: SetBit(&cpu->registers.l, 3); break;
    // SET 3,(HL): Set bit 3 of value pointed by HL
    case 0xDE: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      SetBit(&val, 3);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // SET 3,A: Set bit 3 of A
    case 0xDF: SetBit(&cpu->registers.a, 3); break;

    // SET 4,B: Set bit 4 of B
    case 0xE0: SetBit(&cpu->registers.b, 4); break;
    // SET 4,C: Set bit 4 of C
    case 0xE1: SetBit(&cpu->registers.c, 4); break;
    // SET 4,D: Set bit 4 of D
    case 0xE2: SetBit(&cpu->registers.d, 4); break;
    // SET 4,E: Set bit 4 of E
    case 0xE3: SetBit(&cpu->registers.e, 4); break;
    // SET 4,H: Set bit 4 of H
    case 0xE4: SetBit(&cpu->registers.h, 4); break;
    // SET 4,L: Set bit 4 of L
    case 0xE5: SetBit(&cpu->registers.l, 4); break;
    // SET 4,(HL): Set bit 4 of value pointed by HL
    case 0xE6: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      SetBit(&val, 4);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // SET 4,A: Set bit 4 of A
    case 0xE7: SetBit(&cpu->registers.a, 4); break;

    // SET 5,B: Set bit 5 of B
    case 0xE8: SetBit(&cpu->registers.b, 5); break;
    // SET 5,C: Set bit 5 of C
    case 0xE9: SetBit(&cpu->registers.c, 5); break;
    // SET 5,D: Set bit 5 of D
    case 0xEA: SetBit(&cpu->registers.d, 5); break;
    // SET 5,E: Set bit 5 of E
    case 0xEB: SetBit(&cpu->registers.e, 5); break;
    // SET 5,H: Set bit 5 of H
    case 0xEC: SetBit(&cpu->registers.h, 5); break;
    // SET 5,L: Set bit 5 of L
    case 0xED: SetBit(&cpu->registers.l, 5); break;
    // SET 5,(HL): Set bit 5 of value pointed by HL
    case 0xEE: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      SetBit(&val, 5);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // SET 5,A: Set bit 5 of A
    case 0xEF: SetBit(&cpu->registers.a, 5); break;

    // SET 6,B: Set bit 6 of B
    case 0xF0: SetBit(&cpu->registers.b, 6); break;
    // SET 6,C: Set bit 6 of C
    case 0xF1: SetBit(&cpu->registers.c, 6); break;
    // SET 6,D: Set bit 6 of D
    case 0xF2: SetBit(&cpu->registers.d, 6); break;
    // SET 6,E: Set bit 6 of E
    case 0xF3: SetBit(&cpu->registers.e, 6); break;
    // SET 6,H: Set bit 6 of H
    case 0xF4: SetBit(&cpu->registers.h, 6); break;
    // SET 6,L: Set bit 6 of L
    case 0xF5: SetBit(&cpu->registers.l, 6); break;
    // SET 6,(HL): Set bit 6 of value pointed by HL
    case 0xF6: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      SetBit(&val, 6);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // SET 6,A: Set bit 6 of A
    case 0xF7: SetBit(&cpu->registers.a, 6); break;

    // SET 7,B: Set bit 7 of B
    case 0xF8: SetBit(&cpu->registers.b, 7); break;
    // SET 7,C: Set bit 7 of C
    case 0xF9: SetBit(&cpu->registers.c, 7); break;
    // SET 7,D: Set bit 7 of D
    case 0xFA: SetBit(&cpu->registers.d, 7); break;
    // SET 7,E: Set bit 7 of E
    case 0xFB: SetBit(&cpu->registers.e, 7); break;
    // SET 7,H: Set bit 7 of H
    case 0xFC: SetBit(&cpu->registers.h, 7); break;
    // SET 7,L: Set bit 7 of L
    case 0xFD: SetBit(&cpu->registers.l, 7); break;
    // SET 7,(HL): Set bit 7 of value pointed by HL
    case 0xFE: {
      uint8_t val = gameboy->mbc.Read(gameboy, cpu->registers.hl);
      SetBit(&val, 7);
      gameboy->mbc.WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // SET 7,A: Set bit 7 of A
    case 0xFF: SetBit(&cpu->registers.a, 7); break;
  }
}

}  // namespace

}  // namespace emulator
}  // namespace rothko
