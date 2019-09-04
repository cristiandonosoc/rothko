// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "cpu.h"
#include "cpu_instructions.h"

#include <rothko/logging/logging.h>

namespace rothko {
namespace emulator {

namespace {

// Instruction Lenghts -----------------------------------------------------------------------------

// clang-format off
uint8_t kNormalLengths[0x100] = {
/*        0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f  */
/* 0 */   1,  3,  1,  1,  1,  1,  2,  1,  3,  1,  1,  1,  1,  1,  2,  1,
/* 1 */   2,  3,  1,  1,  1,  1,  2,  1,  2,  1,  1,  1,  1,  1,  2,  1,
/* 2 */   2,  3,  1,  1,  1,  1,  2,  1,  2,  1,  1,  1,  1,  1,  2,  1,
/* 3 */   2,  3,  1,  1,  1,  1,  2,  1,  2,  1,  1,  1,  1,  1,  2,  1,
/* 4 */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 5 */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 6 */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 7 */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 8 */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 9 */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* a */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* b */   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* c */   1,  1,  3,  3,  3,  1,  2,  1,  1,  1,  3,  2,  3,  3,  2,  1,
/* d */   1,  1,  3,  0,  3,  1,  2,  1,  1,  1,  3,  0,  3,  0,  2,  1,
/* e */   2,  1,  1,  0,  1,  1,  2,  1,  2,  1,  3,  0,  0,  0,  2,  1,
/* f */   2,  1,  1,  1,  0,  1,  2,  1,  2,  1,  3,  1,  0,  0,  2,  1,
};
static_assert(sizeof(kNormalLengths) == 256);
// clang-format on

// clang-format off
uint8_t kCBLengths[0x100] = {
/*        0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f  */
/* 0 */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* 1 */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* 2 */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* 3 */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* 4 */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* 5 */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* 6 */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* 7 */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* 8 */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* 9 */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* a */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* b */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* c */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* d */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* e */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* f */   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
};
static_assert(sizeof(kCBLengths) == 256);
// clang-format on

// Instruction Clocks ------------------------------------------------------------------------------

// clang-format off
uint8_t kNormalTicks[0x100] = {
/*        0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f  */
/* 0 */   4, 12,  8,  8,  4,  4,  8,  4, 20,  8,  8,  8,  4,  4,  8,  4,
/* 1 */   4, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4,
/* 2 */   8, 12,  8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4,
/* 3 */   8, 12,  8,  8, 12, 12, 12,  4,  8,  8,  8,  8,  4,  4,  8,  4,
/* 4 */   4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 5 */   4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 6 */   4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 7 */   8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
/* 8 */   4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* 9 */   4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* a */   4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* b */   4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/* c */   8, 12, 12, 16, 12, 16,  8, 16,  8, 16, 12,  4, 12, 24,  8, 16,
/* d */   8, 12, 12,  0, 12, 16,  8, 16,  8, 16, 12,  0, 12,  0,  8, 16,
/* e */  12, 12,  8,  0,  0, 16,  8, 16, 16,  4, 16,  0,  0,  0,  8, 16,
/* f */  12, 12,  8,  4,  0, 16,  8, 16, 12,  8, 16,  4,  0,  0,  8, 16,
};
static_assert(sizeof(kNormalTicks) == 256);
// clang-format on

// clang-format off
uint8_t kCBTicks[0x100] = {
/*        0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f  */
/* 0 */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* 1 */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* 2 */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* 3 */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* 4 */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* 5 */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* 6 */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* 7 */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* 8 */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* 9 */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* a */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* b */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* c */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* d */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* e */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
/* f */   8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
};
static_assert(sizeof(kCBTicks) == 256);
// clang-format on

}  // namespace

// GetConditionalTicks -----------------------------------------------------------------------------

namespace {

ConditionalTicks kNormalConditionalMasks[] = {
    // (0x20) JR NZ,n: Relative jump by signed immediate if last result was not zero.
    COND_TICKS_NEGATIVE_FLAG(Z, 4),

    // (0x28) JR Z,n: Relative jump by signed immediate if last result was zero.
    COND_TICKS_POSITIVE_FLAG(Z, 4),

    // (0x30) JR NC,n: Relative jump by signed immediate if last result caused no carry.
    COND_TICKS_NEGATIVE_FLAG(C, 4),

    // (0x38) JR C,n: Relative jump by signed immediate if last result caused carry.
    COND_TICKS_POSITIVE_FLAG(C, 4),

    // (0xc0) RET NZ: Return if last result was not zero.
    COND_TICKS_NEGATIVE_FLAG(Z, 12),

    // (0xc2) JP NZ,nn: Absolute jump to 16-bit location if last result was not zero.
    COND_TICKS_NEGATIVE_FLAG(Z, 4),

    // (0xc4) CALL NZ,nn: Call routine at 16-bit location if last result was not zero.
    COND_TICKS_NEGATIVE_FLAG(Z, 12),

    // (0xc8) RET Z: Return if last result was zero.
    COND_TICKS_POSITIVE_FLAG(Z, 12),

    // (0xca) JP Z,nn: Absolute jump to 16-bit location if last result was zero.
    COND_TICKS_POSITIVE_FLAG(Z, 4),

    // (0xcc) CALL Z,nn: Call routine at 16-bit location if last result was zero.
    COND_TICKS_POSITIVE_FLAG(Z, 12),

    // (0xd0) RET NC: Return if last result caused no carry.
    COND_TICKS_NEGATIVE_FLAG(C, 12),

    // (0xd2) JP NC,nn: Absolute jump to 16-bit location if last result caused no carry.
    COND_TICKS_NEGATIVE_FLAG(C, 4),

    // (0xd4) CALL NC,nn: Call routine at 16-bit location if last result caused no carry.
    COND_TICKS_NEGATIVE_FLAG(C, 12),

    // (0xd8) RET C: Return if last result caused carry.
    COND_TICKS_POSITIVE_FLAG(C, 12),

    // (0xda) JP C,nn: Absolute jump to 16-bit location if last result caused carry.
    COND_TICKS_POSITIVE_FLAG(C, 4),

    // (0xdc) CALL C,nn: Call routine at 16-bit location if last result caused carry.
    COND_TICKS_NEGATIVE_FLAG(C, 12),
};

}  // namespace

uint8_t GetConditionalTicks(const Instruction& instruction, uint8_t flags) {
  // CB instruction have no conditional ticks.
  if (IsCBInstruction(instruction))
    return 0;

  switch (instruction.opcode.low) {
    case 0x20: return GetConditionalTicks(kNormalConditionalMasks[0],  flags);
    case 0x28: return GetConditionalTicks(kNormalConditionalMasks[1],  flags);
    case 0x30: return GetConditionalTicks(kNormalConditionalMasks[2],  flags);
    case 0x38: return GetConditionalTicks(kNormalConditionalMasks[3],  flags);
    case 0xc0: return GetConditionalTicks(kNormalConditionalMasks[4],  flags);
    case 0xc2: return GetConditionalTicks(kNormalConditionalMasks[5],  flags);
    case 0xc4: return GetConditionalTicks(kNormalConditionalMasks[6],  flags);
    case 0xc8: return GetConditionalTicks(kNormalConditionalMasks[7],  flags);
    case 0xca: return GetConditionalTicks(kNormalConditionalMasks[8],  flags);
    case 0xcc: return GetConditionalTicks(kNormalConditionalMasks[9],  flags);
    case 0xd0: return GetConditionalTicks(kNormalConditionalMasks[10], flags);
    case 0xd2: return GetConditionalTicks(kNormalConditionalMasks[11], flags);
    case 0xd4: return GetConditionalTicks(kNormalConditionalMasks[12], flags);
    case 0xd8: return GetConditionalTicks(kNormalConditionalMasks[13], flags);
    case 0xda: return GetConditionalTicks(kNormalConditionalMasks[14], flags);
    case 0xdc: return GetConditionalTicks(kNormalConditionalMasks[15], flags);
    case 0xcb: {
      NOT_REACHED_MSG("Should not receive 0xcb prefix as a normal instruction.");
      return 0;
    }
    default: return 0;
  }
}

// The idea is that the masked bit has to be the same for the extra ticks be returned.
// For this we mask the bit, we XNOR (XOR then negate) and them mask that bit.
// 1 = The bits were the same. 0 = They were different.
uint8_t GetConditionalTicks(const ConditionalTicks& ct, uint8_t flags) {
  uint8_t result = (flags ^ ct.xnor_comparator);
  uint8_t not_result = (~result & ct.mask);
  return not_result != 0 ? ct.extra_ticks : 0;
}

// Fetch -------------------------------------------------------------------------------------------

namespace {

inline bool GetOpcode(Instruction* instruction, const uint8_t data[3]) {
  // The first byte will tell if it's a CB instruction.
  if (data[0] == 0xcb) {
    instruction->opcode.high = data[0];
    instruction->opcode.low = data[1];
    return true;
  }

  // Check if it's one of the invalid normal instructions.
  switch (data[0]) {
    case 0xD3:
    case 0xDB:
    case 0xDD:
    case 0xE3:
    case 0xE4:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xF4:
    case 0xFC:
    case 0xFD:
      return false;
  }

  instruction->opcode.opcode = data[0];
  return true;
}

inline void Decode(Instruction* instruction, const uint8_t data[3]) {
  bool is_cb = IsCBInstruction(*instruction);

  // Both normal and CB instruction will have the index opcode in |low|.
  uint8_t index = instruction->opcode.low;

  instruction->ticks = !is_cb ? kNormalTicks[index] : kCBTicks[index];
  instruction->length = !is_cb ? kNormalLengths[index] : kCBLengths[index];

  // cb instructions have no operands.
  if (is_cb)
    return;

  // IMPORTANT: This getting of the operands will only work on little endian machines.
  *(uint16_t*)instruction->operands = *(const uint16_t*)(data + 1);
}

}  // namespace

bool FetchAndDecode(Instruction* instruction, const uint8_t data[3]) {
  if (!GetOpcode(instruction, data))
    return false;

  Decode(instruction, data);

  return true;
}

// GetName -----------------------------------------------------------------------------------------

const char* GetName(const Instruction& instruction) {
  if (!IsCBInstruction(instruction)) {
    switch (instruction.opcode.low) {
      case 0x00: return "NOP";
      case 0x01: return "LD BC,nn";
      case 0x02: return "LD (BC),A";
      case 0x03: return "INC BC";
      case 0x04: return "INC B";
      case 0x05: return "DEC B";
      case 0x06: return "LD B,n";
      case 0x07: return "RLC A";
      case 0x08: return "LD (nn),SP";
      case 0x09: return "ADD HL,BC";
      case 0x0a: return "LD A,(BC)";
      case 0x0b: return "DEC BC";
      case 0x0c: return "INC C";
      case 0x0d: return "DEC C";
      case 0x0e: return "LD C,n";
      case 0x0f: return "RRC A";
      case 0x10: return "STOP";
      case 0x11: return "LD DE,nn";
      case 0x12: return "LD (DE),A";
      case 0x13: return "INC DE";
      case 0x14: return "INC D";
      case 0x15: return "DEC D";
      case 0x16: return "LD D,n";
      case 0x17: return "RL A";
      case 0x18: return "JR n";
      case 0x19: return "ADD HL,DE";
      case 0x1a: return "LD A,(DE)";
      case 0x1b: return "DEC DE";
      case 0x1c: return "INC E";
      case 0x1d: return "DEC E";
      case 0x1e: return "LD E,n";
      case 0x1f: return "RR A";
      case 0x20: return "JR NZ,n";
      case 0x21: return "LD HL,nn";
      case 0x22: return "LDI (HL),A";
      case 0x23: return "INC HL";
      case 0x24: return "INC H";
      case 0x25: return "DEC H";
      case 0x26: return "LD H,n";
      case 0x27: return "DAA";
      case 0x28: return "JR Z,n";
      case 0x29: return "ADD HL,HL";
      case 0x2a: return "LDI A,(HL)";
      case 0x2b: return "DEC HL";
      case 0x2c: return "INC L";
      case 0x2d: return "DEC L";
      case 0x2e: return "LD L,n";
      case 0x2f: return "CPL";
      case 0x30: return "JR NC,n";
      case 0x31: return "LD SP,nn";
      case 0x32: return "LDD (HL),A";
      case 0x33: return "INC SP";
      case 0x34: return "INC (HL)";
      case 0x35: return "DEC (HL)";
      case 0x36: return "LD (HL),n";
      case 0x37: return "SCF";
      case 0x38: return "JR C,n";
      case 0x39: return "ADD HL,SP";
      case 0x3a: return "LDD A,(HL)";
      case 0x3b: return "DEC SP";
      case 0x3c: return "INC A";
      case 0x3d: return "DEC A";
      case 0x3e: return "LD A,n";
      case 0x3f: return "CCF";
      case 0x40: return "LD B,B";
      case 0x41: return "LD B,C";
      case 0x42: return "LD B,D";
      case 0x43: return "LD B,E";
      case 0x44: return "LD B,H";
      case 0x45: return "LD B,L";
      case 0x46: return "LD B,(HL)";
      case 0x47: return "LD B,A";
      case 0x48: return "LD C,B";
      case 0x49: return "LD C,C";
      case 0x4a: return "LD C,D";
      case 0x4b: return "LD C,E";
      case 0x4c: return "LD C,H";
      case 0x4d: return "LD C,L";
      case 0x4e: return "LD C,(HL)";
      case 0x4f: return "LD C,A";
      case 0x50: return "LD D,B";
      case 0x51: return "LD D,C";
      case 0x52: return "LD D,D";
      case 0x53: return "LD D,E";
      case 0x54: return "LD D,H";
      case 0x55: return "LD D,L";
      case 0x56: return "LD D,(HL)";
      case 0x57: return "LD D,A";
      case 0x58: return "LD E,B";
      case 0x59: return "LD E,C";
      case 0x5a: return "LD E,D";
      case 0x5b: return "LD E,E";
      case 0x5c: return "LD E,H";
      case 0x5d: return "LD E,L";
      case 0x5e: return "LD E,(HL)";
      case 0x5f: return "LD E,A";
      case 0x60: return "LD H,B";
      case 0x61: return "LD H,C";
      case 0x62: return "LD H,D";
      case 0x63: return "LD H,E";
      case 0x64: return "LD H,H";
      case 0x65: return "LD H,L";
      case 0x66: return "LD H,(HL)";
      case 0x67: return "LD H,A";
      case 0x68: return "LD L,B";
      case 0x69: return "LD L,C";
      case 0x6a: return "LD L,D";
      case 0x6b: return "LD L,E";
      case 0x6c: return "LD L,H";
      case 0x6d: return "LD L,L";
      case 0x6e: return "LD L,(HL)";
      case 0x6f: return "LD L,A";
      case 0x70: return "LD (HL),B";
      case 0x71: return "LD (HL),C";
      case 0x72: return "LD (HL),D";
      case 0x73: return "LD (HL),E";
      case 0x74: return "LD (HL),H";
      case 0x75: return "LD (HL),L";
      case 0x76: return "HALT";
      case 0x77: return "LD (HL),A";
      case 0x78: return "LD A,B";
      case 0x79: return "LD A,C";
      case 0x7a: return "LD A,D";
      case 0x7b: return "LD A,E";
      case 0x7c: return "LD A,H";
      case 0x7d: return "LD A,L";
      case 0x7e: return "LD A,(HL)";
      case 0x7f: return "LD A,A";
      case 0x80: return "ADD A,B";
      case 0x81: return "ADD A,C";
      case 0x82: return "ADD A,D";
      case 0x83: return "ADD A,E";
      case 0x84: return "ADD A,H";
      case 0x85: return "ADD A,L";
      case 0x86: return "ADD A,(HL)";
      case 0x87: return "ADD A,A";
      case 0x88: return "ADC A,B";
      case 0x89: return "ADC A,C";
      case 0x8a: return "ADC A,D";
      case 0x8b: return "ADC A,E";
      case 0x8c: return "ADC A,H";
      case 0x8d: return "ADC A,L";
      case 0x8e: return "ADC A,(HL)";
      case 0x8f: return "ADC A,A";
      case 0x90: return "SUB A,B";
      case 0x91: return "SUB A,C";
      case 0x92: return "SUB A,D";
      case 0x93: return "SUB A,E";
      case 0x94: return "SUB A,H";
      case 0x95: return "SUB A,L";
      case 0x96: return "SUB A,(HL)";
      case 0x97: return "SUB A,A";
      case 0x98: return "SBC A,B";
      case 0x99: return "SBC A,C";
      case 0x9a: return "SBC A,D";
      case 0x9b: return "SBC A,E";
      case 0x9c: return "SBC A,H";
      case 0x9d: return "SBC A,L";
      case 0x9e: return "SBC A,(HL)";
      case 0x9f: return "SBC A,A";
      case 0xa0: return "AND B";
      case 0xa1: return "AND C";
      case 0xa2: return "AND D";
      case 0xa3: return "AND E";
      case 0xa4: return "AND H";
      case 0xa5: return "AND L";
      case 0xa6: return "AND (HL)";
      case 0xa7: return "AND A";
      case 0xa8: return "XOR B";
      case 0xa9: return "XOR C";
      case 0xaa: return "XOR D";
      case 0xab: return "XOR E";
      case 0xac: return "XOR H";
      case 0xad: return "XOR L";
      case 0xae: return "XOR (HL)";
      case 0xaf: return "XOR A";
      case 0xb0: return "OR B";
      case 0xb1: return "OR C";
      case 0xb2: return "OR D";
      case 0xb3: return "OR E";
      case 0xb4: return "OR H";
      case 0xb5: return "OR L";
      case 0xb6: return "OR (HL)";
      case 0xb7: return "OR A";
      case 0xb8: return "CP B";
      case 0xb9: return "CP C";
      case 0xba: return "CP D";
      case 0xbb: return "CP E";
      case 0xbc: return "CP H";
      case 0xbd: return "CP L";
      case 0xbe: return "CP (HL)";
      case 0xbf: return "CP A";
      case 0xc0: return "RET NZ";
      case 0xc1: return "POP BC";
      case 0xc2: return "JP NZ,nn";
      case 0xc3: return "JP nn";
      case 0xc4: return "CALL NZ,nn";
      case 0xc5: return "PUSH BC";
      case 0xc6: return "ADD A,n";
      case 0xc7: return "RST 0";
      case 0xc8: return "RET Z";
      case 0xc9: return "RET";
      case 0xca: return "JP Z,nn";
      case 0xcb: return "<0xcb: PREFIX>";
      case 0xcc: return "CALL Z,nn";
      case 0xcd: return "CALL nn";
      case 0xce: return "ADC A,n";
      case 0xcf: return "RST 8";
      case 0xd0: return "RET NC";
      case 0xd1: return "POP DE";
      case 0xd2: return "JP NC,nn";
      case 0xd3: return "<0xd3: INVALID>";
      case 0xd4: return "CALL NC,nn";
      case 0xd5: return "PUSH DE";
      case 0xd6: return "SUB A,n";
      case 0xd7: return "RST 10";
      case 0xd8: return "RET C";
      case 0xd9: return "RETI";
      case 0xda: return "JP C,nn";
      case 0xdb: return "<0xdb: INVALID>";
      case 0xdc: return "CALL C,nn";
      case 0xdd: return "<0xdd: INVALID>";
      case 0xde: return "SBC A,n";
      case 0xdf: return "RST 18";
      case 0xe0: return "LDH (n),A";
      case 0xe1: return "POP HL";
      case 0xe2: return "LDH (C),A";
      case 0xe3: return "<0xe3: INVALID>";
      case 0xe4: return "<0xe4: INVALID>";
      case 0xe5: return "PUSH HL";
      case 0xe6: return "AND n";
      case 0xe7: return "RST 20";
      case 0xe8: return "ADD SP,d";
      case 0xe9: return "JP (HL)";
      case 0xea: return "LD (nn),A";
      case 0xeb: return "<0xeb: INVALID>";
      case 0xec: return "<0xec: INVALID>";
      case 0xed: return "<0xed: INVALID>";
      case 0xee: return "XOR n";
      case 0xef: return "RST 28";
      case 0xf0: return "LDH A,(n)";
      case 0xf1: return "POP AF";
      case 0xf2: return "LDH A, (C)";
      case 0xf3: return "DI";
      case 0xf4: return "<0xf4: INVALID>";
      case 0xf5: return "PUSH AF";
      case 0xf6: return "OR n";
      case 0xf7: return "RST 30";
      case 0xf8: return "LDHL SP,d";
      case 0xf9: return "LD SP,HL";
      case 0xfa: return "LD A,(nn)";
      case 0xfb: return "EI";
      case 0xfc: return "<0xfc: INVALID>";
      case 0xfd: return "<0xfd: INVALID>";
      case 0xfe: return "CP n";
      case 0xff: return "RST 38";
    }

  } else {
    switch (instruction.opcode.high) {
      case 0x00: return "RLC B";
      case 0x01: return "RLC C";
      case 0x02: return "RLC D";
      case 0x03: return "RLC E";
      case 0x04: return "RLC H";
      case 0x05: return "RLC L";
      case 0x06: return "RLC (HL)";
      case 0x07: return "RLC A";
      case 0x08: return "RRC B";
      case 0x09: return "RRC C";
      case 0x0A: return "RRC D";
      case 0x0B: return "RRC E";
      case 0x0C: return "RRC H";
      case 0x0D: return "RRC L";
      case 0x0E: return "RRC (HL)";
      case 0x0F: return "RRC A";
      case 0x10: return "RL B";
      case 0x11: return "RL C";
      case 0x12: return "RL D";
      case 0x13: return "RL E";
      case 0x14: return "RL H";
      case 0x15: return "RL L";
      case 0x16: return "RL (HL)";
      case 0x17: return "RL A";
      case 0x18: return "RR B";
      case 0x19: return "RR C";
      case 0x1a: return "RR D";
      case 0x1b: return "RR E";
      case 0x1c: return "RR H";
      case 0x1d: return "RR L";
      case 0x1e: return "RR (HL)";
      case 0x1f: return "RR A";
      case 0x20: return "SLA B";
      case 0x21: return "SLA C";
      case 0x22: return "SLA D";
      case 0x23: return "SLA E";
      case 0x24: return "SLA H";
      case 0x25: return "SLA L";
      case 0x26: return "SLA (HL)";
      case 0x27: return "SLA A";
      case 0x28: return "SRA B";
      case 0x29: return "SRA C";
      case 0x2a: return "SRA D";
      case 0x2b: return "SRA E";
      case 0x2c: return "SRA H";
      case 0x2d: return "SRA L";
      case 0x2e: return "SRA (HL)";
      case 0x2f: return "SRA A";
      case 0x30: return "SWAP B";
      case 0x31: return "SWAP C";
      case 0x32: return "SWAP D";
      case 0x33: return "SWAP E";
      case 0x34: return "SWAP H";
      case 0x35: return "SWAP L";
      case 0x36: return "SWAP (HL)";
      case 0x37: return "SWAP A";
      case 0x38: return "SRL B";
      case 0x39: return "SRL C";
      case 0x3a: return "SRL D";
      case 0x3b: return "SRL E";
      case 0x3c: return "SRL H";
      case 0x3d: return "SRL L";
      case 0x3e: return "SRL (HL)";
      case 0x3f: return "SRL A";
      case 0x40: return "BIT 0,B";
      case 0x41: return "BIT 0,C";
      case 0x42: return "BIT 0,D";
      case 0x43: return "BIT 0,E";
      case 0x44: return "BIT 0,H";
      case 0x45: return "BIT 0,L";
      case 0x46: return "BIT 0,(HL)";
      case 0x47: return "BIT 0,A";
      case 0x48: return "BIT 1,B";
      case 0x49: return "BIT 1,C";
      case 0x4a: return "BIT 1,D";
      case 0x4b: return "BIT 1,E";
      case 0x4c: return "BIT 1,H";
      case 0x4d: return "BIT 1,L";
      case 0x4e: return "BIT 1,(HL)";
      case 0x4f: return "BIT 1,A";
      case 0x50: return "BIT 2,B";
      case 0x51: return "BIT 2,C";
      case 0x52: return "BIT 2,D";
      case 0x53: return "BIT 2,E";
      case 0x54: return "BIT 2,H";
      case 0x55: return "BIT 2,L";
      case 0x56: return "BIT 2,(HL)";
      case 0x57: return "BIT 2,A";
      case 0x58: return "BIT 3,B";
      case 0x59: return "BIT 3,C";
      case 0x5a: return "BIT 3,D";
      case 0x5b: return "BIT 3,E";
      case 0x5c: return "BIT 3,H";
      case 0x5d: return "BIT 3,L";
      case 0x5e: return "BIT 3,(HL)";
      case 0x5f: return "BIT 3,A";
      case 0x60: return "BIT 4,B";
      case 0x61: return "BIT 4,C";
      case 0x62: return "BIT 4,D";
      case 0x63: return "BIT 4,E";
      case 0x64: return "BIT 4,H";
      case 0x65: return "BIT 4,L";
      case 0x66: return "BIT 4,(HL)";
      case 0x67: return "BIT 4,A";
      case 0x68: return "BIT 5,B";
      case 0x69: return "BIT 5,C";
      case 0x6a: return "BIT 5,D";
      case 0x6b: return "BIT 5,E";
      case 0x6c: return "BIT 5,H";
      case 0x6d: return "BIT 5,L";
      case 0x6e: return "BIT 5,(HL)";
      case 0x6f: return "BIT 5,A";
      case 0x70: return "BIT 6,B";
      case 0x71: return "BIT 6,C";
      case 0x72: return "BIT 6,D";
      case 0x73: return "BIT 6,E";
      case 0x74: return "BIT 6,H";
      case 0x75: return "BIT 6,L";
      case 0x76: return "BIT 6,(HL)";
      case 0x77: return "BIT 6,A";
      case 0x78: return "BIT 7,B";
      case 0x79: return "BIT 7,C";
      case 0x7a: return "BIT 7,D";
      case 0x7b: return "BIT 7,E";
      case 0x7c: return "BIT 7,H";
      case 0x7d: return "BIT 7,L";
      case 0x7e: return "BIT 7,(HL)";
      case 0x7f: return "BIT 7,A";
      case 0x80: return "RES 0,B";
      case 0x81: return "RES 0,C";
      case 0x82: return "RES 0,D";
      case 0x83: return "RES 0,E";
      case 0x84: return "RES 0,H";
      case 0x85: return "RES 0,L";
      case 0x86: return "RES 0,(HL)";
      case 0x87: return "RES 0,A";
      case 0x88: return "RES 1,B";
      case 0x89: return "RES 1,C";
      case 0x8a: return "RES 1,D";
      case 0x8b: return "RES 1,E";
      case 0x8c: return "RES 1,H";
      case 0x8d: return "RES 1,L";
      case 0x8e: return "RES 1,(HL)";
      case 0x8f: return "RES 1,A";
      case 0x90: return "RES 2,B";
      case 0x91: return "RES 2,C";
      case 0x92: return "RES 2,D";
      case 0x93: return "RES 2,E";
      case 0x94: return "RES 2,H";
      case 0x95: return "RES 2,L";
      case 0x96: return "RES 2,(HL)";
      case 0x97: return "RES 2,A";
      case 0x98: return "RES 3,B";
      case 0x99: return "RES 3,C";
      case 0x9a: return "RES 3,D";
      case 0x9b: return "RES 3,E";
      case 0x9c: return "RES 3,H";
      case 0x9d: return "RES 3,L";
      case 0x9e: return "RES 3,(HL)";
      case 0x9f: return "RES 3,A";
      case 0xa0: return "RES 4,B";
      case 0xa1: return "RES 4,C";
      case 0xa2: return "RES 4,D";
      case 0xa3: return "RES 4,E";
      case 0xa4: return "RES 4,H";
      case 0xa5: return "RES 4,L";
      case 0xa6: return "RES 4,(HL)";
      case 0xa7: return "RES 4,A";
      case 0xa8: return "RES 5,B";
      case 0xa9: return "RES 5,C";
      case 0xaa: return "RES 5,D";
      case 0xab: return "RES 5,E";
      case 0xac: return "RES 5,H";
      case 0xad: return "RES 5,L";
      case 0xae: return "RES 5,(HL)";
      case 0xaf: return "RES 5,A";
      case 0xb0: return "RES 6,B";
      case 0xb1: return "RES 6,C";
      case 0xb2: return "RES 6,D";
      case 0xb3: return "RES 6,E";
      case 0xb4: return "RES 6,H";
      case 0xb5: return "RES 6,L";
      case 0xb6: return "RES 6,(HL)";
      case 0xb7: return "RES 6,A";
      case 0xb8: return "RES 7,B";
      case 0xb9: return "RES 7,C";
      case 0xba: return "RES 7,D";
      case 0xbb: return "RES 7,E";
      case 0xbc: return "RES 7,H";
      case 0xbd: return "RES 7,L";
      case 0xbe: return "RES 7,(HL)";
      case 0xbf: return "RES 7,A";
      case 0xc0: return "SET 0,B";
      case 0xc1: return "SET 0,C";
      case 0xc2: return "SET 0,D";
      case 0xc3: return "SET 0,E";
      case 0xc4: return "SET 0,H";
      case 0xc5: return "SET 0,L";
      case 0xc6: return "SET 0,(HL)";
      case 0xc7: return "SET 0,A";
      case 0xc8: return "SET 1,B";
      case 0xc9: return "SET 1,C";
      case 0xca: return "SET 1,D";
      case 0xcb: return "SET 1,E";
      case 0xcc: return "SET 1,H";
      case 0xcd: return "SET 1,L";
      case 0xce: return "SET 1,(HL)";
      case 0xcf: return "SET 1,A";
      case 0xd0: return "SET 2,B";
      case 0xd1: return "SET 2,C";
      case 0xd2: return "SET 2,D";
      case 0xd3: return "SET 2,E";
      case 0xd4: return "SET 2,H";
      case 0xd5: return "SET 2,L";
      case 0xd6: return "SET 2,(HL)";
      case 0xd7: return "SET 2,A";
      case 0xd8: return "SET 3,B";
      case 0xd9: return "SET 3,C";
      case 0xda: return "SET 3,D";
      case 0xdb: return "SET 3,E";
      case 0xdc: return "SET 3,H";
      case 0xdd: return "SET 3,L";
      case 0xde: return "SET 3,(HL)";
      case 0xdf: return "SET 3,A";
      case 0xe0: return "SET 4,B";
      case 0xe1: return "SET 4,C";
      case 0xe2: return "SET 4,D";
      case 0xe3: return "SET 4,E";
      case 0xe4: return "SET 4,H";
      case 0xe5: return "SET 4,L";
      case 0xe6: return "SET 4,(HL)";
      case 0xe7: return "SET 4,A";
      case 0xe8: return "SET 5,B";
      case 0xe9: return "SET 5,C";
      case 0xea: return "SET 5,D";
      case 0xeb: return "SET 5,E";
      case 0xec: return "SET 5,H";
      case 0xed: return "SET 5,L";
      case 0xee: return "SET 5,(HL)";
      case 0xef: return "SET 5,A";
      case 0xf0: return "SET 6,B";
      case 0xf1: return "SET 6,C";
      case 0xf2: return "SET 6,D";
      case 0xf3: return "SET 6,E";
      case 0xf4: return "SET 6,H";
      case 0xf5: return "SET 6,L";
      case 0xf6: return "SET 6,(HL)";
      case 0xf7: return "SET 6,A";
      case 0xf8: return "SET 7,B";
      case 0xf9: return "SET 7,C";
      case 0xfa: return "SET 7,D";
      case 0xfb: return "SET 7,E";
      case 0xfc: return "SET 7,H";
      case 0xfd: return "SET 7,L";
      case 0xfe: return "SET 7,(HL)";
      case 0xff: return "SET 7,A";
    }
  }

  NOT_REACHED();
  return "<unknown>";
}

}  // namespace emulator
}  // namespace rothko
