// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "cpu.h"
#include "cpu_instructions.h"

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

ConditionalTicks conditional_masks[] = {
    // (0x20) JR NZ,n: Relative jump by signed immediate if last result was not zero
    COND_TICKS_NEGATIVE_FLAG(Z, 4),
    /* if (cpu.Registers.FZ != 0) { break; } */
    /* result = 4; */
    // (0x28) JR Z,n: Relative jump by signed immediate if last result was zero
    COND_TICKS_POSITIVE_FLAG(Z, 4),
    /* if (cpu.Registers.FZ == 0) { break; } */
    /* result = 4; */
    // (0x30) JR NC,n: Relative jump by signed immediate if last result caused no carry
    COND_TICKS_NEGATIVE_FLAG(C, 4),
    /* if (cpu.Registers.FC != 0) { break; } */
    /* result = 4; */
    // (0x38) JR C,n: Relative jump by signed immediate if last result caused carry
    COND_TICKS_POSITIVE_FLAG(C, 4),
    /* if (cpu.Registers.FC == 0) { break; } */
    /* result = 4; */
    // (0xC0) RET NZ: Return if last result was not zero
    COND_TICKS_NEGATIVE_FLAG(Z, 12),
    /* if (cpu.Registers.FZ != 0) { break; } */
    /* result = 12; */
    // (0xC2) JP NZ,nn: Absolute jump to 16-bit location if last result was not zero
    COND_TICKS_NEGATIVE_FLAG(Z, 4),
    /* if (cpu.Registers.FZ != 0) { break; } */
    /* result = 4; */
    // (0xC4) CALL NZ,nn: Call routine at 16-bit location if last result was not zero
    COND_TICKS_NEGATIVE_FLAG(Z, 12),
    /* if (cpu.Registers.FZ != 0) { break; } */
    /* result = 12; */
    // (0xC8) RET Z: Return if last result was zero
    COND_TICKS_POSITIVE_FLAG(Z, 12),
    /* if (cpu.Registers.FZ == 0) { break; } */
    /* result = 12; */
    // (0xCA) JP Z,nn: Absolute jump to 16-bit location if last result was zero
    COND_TICKS_POSITIVE_FLAG(Z, 4),
    /* if (cpu.Registers.FZ == 0) { break; } */
    /* result = 4; */
    // (0xCC) CALL Z,nn: Call routine at 16-bit location if last result was zero
    COND_TICKS_POSITIVE_FLAG(Z, 12),
    /* if (cpu.Registers.FZ == 0) { break; } */
    /* result = 12; */
    // (0xD0) RET NC: Return if last result caused no carry
    COND_TICKS_NEGATIVE_FLAG(C, 12),
    /* if (cpu.Registers.FC != 0) { break; } */
    /* result = 12; */
    // (0xD2) JP NC,nn: Absolute jump to 16-bit location if last result caused no carry
    COND_TICKS_NEGATIVE_FLAG(C, 4),
    /* if (cpu.Registers.FC != 0) { break; } */
    /* result = 4; */
    // (0xD4) CALL NC,nn: Call routine at 16-bit location if last result caused no carry
    COND_TICKS_NEGATIVE_FLAG(C, 12),
    /* if (cpu.Registers.FC != 0) { break; } */
    /* result = 12; */
    // (0xD8) RET C: Return if last result caused carry
    COND_TICKS_POSITIVE_FLAG(C, 12),
    /* if (cpu.Registers.FC == 0) { break; } */
    /* result = 12; */
    // (0xDA) JP C,nn: Absolute jump to 16-bit location if last result caused carry
    COND_TICKS_POSITIVE_FLAG(C, 4),
    /* if (cpu.Registers.FC == 0) { break; } */
    /* result = 4; */
    // (0xDC) CALL C,nn: Call routine at 16-bit location if last result caused carry
    COND_TICKS_NEGATIVE_FLAG(C, 12),
    /* if (cpu.Registers.FC == 0) { break; } */
    /* result = 12; */
};

}  // namespace

void InitInstructions(CPU* cpu) {
  (void)cpu;
}

}  // namespace emulator
}  // namespace rothko
