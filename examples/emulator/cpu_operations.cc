// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "cpu_operations.h"

#include "gameboy.h"

namespace rothko {
namespace emulator {

namespace {

// Z=* N=0 H=1 C=/
inline void TestBit(CPU* cpu, uint8_t word, int bit) {
  uint8_t mask = (uint8_t)(1 << bit);

  CPUFlagsSetZ(cpu, (word & mask) == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsSetH(cpu);
}

// Z=/ N=/ H=/ C=/
inline void SetBit(uint8_t* target, int bit) {
  uint8_t mask = (uint8_t)(1 << bit);
  *target |= mask;
}

// Z=/ N=/ H=/ C=/
inline void ClearBit(uint8_t* target, int bit) {
  uint8_t mask = (uint8_t) ~(1 << bit);
  *target &= mask;
}

// Z=* N=0 H=0 C=*
inline void RotateLeftThroughCarry(CPU* cpu, uint8_t* target, int count, int carry) {
  int carry_out = (*target >> 7) & 0x1;
  uint8_t value_shifted = *target << count;
  *target = value_shifted | carry;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

// Z=* N=0 H=0 C=*
inline void RotateLeftAndCarry(CPU* cpu, uint8_t* target, int count = 1) {
  int carry_out = (*target >> 7) & 0x1;
  uint8_t value_shifted = *target << count;
  *target = value_shifted | carry_out;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

// Z=* N=0 H=0 C=*
inline void RotateRightThroughCarry(CPU* cpu, uint8_t* target, int count = 1, int carry = 0) {
  int carry_out = *target & 0x1;
  uint8_t value_shifted = *target >> count;
  *target = value_shifted | (carry << 7);

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

// Z=* N=0 H=0 C=*
inline void RotateRightAndCarry(CPU* cpu, uint8_t* target, int count = 1) {
  uint8_t carry_out = *target & 0x1;
  uint8_t value_shifted = *target >> count;
  *target = value_shifted | (carry_out << 7);

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

// Z=* N=0 H=0 C=*
inline void ShiftLeft(CPU* cpu, uint8_t* target, int count = 1) {
  int carry_out = (*target >> 7) & 0x1;
  *target = (uint8_t)(*target << count);

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

// Z=* N=0 H=0 C=*
inline void ShiftRightLogic(CPU* cpu, uint8_t* target, int count = 1) {
  int carry_out = *target & 0x1;
  *target = *target >> count;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

// Z=* N=0 H=0 C=*
inline void ShiftRightArithmetic(CPU* cpu, uint8_t* target, int count = 1) {
  int carry_out = *target & 0x1;
  uint8_t msb = *target & 0x80;
  *target = (*target >> count) | msb;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsSetC(cpu, carry_out);
}

// Z=* N=0 H=0 C=0
inline void SwapNibbles(CPU* cpu, uint8_t* target) {
  uint8_t highNibble = (uint8_t)((*target >> 4) & 0x0f);
  uint8_t lowNibble = (uint8_t)((*target << 4) & 0xf0);
  *target = (uint8_t)(highNibble | lowNibble);

  *target == 0 ? CPUFlagsSetZ(cpu) : CPUFlagsClearZ(cpu);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsClearC(cpu);
}

// Z=* N=1 H=* C=*
inline void SBC(CPU* cpu, uint8_t* substractee, uint8_t substractor, uint8_t extra_sub) {
  // Need more range for flags
  int A = *substractee;
  int B = substractor;
  int C = extra_sub;

  *substractee -= substractor;
  *substractee -= extra_sub;

  uint8_t set_z = (*substractee == 0);
  CPUFlagsSetZ(cpu, set_z);

  CPUFlagsSetN(cpu);

  uint8_t set_h = (A & 0x0F) < ((B & 0x0F) + C);
  CPUFlagsSetH(cpu, set_h);

  uint8_t set_c = A < (B + C);
  CPUFlagsSetC(cpu, set_c);
}

// Z=* N=0 H=* C=/
inline void IncrementByteRegister(CPU* cpu, uint8_t* target) {
  *target += 1;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearH(cpu);
  CPUFlagsSetH(cpu, (*target & 0x0f) == 0);
}

// Z=* N=0 H=* C=/
inline void DecrementByteRegister(CPU* cpu, uint8_t* target) {
  *target -= 1;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearH(cpu);
  CPUFlagsSetH(cpu, (*target & 0x0f) == 0);
}

// Z=* N=0 H=* C=*
inline void Add(CPU* cpu, uint8_t* target, uint8_t value) {
  uint8_t initial = *target;
  int sum = initial + value;

  (*target) += value;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  uint8_t set_c = ((*target ^ value ^ initial) & 0x10);
  CPUFlagsSetH(cpu, set_c);
  CPUFlagsSetC(cpu, sum > 0xff);
}

// Z=* N=1 H=* C=*
inline void Sub(CPU* cpu, uint8_t* target, uint8_t value) {
  SBC(cpu, target, value, 0);
}

// Z=* N=0 H=* C=*
inline void Adc(CPU* cpu, uint8_t* target, uint8_t value) {
  uint16_t sum = *target;
  uint8_t initial = *target;
  sum += value;
  sum += CPUFlagsGetC(*cpu);
  *target = (uint8_t)sum;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  uint8_t set_c = ((*target ^ value ^ initial) & 0x10);
  CPUFlagsSetH(cpu, set_c);
  CPUFlagsSetC(cpu, sum > 0xff);
}

// Z=* N=0 H=1 C=0
inline void And(CPU* cpu, uint8_t* target, uint8_t value) {
  *target &= value;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsSetH(cpu);
  CPUFlagsSetC(cpu);
}

// Z=* N=0 H=0 C=0
inline void Xor(CPU* cpu, uint8_t* target, uint8_t value) {
  *target ^= value;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsClearC(cpu);
}

// Z=* N=0 H=0 C=0
inline void Or(CPU* cpu, uint8_t* target, uint8_t value) {
  *target |= value;

  CPUFlagsSetZ(cpu, *target == 0);
  CPUFlagsClearN(cpu);
  CPUFlagsClearH(cpu);
  CPUFlagsClearC(cpu);
}

// Z=* N=1 H=* C=*
inline void Cp(CPU* cpu, uint8_t* target, uint8_t value) {
  int set_c = 0;
  if (*target == value) {
    CPUFlagsSetZ(cpu);
  } else {
    CPUFlagsClearZ(cpu);
    if (*target < value) {
      set_c = 1;
    }
  }

  CPUFlagsSetN(cpu);

  int set_h = ((*target & 0x0F) < (value & 0x0F));
  CPUFlagsSetH(cpu, set_h);

  CPUFlagsSetC(cpu, set_c);
}

inline void Call(Gameboy* gameboy, uint16_t address) {
  CPU* cpu = &gameboy->cpu;

  // Write into the stack.
  cpu->registers.sp -= 2;
  WriteShort(gameboy, cpu->registers.sp, cpu->registers.pc);

  // Jump.
  cpu->registers.pc = address;
}

inline void Ret(Gameboy* gameboy) {
  CPU* cpu = &gameboy->cpu;

  cpu->registers.pc = ReadShort(gameboy, cpu->registers.sp);
  cpu->registers.sp += 2;
}

inline void Pop(Gameboy* gameboy, uint16_t* target) {
  CPU* cpu = &gameboy->cpu;

  *target = ReadShort(gameboy, cpu->registers.sp);
  cpu->registers.sp += 2;
}

inline void Push(Gameboy* gameboy, uint16_t target) {
  CPU* cpu = &gameboy->cpu;

  cpu->registers.sp -= 2;
  WriteShort(gameboy, cpu->registers.sp, target);
}

inline void Ldh(Gameboy* gameboy, uint8_t offset, uint8_t value) {
  uint16_t address = (uint16_t)(0xff00 | offset);
  WriteByte(gameboy, address, value);
}

}  // namespace

// Execute -----------------------------------------------------------------------------------------

namespace {

void ExecuteNormalInstruction(Gameboy* gameboy, const Instruction& instruction) {
  CPU* cpu = &gameboy->cpu;

  switch (instruction.opcode.low) {
    // NOP: No Operation
    case 0x00: break;
    // LD BC,nn: Load 16-bit immediate into BC
    case 0x01: cpu->registers.bc = instruction.operand; break;
    // LD (BC),A: Save A to address pointed by BC
    case 0x02: WriteByte(gameboy, cpu->registers.bc, cpu->registers.a); break;
    // INC BC: Increment 16-bit BC
    case 0x03: cpu->registers.bc++; break;
    // INC B: Increment B.
    case 0x04: IncrementByteRegister(cpu, &cpu->registers.b); break;
    // DEC B: Decrement B
    case 0x05: DecrementByteRegister(cpu, &cpu->registers.b); break;
    // LD B,n: Load 8-bit immediate into B
    case 0x06: cpu->registers.b = instruction.operands[0]; break;
    // RLC A: Rotate A left with carry
    case 0x07: RotateLeftAndCarry(cpu, &cpu->registers.a); break;
    // LD (nn),SP: Save SP to given address
    case 0x08: WriteShort(gameboy, instruction.operand, cpu->registers.sp); break;
    // ADD HL,BC: Add 16-bit BC to HL
    case 0x09: {
      uint8_t initial_h = cpu->registers.h;
      uint32_t res = cpu->registers.hl + cpu->registers.bc;
      cpu->registers.hl += cpu->registers.bc;

      CPUFlagsClearN(cpu);
      CPUFlagsSetH(cpu, ((cpu->registers.h ^ cpu->registers.b ^ initial_h) & 0x10));
      CPUFlagsSetC(cpu, res > 0xffff);
      break;
    }
    // LD A,(BC): Load A from address pointed to by BC
    case 0x0A: cpu->registers.a = ReadByte(gameboy, cpu->registers.bc); break;
    // DEC BC: Decrement 16-bit BC
    case 0x0B: cpu->registers.bc--; break;
    // INC C: Increment C
    case 0x0C: IncrementByteRegister(cpu, &cpu->registers.c); break;
    // DEC C: Decrement C
    case 0x0D: DecrementByteRegister(cpu, &cpu->registers.c); break;
    // LD C,n: Load 8-bit immediate into C
    case 0x0E: cpu->registers.c = instruction.operands[0]; break;
    // RRC A: Rotate A right with carry
    case 0x0F: RotateRightAndCarry(cpu, &cpu->registers.a); break;
    // STOP: Stop processor
    case 0x10: cpu->stopped = true; break;
    // LD DE,nn: Load 16-bit immediate into DE
    case 0x11: cpu->registers.de = instruction.operand; break;
    // LD (DE),A: Save A to address pointed by DE
    case 0x12: WriteByte(gameboy, cpu->registers.de, cpu->registers.a); break;
    // INC DE: Increment 16-bit DE
    case 0x13: cpu->registers.de++; break;
    // INC D: Increment D
    case 0x14: IncrementByteRegister(cpu, &cpu->registers.d); break;
    // DEC D: Decrement D
    case 0x15: DecrementByteRegister(cpu, &cpu->registers.d); break;
    // LD D,n: Load 8-bit immediate into D
    case 0x16: cpu->registers.d = instruction.operands[0]; break;
    // RL A: Rotate A left
    case 0x17: RotateLeftThroughCarry(cpu, &cpu->registers.a, 1, CPUFlagsGetC(*cpu)); break;
    // JR n: Relative jump by signed immediate
    case 0x18: cpu->registers.pc += (int8_t)instruction.operands[0]; break;
    // ADD HL,DE: Add 16-bit DE to HL
    case 0x19: {
      uint8_t initial_h = cpu->registers.h;
      uint32_t res = cpu->registers.hl + cpu->registers.de;
      cpu->registers.hl += cpu->registers.de;

      CPUFlagsClearN(cpu);
      CPUFlagsSetH(cpu, ((cpu->registers.h ^ cpu->registers.b ^ initial_h) & 0x10));
      CPUFlagsSetC(cpu, res > 0xffff);
      break;
    }
    // LD A,(DE): Load A from address pointed to by DE
    case 0x1A: cpu->registers.a = ReadByte(gameboy, cpu->registers.de); break;
    // DEC DE: Decrement 16-bit DE
    case 0x1B: cpu->registers.de--; break;
    // INC E: Increment E
    case 0x1C: IncrementByteRegister(cpu, &cpu->registers.e); break;
    // DEC E: Decrement E
    case 0x1D: DecrementByteRegister(cpu, &cpu->registers.e); break;
    // LD E,n: Load 8-bit immediate into E
    case 0x1E: cpu->registers.e = instruction.operands[0]; break;
    // RR A: Rotate A right
    case 0x1F: RotateRightThroughCarry(cpu, &cpu->registers.a, 1, CPUFlagsGetC(*cpu)); break;
    // JR NZ,n: Relative jump by signed immediate if last result was not zero
    case 0x20: {
      if (CPUFlagsGetZ(*cpu) != 0)
        cpu->registers.pc += (int8_t)instruction.operands[0];
      break;
    }
    // LD HL,nn: Load 16-bit immediate into HL
    case 0x21: cpu->registers.hl = instruction.operand; break;
    // LDI (HL),A: Save A to address pointed by HL, and increment HL
    case 0x22: WriteByte(gameboy, cpu->registers.hl++, cpu->registers.a); break;
    // INC HL: Increment 16-bit HL
    case 0x23: cpu->registers.hl++; break;
    // INC H: Increment H
    case 0x24: IncrementByteRegister(cpu, &cpu->registers.h); break;
    // DEC H: Decrement H
    case 0x25: DecrementByteRegister(cpu, &cpu->registers.h); break;
    // LD H,n: Load 8-bit immediate into H
    case 0x26: cpu->registers.h = instruction.operands[0]; break;
    // DAA: Adjust A for BCD addition
    case 0x27: {
      int value = cpu->registers.a;

      // ADD, ADC, INC
      if (CPUFlagsGetN(*cpu) != 0) {
        if (CPUFlagsGetH(*cpu) != 0)
          value = (value - 0x06) & 0xFF;

        if (CPUFlagsGetC(*cpu) != 0)
          value -= 0x60;
      } else {
        // SUB, SBC, DEC, NEG
        if ((CPUFlagsGetH(*cpu) != 0) || ((value & 0x0F) > 0x09))
          value += 0x06;

        if ((CPUFlagsGetC(*cpu) != 0) || (value > 0x9F))
          value += 0x60;
      }

      value &= 0xFF;

      CPUFlagsSetZ(cpu, value == 0);
      CPUFlagsClearH(cpu);
      CPUFlagsSetC(cpu, (value & 0x100));

      cpu->registers.a = (uint8_t)value;

      break;
    }
    // JR Z,n: Relative jump by signed immediate if last result was zero
    case 0x28: {
      if (CPUFlagsGetZ(*cpu) == 0)
        cpu->registers.pc += (int8_t)instruction.operands[0];
      break;
    }
    // ADD HL,HL: Add 16-bit HL to HL
    case 0x29: {
      uint8_t initial_h = cpu->registers.h;
      uint32_t res = cpu->registers.hl + cpu->registers.hl;
      cpu->registers.hl += cpu->registers.hl;

      CPUFlagsClearN(cpu);
      CPUFlagsSetH(cpu, ((cpu->registers.h ^ cpu->registers.b ^ initial_h) & 0x10));
      CPUFlagsSetC(cpu, res > 0xffff);
      break;
    }
    // LDI A,(HL): Load A from address pointed to by HL, and increment HL
    case 0x2A: cpu->registers.a = ReadByte(gameboy, cpu->registers.hl++); break;
    // DEC HL: Decrement 16-bit HL
    case 0x2B: cpu->registers.hl--; break;
    // INC L: Increment L
    case 0x2C: IncrementByteRegister(cpu, &cpu->registers.l); break;
    // DEC L: Decrement L
    case 0x2D: DecrementByteRegister(cpu, &cpu->registers.l); break;
    // LD L,n: Load 8-bit immediate into L
    case 0x2E: cpu->registers.l = instruction.operands[0]; break;
    // CPL: Complement (logical NOT) on A
    case 0x2F: {
      cpu->registers.a = (uint8_t)(~cpu->registers.a);

      CPUFlagsSetN(cpu);
      CPUFlagsSetH(cpu);
      break;
    }
    // JR NC,n: Relative jump by signed immediate if last result caused no carry
    case 0x30: {
      if (CPUFlagsGetC(*cpu) != 0)
        cpu->registers.pc += (int8_t)instruction.operands[0];
      break;
    }
    // LD SP,nn: Load 16-bit immediate into SP
    case 0x31: cpu->registers.sp = instruction.operand; break;
    // LDD (HL),A: Save A to address pointed by HL, and decrement HL
    case 0x32: WriteByte(gameboy, cpu->registers.hl--, cpu->registers.a); break;
    // INC SP: Increment 16-bit HL
    case 0x33: cpu->registers.sp++; break;
    // INC (HL): Increment value pointed by HL
    case 0x34: {
      uint8_t val = ReadByte(gameboy, cpu->registers.hl) + 1;
      WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // DEC (HL): Decrement value pointed by HL
    case 0x35: {
      uint8_t val = ReadByte(gameboy, cpu->registers.hl) - 1;
      WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // LD (HL),n: Load 8-bit immediate into address pointed by HL
    case 0x36: WriteByte(gameboy, cpu->registers.hl, instruction.operands[0]); break;
    // SCF: Set carry flag
    case 0x37: {
      CPUFlagsClearN(cpu);
      CPUFlagsClearH(cpu);
      CPUFlagsSetC(cpu);
      break;
    }
    // JR C,n: Relative jump by signed immediate if last result caused carry
    case 0x38: {
      if (CPUFlagsGetC(*cpu) != 0)
        cpu->registers.pc += (int8_t)instruction.operands[0];
      break;
    }
    // ADD HL,SP: Add 16-bit SP to HL
    case 0x39: {
      uint8_t initial_h = cpu->registers.h;
      uint32_t res = cpu->registers.hl + cpu->registers.sp;
      cpu->registers.hl += cpu->registers.sp;

      CPUFlagsClearN(cpu);
      CPUFlagsSetH(cpu, ((cpu->registers.h ^ cpu->registers.b ^ initial_h) & 0x10));
      CPUFlagsSetC(cpu, res > 0xffff);
      break;
    }
    // LDD A,(HL): Load A from address pointed to by HL, and decrement HL
    case 0x3A: cpu->registers.a = ReadByte(gameboy, cpu->registers.hl--); break;
    // DEC SP: Decrement 16-bit SP
    case 0x3B: cpu->registers.sp--; break;
    // INC A: Increment A
    case 0x3C: IncrementByteRegister(cpu, &cpu->registers.a); break;
    // DEC A: Decrement A
    case 0x3D: DecrementByteRegister(cpu, &cpu->registers.a); break;
    // LD A,n: Load 8-bit immediate into A
    case 0x3E: cpu->registers.a = instruction.operands[0]; break;
    // CCF: Complement Carry Flag
    case 0x3F: {
      CPUFlagsClearN(cpu);
      CPUFlagsClearH(cpu);
      CPUFlagsSetC(cpu, !CPUFlagsGetC(*cpu));
      break;
    }
    // LD B,B: Copy B to B
    case 0x40: break;
    // LD B,C: Copy C to B
    case 0x41: cpu->registers.b = cpu->registers.c; break;
    // LD B,D: Copy D to B
    case 0x42: cpu->registers.b = cpu->registers.d; break;
    // LD B,E: Copy E to B
    case 0x43: cpu->registers.b = cpu->registers.e; break;
    // LD B,H: Copy H to B
    case 0x44: cpu->registers.b = cpu->registers.h; break;
    // LD B,L: Copy L to B
    case 0x45: cpu->registers.b = cpu->registers.l; break;
    // LD B,(HL): Copy value pointed by HL to B
    case 0x46: cpu->registers.b = ReadByte(gameboy, cpu->registers.hl); break;
    // LD B,A: Copy A to B
    case 0x47: cpu->registers.b = cpu->registers.a; break;
    // LD C,B: Copy B to
    case 0x48: cpu->registers.c = cpu->registers.b; break;
    // LD C,C: Copy C to C
    case 0x49: break;
    // LD C,D: Copy D to
    case 0x4A: cpu->registers.c = cpu->registers.d; break;
    // LD C,E: Copy E to C
    case 0x4B: cpu->registers.c = cpu->registers.e; break;
    // LD C,H: Copy H to C
    case 0x4C: cpu->registers.c = cpu->registers.h; break;
    // LD C,L: Copy L to C
    case 0x4D: cpu->registers.c = cpu->registers.l; break;
    // LD C,(HL): Copy value pointed by HL to C
    case 0x4E: cpu->registers.c = ReadByte(gameboy, cpu->registers.hl); break;
    // LD C,A: Copy A to C
    case 0x4F: cpu->registers.c = cpu->registers.a; break;
    // LD D,B: Copy B to D
    case 0x50: cpu->registers.d = cpu->registers.b; break;
    // LD D,C: Copy C to D
    case 0x51: cpu->registers.d = cpu->registers.c; break;
    // LD D,D: Copy D to D
    case 0x52: break;
    // LD D,E: Copy E to D
    case 0x53: cpu->registers.d = cpu->registers.e; break;
    // LD D,H: Copy H to D
    case 0x54: cpu->registers.d = cpu->registers.h; break;
    // LD D,L: Copy L to D
    case 0x55: cpu->registers.d = cpu->registers.l; break;
    // LD D,(HL): Copy value pointed by HL to D
    case 0x56: cpu->registers.d = ReadByte(gameboy, cpu->registers.hl); break;
    // LD D,A: Copy A to D
    case 0x57: cpu->registers.d = cpu->registers.a; break;
    // LD E,B: Copy B to E
    case 0x58: cpu->registers.e = cpu->registers.b; break;
    // LD E,C: Copy C to E
    case 0x59: cpu->registers.e = cpu->registers.c; break;
    // LD E,D: Copy D to E
    case 0x5A: cpu->registers.e = cpu->registers.d; break;
    // LD E,E: Copy E to E
    case 0x5B: break;
    // LD E,H: Copy H to E
    case 0x5C: cpu->registers.e = cpu->registers.h; break;
    // LD E,L: Copy L to E
    case 0x5D: cpu->registers.e = cpu->registers.l; break;
    // LD E,(HL): Copy value pointed by HL to E
    case 0x5E: cpu->registers.e = ReadByte(gameboy, cpu->registers.hl); break;
    // LD E,A: Copy A to E
    case 0x5F: cpu->registers.e = cpu->registers.a; break;
    // LD H,B: Copy B to H
    case 0x60: cpu->registers.h = cpu->registers.b; break;
    // LD H,C: Copy C to H
    case 0x61: cpu->registers.h = cpu->registers.c; break;
    // LD H,D: Copy D to H
    case 0x62: cpu->registers.h = cpu->registers.d; break;
    // LD H,E: Copy E to H
    case 0x63: cpu->registers.h = cpu->registers.e; break;
    // LD H,H: Copy H to H
    case 0x64: break;
    // LD H,L: Copy L to H
    case 0x65: cpu->registers.h = cpu->registers.l; break;
    // LD H,(HL): Copy value pointed by HL to H
    case 0x66: cpu->registers.h = ReadByte(gameboy, cpu->registers.hl); break;
    // LD H,A: Copy A to H
    case 0x67: cpu->registers.h = cpu->registers.a; break;
    // LD L,B: Copy B to L
    case 0x68: cpu->registers.l = cpu->registers.b; break;
    // LD L,C: Copy C to L
    case 0x69: cpu->registers.l = cpu->registers.c; break;
    // LD L,D: Copy D to L
    case 0x6A: cpu->registers.l = cpu->registers.d; break;
    // LD L,E: Copy E to L
    case 0x6B: cpu->registers.l = cpu->registers.e; break;
    // LD L,H: Copy H to L
    case 0x6C: cpu->registers.l = cpu->registers.h; break;
    // LD L,L: Copy L to L
    case 0x6D: break;
    // LD L,(HL): Copy value pointed by HL to L
    case 0x6E: cpu->registers.l = ReadByte(gameboy, cpu->registers.hl); break;
    // LD L,A: Copy A to L
    case 0x6F: cpu->registers.l = cpu->registers.a; break;
    // LD (HL),B: Copy B to address pointed by HL
    case 0x70: WriteByte(gameboy, cpu->registers.hl, cpu->registers.b); break;
    // LD (HL),C: Copy C to address pointed by HL
    case 0x71: WriteByte(gameboy, cpu->registers.hl, cpu->registers.c); break;
    // LD (HL),D: Copy D to address pointed by HL
    case 0x72: WriteByte(gameboy, cpu->registers.hl, cpu->registers.d); break;
    // LD (HL),E: Copy E to address pointed by HL
    case 0x73: WriteByte(gameboy, cpu->registers.hl, cpu->registers.e); break;
    // LD (HL),H: Copy H to address pointed by HL
    case 0x74: WriteByte(gameboy, cpu->registers.hl, cpu->registers.h); break;
    // LD (HL),L: Copy L to address pointed by HL
    case 0x75: WriteByte(gameboy, cpu->registers.hl, cpu->registers.l); break;
    // HALT: Halt processor
    case 0x76: {
      /* if (cpu._interruptController.InterruptMasterEnable) { */
      /*   cpu.Halted = true; */
      /* } else { */
      /*   cpu.Halted = true; */
      /*   cpu.HaltLoad = true; */
      /* } */
      NOT_IMPLEMENTED();
      break;
    }
    // LD (HL),A: Copy A to address pointed by HL
    case 0x77: WriteByte(gameboy, cpu->registers.hl, cpu->registers.a); break;
    // LD A,B: Copy B to A
    case 0x78: cpu->registers.a = cpu->registers.b; break;
    // LD A,C: Copy C to A
    case 0x79: cpu->registers.a = cpu->registers.c; break;
    // LD A,D: Copy D to A
    case 0x7A: cpu->registers.a = cpu->registers.d; break;
    // LD A,E: Copy E to A
    case 0x7B: cpu->registers.a = cpu->registers.e; break;
    // LD A,H: Copy H to A
    case 0x7C: cpu->registers.a = cpu->registers.h; break;
    // LD A,L: Copy L to A
    case 0x7D: cpu->registers.a = cpu->registers.l; break;
    // LD A,(HL): Copy value pointed by HL to A
    case 0x7E: cpu->registers.a = ReadByte(gameboy, cpu->registers.hl); break;
    // LD A,A: Copy A to A
    case 0x7F: break;
    // ADD A,B: Add B to A
    case 0x80: Add(cpu, &cpu->registers.a, cpu->registers.b); break;
    // ADD A,C: Add C to A
    case 0x81: Add(cpu, &cpu->registers.a, cpu->registers.c); break;
    // ADD A,D: Add D to A
    case 0x82: Add(cpu, &cpu->registers.a, cpu->registers.d); break;
    // ADD A,E: Add E to A
    case 0x83: Add(cpu, &cpu->registers.a, cpu->registers.e); break;
    // ADD A,H: Add H to A
    case 0x84: Add(cpu, &cpu->registers.a, cpu->registers.h); break;
    // ADD A,L: Add L to A
    case 0x85: Add(cpu, &cpu->registers.a, cpu->registers.l); break;
    // ADD A,(HL): Add value pointed by HL to A
    case 0x86: Add(cpu, &cpu->registers.a, ReadByte(gameboy, cpu->registers.hl)); break;
    // ADD A,A: Add A to A
    case 0x87: Add(cpu, &cpu->registers.a, cpu->registers.a); break;
    // ADC A,B: Add B and carry flag to A
    case 0x88: Adc(cpu, &cpu->registers.a, cpu->registers.b); break;
    // ADC A,C: Add C and carry flag to A
    case 0x89: Adc(cpu, &cpu->registers.a, cpu->registers.c); break;
    // ADC A,D: Add D and carry flag to A
    case 0x8A: Adc(cpu, &cpu->registers.a, cpu->registers.d); break;
    // ADC A,E: Add E and carry flag to A
    case 0x8B: Adc(cpu, &cpu->registers.a, cpu->registers.e); break;
    // ADC A,H: Add H and carry flag to A
    case 0x8C: Adc(cpu, &cpu->registers.a, cpu->registers.h); break;
    // ADC A,L: Add and carry flag L to A
    case 0x8D: Adc(cpu, &cpu->registers.a, cpu->registers.l); break;
    // ADC A,(HL): Add value pointed by HL and carry flag to A
    case 0x8E: Adc(cpu, &cpu->registers.a, ReadByte(gameboy, cpu->registers.hl)); break;
    // ADC A,A: Add A and carry flag to A
    case 0x8F: Adc(cpu, &cpu->registers.a, cpu->registers.a); break;
    // SUB A,B: Subtract B from A
    case 0x90: SBC(cpu, &cpu->registers.a, cpu->registers.b, 0); break;
    // SUB A,C: Subtract C from A
    case 0x91: SBC(cpu, &cpu->registers.a, cpu->registers.c, 0); break;
    // SUB A,D: Subtract D from A
    case 0x92: SBC(cpu, &cpu->registers.a, cpu->registers.d, 0); break;
    // SUB A,E: Subtract E from A
    case 0x93: SBC(cpu, &cpu->registers.a, cpu->registers.e, 0); break;
    // SUB A,H: Subtract H from A
    case 0x94: SBC(cpu, &cpu->registers.a, cpu->registers.h, 0); break;
    // SUB A,L: Subtract L from A
    case 0x95: SBC(cpu, &cpu->registers.a, cpu->registers.l, 0); break;
    // SUB A,(HL): Subtract value pointed by HL from A
    case 0x96: SBC(cpu, &cpu->registers.a, ReadByte(gameboy, cpu->registers.hl), 0); break;
    // SUB A,A: Subtract A from A
    case 0x97: SBC(cpu, &cpu->registers.a, cpu->registers.a, 0); break;
    // SBC A,B: Subtract B and carry flag from A
    case 0x98: SBC(cpu, &cpu->registers.a, cpu->registers.b, CPUFlagsGetC(*cpu)); break;
    // SBC A,C: Subtract C and carry flag from A
    case 0x99: SBC(cpu, &cpu->registers.a, cpu->registers.c, CPUFlagsGetC(*cpu)); break;
    // SBC A,D: Subtract D and carry flag from A
    case 0x9A: SBC(cpu, &cpu->registers.a, cpu->registers.d, CPUFlagsGetC(*cpu)); break;
    // SBC A,E: Subtract E and carry flag from A
    case 0x9B: SBC(cpu, &cpu->registers.a, cpu->registers.e, CPUFlagsGetC(*cpu)); break;
    // SBC A,H: Subtract H and carry flag from A
    case 0x9C: SBC(cpu, &cpu->registers.a, cpu->registers.h, CPUFlagsGetC(*cpu)); break;
    // SBC A,L: Subtract and carry flag L from A
    case 0x9D: SBC(cpu, &cpu->registers.a, cpu->registers.l, CPUFlagsGetC(*cpu)); break;
    // SBC A,(HL): Subtract value pointed by HL and carry flag from A
    case 0x9E: {
      SBC(cpu, &cpu->registers.a, ReadByte(gameboy, cpu->registers.hl), CPUFlagsGetC(*cpu));
      break;
    }
    // SBC A,A: Subtract A and carry flag from A
    case 0x9F: SBC(cpu, &cpu->registers.a, cpu->registers.a, CPUFlagsGetC(*cpu)); break;
    // AND B: Logical AND B against A
    case 0xA0: And(cpu, &cpu->registers.a, cpu->registers.b); break;
    // AND C: Logical AND C against A
    case 0xA1: And(cpu, &cpu->registers.a, cpu->registers.c); break;
    // AND D: Logical AND D against A
    case 0xA2: And(cpu, &cpu->registers.a, cpu->registers.d); break;
    // AND E: Logical AND E against A
    case 0xA3: And(cpu, &cpu->registers.a, cpu->registers.e); break;
    // AND H: Logical AND H against A
    case 0xA4: And(cpu, &cpu->registers.a, cpu->registers.h); break;
    // AND L: Logical AND L against A
    case 0xA5: And(cpu, &cpu->registers.a, cpu->registers.l); break;
    // AND (HL): Logical AND value pointed by HL against A
    case 0xA6: And(cpu, &cpu->registers.a, ReadByte(gameboy, cpu->registers.hl)); break;
    // AND A: Logical AND A against A
    case 0xA7: And(cpu, &cpu->registers.a, cpu->registers.a); break;
    // XOR B: Logical XOR B against A
    case 0xA8: Xor(cpu, &cpu->registers.a, cpu->registers.b); break;
    // XOR C: Logical XOR C against A
    case 0xA9: Xor(cpu, &cpu->registers.a, cpu->registers.c); break;
    // XOR D: Logical XOR D against A
    case 0xAA: Xor(cpu, &cpu->registers.a, cpu->registers.d); break;
    // XOR E: Logical XOR E against A
    case 0xAB: Xor(cpu, &cpu->registers.a, cpu->registers.e); break;
    // XOR H: Logical XOR H against A
    case 0xAC: Xor(cpu, &cpu->registers.a, cpu->registers.h); break;
    // XOR L: Logical XOR L against A
    case 0xAD: Xor(cpu, &cpu->registers.a, cpu->registers.l); break;
    // XOR (HL): Logical XOR value pointed by HL against A
    case 0xAE: Xor(cpu, &cpu->registers.a, ReadByte(gameboy, cpu->registers.hl)); break;
    // XOR A: Logical XOR A against A
    case 0xAF: Xor(cpu, &cpu->registers.a, cpu->registers.a); break;
    // OR B: Logical OR B against A
    case 0xB0: Or(cpu, &cpu->registers.a, cpu->registers.b); break;
    // OR C: Logical OR C against A
    case 0xB1: Or(cpu, &cpu->registers.a, cpu->registers.c); break;
    // OR D: Logical OR D against A
    case 0xB2: Or(cpu, &cpu->registers.a, cpu->registers.d); break;
    // OR E: Logical OR E against A
    case 0xB3: Or(cpu, &cpu->registers.a, cpu->registers.e); break;
    // OR H: Logical OR H against A
    case 0xB4: Or(cpu, &cpu->registers.a, cpu->registers.h); break;
    // OR L: Logical OR L against A
    case 0xB5: Or(cpu, &cpu->registers.a, cpu->registers.l); break;
    // OR (HL): Logical OR value pointed by HL against A
    case 0xB6: Or(cpu, &cpu->registers.a, ReadByte(gameboy, cpu->registers.hl)); break;
    // OR A: Logical OR A against A
    case 0xB7: Or(cpu, &cpu->registers.a, cpu->registers.a); break;
    // CP B: Compare B against A
    case 0xB8: Cp(cpu, &cpu->registers.a, cpu->registers.b); break;
    // CP C: Compare C against A
    case 0xB9: Cp(cpu, &cpu->registers.a, cpu->registers.c); break;
    // CP D: Compare D against A
    case 0xBA: Cp(cpu, &cpu->registers.a, cpu->registers.d); break;
    // CP E: Compare E against A
    case 0xBB: Cp(cpu, &cpu->registers.a, cpu->registers.e); break;
    // CP H: Compare H against A
    case 0xBC: Cp(cpu, &cpu->registers.a, cpu->registers.h); break;
    // CP L: Compare L against A
    case 0xBD: Cp(cpu, &cpu->registers.a, cpu->registers.l); break;
    // CP (HL): Compare value pointed by HL against A
    case 0xBE: Cp(cpu, &cpu->registers.a, ReadByte(gameboy, cpu->registers.hl)); break;
    // CP A: Compare A against A
    case 0xBF: Cp(cpu, &cpu->registers.a, cpu->registers.a); break;
    // RET NZ: Return if last result was not zero
    case 0xC0: {
      if (CPUFlagsGetZ(*cpu) == 0)
        break;
      Ret(gameboy);
      break;
    }
    // POP BC: Pop 16-bit value from stack into BC
    case 0xC1: Pop(gameboy, &cpu->registers.bc); break;
    // JP NZ,nn: Absolute jump to 16-bit location if last result was not zero
    case 0xC2: {
      if (CPUFlagsGetZ(*cpu) == 0)
        break;

      cpu->registers.pc = instruction.operand;
      break;
    }
    // JP nn: Absolute jump to 16-bit location
    case 0xC3: cpu->registers.pc = instruction.operand; break;
    // CALL NZ,nn: Call routine at 16-bit location if last result was not zero
    case 0xC4: {
      if (CPUFlagsGetZ(*cpu) == 0)
        break;
      Call(gameboy, instruction.operand);
      break;
    }
    // PUSH BC: Push 16-bit BC onto stack
    case 0xC5: Push(gameboy, cpu->registers.bc); break;
    // ADD A,n: Add 8-bit immediate to A
    case 0xC6: Add(cpu, &cpu->registers.a, instruction.operands[0]); break;
    // RST 0: Call routine at address 0000h
    case 0xC7: Call(gameboy, 0x0000); break;
    // RET Z: Return if last result was zero
    case 0xC8: {
      if (CPUFlagsGetZ(*cpu) != 0)
        break;
      Ret(gameboy);
      break;
    }
    // RET: Return to calling routine
    case 0xC9: Ret(gameboy); break;
    // JP Z,nn: Absolute jump to 16-bit location if last result was zero
    case 0xCA: {
      if (CPUFlagsGetZ(*cpu) != 0)
        break;
      cpu->registers.pc = instruction.operand;
      break;
    }
    // Ext ops: Extended operations (two-byte instruction code)
    case 0xCB: NOT_REACHED(); break;
    // CALL Z,nn: Call routine at 16-bit location if last result was zero
    case 0xCC: {
      if (CPUFlagsGetZ(*cpu) != 0)
        break;
      Call(gameboy, instruction.operand);
      break;
    }
    // CALL nn: Call routine at 16-bit location
    case 0xCD: Call(gameboy, instruction.operand); break;
    // ADC A,n: Add 8-bit immediate and carry to A
    case 0xCE: Adc(cpu, &cpu->registers.a, instruction.operands[0]); break;
    // RST 8: Call routine at address 0008h
    case 0xCF: Call(gameboy, 0x08); break;
    // RET NC: Return if last result caused no carry
    case 0xD0: {
      if (CPUFlagsGetC(*cpu) != 0)
        break;
      Ret(gameboy);
      break;
    }
    // POP DE: Pop 16-bit value from stack into DE
    case 0xD1: Pop(gameboy, &cpu->registers.de); break;
    // JP NC,nn: Absolute jump to 16-bit location if last result caused no carry
    case 0xD2: {
      if (CPUFlagsGetC(*cpu) == 0)
        break;
      cpu->registers.pc = instruction.operand;
    }
    // XX: Operation removed in cpu CPU
    case 0xD3: NOT_REACHED(); break;
    // CALL NC,nn: Call routine at 16-bit location if last result caused no carry
    case 0xD4: {
      if (CPUFlagsGetC(*cpu) != 0)
        break;
      Call(gameboy, instruction.operand);
    }
    // PUSH DE: Push 16-bit DE onto stack
    case 0xD5: Push(gameboy, cpu->registers.de); break;
    // SUB A,n: Subtract 8-bit immediate from A
    case 0xD6: Sub(cpu, &cpu->registers.a, instruction.operands[0]); break;
    // RST 10: Call routine at address 0010h
    case 0xD7: Call(gameboy, 0x10); break;
    // RET C: Return if last result caused carry
    case 0xD8: {
      if (CPUFlagsGetC(*cpu) == 0)
        break;
      Ret(gameboy);
    }
    // RETI: Enable interrupts and return to calling routine
    case 0xD9: {
      /* cpu._interruptController.InterruptMasterEnable = true; */

      /* // We load the program counter (high byte is in higher address) */
      /* cpu.NextPC = cpu._memory.Read(cpu->registers.sp++); */
      /* cpu.NextPC += (ushort)(cpu._memory.Read(cpu->registers.sp++) << 8); */
      /* break; */
      NOT_IMPLEMENTED();
      break;
    }
    // JP C,nn: Absolute jump to 16-bit location if last result caused carry
    case 0xDA: {
      if (CPUFlagsGetC(*cpu) == 0)
        break;
      cpu->registers.pc = instruction.operand;
      break;
    }
    // XX: Operation removed in cpu CPU
    case 0xDB: NOT_REACHED(); break;
    // CALL C,nn: Call routine at 16-bit location if last result caused carry
    case 0xDC: {
      if (CPUFlagsGetC(*cpu) == 0)
        break;
      Call(gameboy, instruction.operand);
      break;
    }
    // XX: Operation removed in cpu CPU
    case 0xDD: NOT_REACHED(); break;
    // SBC A,n: Subtract 8-bit immediate and carry from A
    case 0xDE: SBC(cpu, &cpu->registers.a, instruction.operands[0], CPUFlagsGetC(*cpu)); break;
    // RST 18: Call routine at address 0018h
    case 0xDF: Call(gameboy, 0x18); break;
    // LDH (n),A: Save A at address pointed to by (FF00h + 8-bit immediate)
    case 0xE0: Ldh(gameboy, instruction.operands[0], cpu->registers.a); break;
    // POP HL: Pop 16-bit value from stack into HL
    case 0xE1: Pop(gameboy, &cpu->registers.hl); break;
    // LDH (C),A: Save A at address pointed to by (FF00h + C)
    case 0xE2: Ldh(gameboy, cpu->registers.c, cpu->registers.a); break;
    // XX: Operation removed in cpu CPU
    case 0xE3: NOT_REACHED(); break;
    // XX: Operation removed in cpu CPU
    case 0xE4: NOT_REACHED(); break;
    // PUSH HL: Push 16-bit HL onto stack
    case 0xE5: Push(gameboy, cpu->registers.hl); break;
    // AND n: Logical AND 8-bit immediate against A
    case 0xE6: And(cpu, &cpu->registers.a, instruction.operands[0]); break;
    // RST 20: Call routine at address 0020h
    case 0xE7: Call(gameboy, 0x20); break;
    // ADD SP,d: Add signed 8-bit immediate to SP
    case 0xE8: {
      int8_t s = (int8_t)instruction.operands[0];

      // We set the flags.
      CPUFlagsClearZ(cpu);
      CPUFlagsClearN(cpu);

      int set_h = ((cpu->registers.sp & 0x0f) + (s & 0x0f) > 0x0f);
      CPUFlagsSetH(cpu, set_h);

      int set_c = ((cpu->registers.sp & 0xff) + (s & 0xff) > 0xff);
      CPUFlagsSetC(cpu, set_c);

      cpu->registers.sp += s;
      break;
    }
    // JP (HL): Jump to 16-bit value pointed by HL
    case 0xE9: cpu->registers.pc = cpu->registers.hl; break;
    // LD (nn),A: Save A at given 16-bit address
    case 0xEA: WriteByte(gameboy, instruction.operand, cpu->registers.a); break;
    // XX: Operation removed in cpu CPU
    case 0xEB: NOT_REACHED(); break;
    // XX: Operation removed in cpu CPU
    case 0xEC: NOT_REACHED(); break;
    // XX: Operation removed in cpu CPU
    case 0xED: NOT_REACHED(); break;
    // XOR n: Logical XOR 8-bit immediate against A
    case 0xEE: Xor(cpu, &cpu->registers.a, instruction.operands[0]); break;
    // RST 28: Call routine at address 0028h
    case 0xEF: Call(gameboy, 0x28); break;
    // LDH A,(n): Load A from address pointed to by (FF00h + 8-bit immediate)
    case 0xF0: {
      uint16_t address = (uint16_t)(0xff00 | instruction.operands[0]);
      cpu->registers.a = ReadByte(gameboy, address);
      break;
    }
    // POP AF: Pop 16-bit value from stack into AF
    case 0xF1: Pop(gameboy, &cpu->registers.af); break;
    // LDH A, (C): Operation removed in cpu CPU? (Or Load into A cpu.memory from FF00 + C?)
    case 0xF2: {
      uint16_t address = (uint16_t)(0xff00 | cpu->registers.c);
      cpu->registers.a = ReadByte(gameboy, address);
      break;
    }
    // DI: DIsable interrupts
    case 0xF3: {
      /* cpu._interruptController.InterruptMasterEnable = false; */
      NOT_IMPLEMENTED();
      break;
    }
    // XX: Operation removed in cpu CPU
    case 0xF4: NOT_REACHED(); break;
    // PUSH AF: Push 16-bit AF onto stack
    case 0xF5: Push(gameboy, cpu->registers.af); break;
    // OR n: Logical OR 8-bit immediate against A
    case 0xF6: Or(cpu, &cpu->registers.a, instruction.operands[0]); break;
    // RST 30: Call routine at address 0030h
    case 0xF7: Call(gameboy, 0x30); break;
    // LDHL SP,d: Add signed 8-bit immediate to SP and save result in HL
    case 0xF8: {
      int8_t s = (int8_t)instruction.operands[0];

      // We set the flags.
      CPUFlagsClearZ(cpu);
      CPUFlagsClearN(cpu);

      int set_h = ((cpu->registers.sp & 0x0f) + (s & 0x0f) > 0x0f);
      CPUFlagsSetH(cpu, set_h);

      int set_c = ((cpu->registers.sp & 0xff) + (s & 0xff) > 0xff);
      CPUFlagsSetC(cpu, set_c);

      // We make the sum
      cpu->registers.hl = (uint16_t)(cpu->registers.sp + s);
      break;
    }
    // LD SP,HL: Copy HL to SP
    case 0xF9: cpu->registers.sp = cpu->registers.hl; break;
    // LD A,(nn): Load A from given 16-bit address
    case 0xFA: cpu->registers.a = ReadByte(gameboy, instruction.operand); break;
    // EI: Enable interrupts
    case 0xFB: {
      /* cpu._interruptController.InterruptMasterEnable = true; */
      NOT_IMPLEMENTED();
      break;
    }
    // XX: Operation removed in cpu CPU
    case 0xFC: NOT_REACHED(); break;
    // XX: Operation removed in cpu CPU
    case 0xFD: NOT_REACHED(); break;
    // CP n: Compare 8-bit immediate against A
    case 0xFE: Cp(cpu, &cpu->registers.a, instruction.operands[0]); break;
    // RST 38: Call routine at address 0038h
    case 0xFF: Call(gameboy, 0x38); break;
  }
}

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
      uint8_t val = ReadByte(gameboy, address);
      RotateLeftAndCarry(cpu, &val);
      WriteByte(gameboy, address, val);
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
      uint8_t val = ReadByte(gameboy, address);
      RotateRightAndCarry(cpu, &val);
      WriteByte(gameboy, address, val);
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
      uint8_t val = ReadByte(gameboy, address);
      RotateLeftThroughCarry(cpu, &val, 1, CPUFlagsGetC(*cpu));
      WriteByte(gameboy, address, val);
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
      uint8_t val = ReadByte(gameboy, address);
      RotateRightThroughCarry(cpu, &val, 1, CPUFlagsGetC(*cpu));
      WriteByte(gameboy, address, val);
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
      uint8_t val = ReadByte(gameboy, address);
      ShiftLeft(cpu, &val);
      WriteByte(gameboy, address, val);
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
      uint8_t val = ReadByte(gameboy, address);
      ShiftRightArithmetic(cpu, &val);
      WriteByte(gameboy, address, val);
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
      uint8_t val = ReadByte(gameboy, address);
      SwapNibbles(cpu, &val);
      WriteByte(gameboy, address, val);
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
      uint8_t val = ReadByte(gameboy, address);
      ShiftRightLogic(cpu, &val);
      WriteByte(gameboy, address, val);
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
    case 0x46: TestBit(cpu, ReadByte(gameboy, cpu->registers.hl), 0); break;
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
    case 0x4E: TestBit(cpu, ReadByte(gameboy, cpu->registers.hl), 1); break;
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
    case 0x56: TestBit(cpu, ReadByte(gameboy, cpu->registers.hl), 2); break;
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
    case 0x5E: TestBit(cpu, ReadByte(gameboy, cpu->registers.hl), 3); break;
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
    case 0x66: TestBit(cpu, ReadByte(gameboy, cpu->registers.hl), 4); break;
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
    case 0x6E: TestBit(cpu, ReadByte(gameboy, cpu->registers.hl), 5); break;
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
    case 0x76: TestBit(cpu, ReadByte  (gameboy, cpu->registers.hl), 6); break;
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
    case 0x7E: TestBit(cpu, ReadByte  (gameboy, cpu->registers.hl), 7); break;
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      ClearBit(&val, 0);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      ClearBit(&val, 1);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      ClearBit(&val, 2);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      ClearBit(&val, 3);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      ClearBit(&val, 4);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      ClearBit(&val, 5);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      ClearBit(&val, 6);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      ClearBit(&val, 7);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      SetBit(&val, 0);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      SetBit(&val, 1);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      SetBit(&val, 2);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      SetBit(&val, 3);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      SetBit(&val, 4);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      SetBit(&val, 5);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      SetBit(&val, 6);
      WriteByte(gameboy, cpu->registers.hl, val);
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
      uint8_t val = ReadByte(gameboy, cpu->registers.hl);
      SetBit(&val, 7);
      WriteByte(gameboy, cpu->registers.hl, val);
      break;
    }
    // SET 7,A: Set bit 7 of A
    case 0xFF: SetBit(&cpu->registers.a, 7); break;
  }
}

}  // namespace

void ExecuteInstruction(Gameboy* gameboy, const Instruction& instruction) {
  if (!IsCBInstruction(instruction)) {
    ExecuteNormalInstruction(gameboy, instruction);
  } else {
    ExecuteCBInstruction(gameboy, instruction);
  }
}

}  // namespace emulator
}  // namespace rothko
