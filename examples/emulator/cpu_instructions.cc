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

// Init Instructions -------------------------------------------------------------------------------

void InitInstructions(CPU* cpu) {
  (void)cpu;
}

}  // namespace emulator
}  // namespace rothko
