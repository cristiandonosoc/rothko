// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "cpu.h"
#include "cpu_instructions.h"

#include <rothko/logging/logging.h>

namespace rothko {
namespace emulator {

namespace {

Instruction InitInstruction(uint8_t length) {
  Instruction instruction = {};
  instruction.length = length;

  return instruction;
}

// clang-format off
uint8_t normal_lengths[0x100] = {
/*       0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
/* 0 */  1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,
/* 1 */  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
/* 2 */  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
/* 3 */  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
/* 4 */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 5 */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 6 */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 7 */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 8 */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 9 */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* a */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* b */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* c */  1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 2, 3, 3, 2, 1,
/* d */  1, 1, 3, 0, 3, 1, 2, 1, 1, 1, 3, 0, 3, 0, 2, 1,
/* e */  2, 1, 1, 0, 1, 1, 2, 1, 2, 1, 3, 0, 0, 0, 2, 1,
/* f */  2, 1, 1, 1, 0, 1, 2, 1, 2, 1, 3, 1, 0, 0, 2, 1,
};
static_assert(sizeof(normal_lengths) == 256);
// clang-format on

// clang-format off
uint8_t normal_ticks[0x100] = {
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
static_assert(sizeof(normal_ticks) == 256);
// clang-format on

}  // namespace

// GetConditionalTicks -----------------------------------------------------------------------------

namespace {

ConditionalTicks normal_conditional_masks[] = {
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

uint8_t GetNormalConditionalTicks(const Instruction& instruction, uint8_t flags){
  switch (instruction.opcode.low) {
    case 0x20: return GetConditionalTicks(normal_conditional_masks[0], flags);
    case 0x28: return GetConditionalTicks(normal_conditional_masks[1], flags);
    case 0x30: return GetConditionalTicks(normal_conditional_masks[2], flags);
    case 0x38: return GetConditionalTicks(normal_conditional_masks[3], flags);
    case 0xc0: return GetConditionalTicks(normal_conditional_masks[4], flags);
    case 0xc2: return GetConditionalTicks(normal_conditional_masks[5], flags);
    case 0xc4: return GetConditionalTicks(normal_conditional_masks[6], flags);
    case 0xc8: return GetConditionalTicks(normal_conditional_masks[7], flags);
    case 0xca: return GetConditionalTicks(normal_conditional_masks[8], flags);
    case 0xcc: return GetConditionalTicks(normal_conditional_masks[9], flags);
    case 0xd0: return GetConditionalTicks(normal_conditional_masks[10], flags);
    case 0xd2: return GetConditionalTicks(normal_conditional_masks[11], flags);
    case 0xd4: return GetConditionalTicks(normal_conditional_masks[12], flags);
    case 0xd8: return GetConditionalTicks(normal_conditional_masks[13], flags);
    case 0xda: return GetConditionalTicks(normal_conditional_masks[14], flags);
    case 0xdc: return GetConditionalTicks(normal_conditional_masks[15], flags);
    default: return 0;
  }
}

uint8_t GetCBConditionalTicks(const Instruction&, uint8_t flags) {
  (void)flags;
  NOT_IMPLEMENTED();
  return 0;
}


}  // namespace

uint8_t GetConditionalTicks(const Instruction& instruction, uint8_t flags) {
  return !IsCBInstruction(instruction) ? GetNormalConditionalTicks(instruction, flags) :
                                         GetCBConditionalTicks(instruction, flags);
}

uint8_t GetConditionalTicks(const ConditionalTicks& ct, uint8_t flags) {
  // The idea is that the masked bit has to be the same for the extra ticks be returned.
  // For this we mask the bit, we XNOR (XOR then negate) and them mask that bit.
  // 1 = The bits were the same. 0 = They were different.
  uint8_t result = (flags ^ ct.xnor_comparator);
  uint8_t not_result = (~result & ct.mask);

  return not_result != 0 ? ct.extra_ticks : 0;
}

// Init Instructions -------------------------------------------------------------------------------

void InitInstructions(CPU* cpu) {
  (void)cpu;
}

}  // namespace emulator
}  // namespace rothko
