// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "cpu.h"
#include "cpu_instructions.h"

#define CATCH_CONFIG_MAIN
#include <third_party/catch2/catch.hpp>

namespace rothko {
namespace emulator {
namespace test {
namespace {

constexpr uint8_t kReturnedTicks = 4;

TEST_CASE("Conditional Ticks") {
  SECTION("Z Flag") {
    uint8_t z_not_set = 0;
    uint8_t z_set = SetBit(0, kCPUFlagsZIndex);

    ConditionalTicks neg_ct = COND_TICKS_NEGATIVE_FLAG(Z, kReturnedTicks);
    ConditionalTicks pos_ct = COND_TICKS_POSITIVE_FLAG(Z, kReturnedTicks);

    CHECK(GetConditionalTicks(neg_ct, z_not_set) == kReturnedTicks);
    CHECK(GetConditionalTicks(neg_ct, z_set) == 0);

    CHECK(GetConditionalTicks(pos_ct, z_not_set) == 0);
    CHECK(GetConditionalTicks(pos_ct, z_set) == kReturnedTicks);
  }

  SECTION("C Flag") {
    uint8_t c_not_set = 0;
    uint8_t c_set = SetBit(0, kCPUFlagsCIndex);

    ConditionalTicks neg_ct = COND_TICKS_NEGATIVE_FLAG(C, kReturnedTicks);
    ConditionalTicks pos_ct = COND_TICKS_POSITIVE_FLAG(C, kReturnedTicks);

    CHECK(GetConditionalTicks(neg_ct, c_not_set) == kReturnedTicks);
    CHECK(GetConditionalTicks(neg_ct, c_set) == 0);

    CHECK(GetConditionalTicks(pos_ct, c_not_set) == 0);
    CHECK(GetConditionalTicks(pos_ct, c_set) == kReturnedTicks);
  }
}

// Normal Instruction Conditional Ticks ------------------------------------------------------------

struct InstructionConditionalTicks {
  Instruction::Opcode opcode;
  uint8_t expected_ticks;
};

// clang-format off
constexpr InstructionConditionalTicks kKnownNormalOpcodes[] = {
  {{0x20},  4},
  {{0x28},  4},
  {{0x30},  4},
  {{0x38},  4},
  {{0xc0}, 12},
  {{0xc2},  4},
  {{0xc4}, 12},
  {{0xc8}, 12},
  {{0xca},  4},
  {{0xcc}, 12},
  {{0xd0}, 12},
  {{0xd2},  4},
  {{0xd4}, 12},
  {{0xd8}, 12},
  {{0xda},  4},
  {{0xdc}, 12},
};
// clang-format on

inline bool IsWithinKnownNormalOpcodes(uint8_t opcode) {
  for (auto& known_cond_tick : kKnownNormalOpcodes) {
    if (known_cond_tick.opcode.low == opcode)
      return true;
  }

  return false;
}

TEST_CASE("Normal Instructions Conditional Ticks") {
  SECTION("No known instruction should give 0") {
    Instruction instruction;

    for (uint16_t opcode = 0; opcode < 0x100; opcode++) {
      // Known opcodes are testes later down.
      if (IsWithinKnownNormalOpcodes(opcode))
        continue;

      // We skip the 0xcb prefix, those will be tested otherwise.
      if (opcode == 0xcb)
        continue;

      INFO("Opcode 0x" << std::hex << opcode);
      instruction.opcode.opcode = opcode;

      // Both no flags and all set flags should return 0.
      CHECK(GetConditionalTicks(instruction, 0) == 0);
      CHECK(GetConditionalTicks(instruction, 0xff) == 0);
    }
  }

  SECTION("Known instructions") {
    Instruction instruction;

    uint8_t z_not_set = 0;
    uint8_t z_set = SetBit(0, kCPUFlagsZIndex);

    uint8_t c_not_set = 0;
    uint8_t c_set = SetBit(0, kCPUFlagsCIndex);

    {
      // (0x20) JR NZ,n: Relative jump by signed immediate if last result was not zero.
      instruction.opcode.opcode = 0x20;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, z_not_set) == 4);
      CHECK(GetConditionalTicks(instruction, z_set ) == 0);
    }

    {
      // (0x28) JR Z,n: Relative jump by signed immediate if last result was zero.
      instruction.opcode.opcode = 0x28;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, z_not_set) == 0);
      CHECK(GetConditionalTicks(instruction, z_set ) == 4);
    }

    {
      // (0x30) JR NC,n: Relative jump by signed immediate if last result caused no carry.
      instruction.opcode.opcode = 0x30;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, c_not_set) == 4);
      CHECK(GetConditionalTicks(instruction, c_set ) == 0);
    }

    {
      // (0x38) JR C,n: Relative jump by signed immediate if last result caused carry.
      instruction.opcode.opcode = 0x38;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, c_not_set) == 0);
      CHECK(GetConditionalTicks(instruction, c_set ) == 4);
    }

    {
      // (0xc0) RET NZ: Return if last result was not zero.
      instruction.opcode.opcode = 0xc0;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, z_not_set) == 12);
      CHECK(GetConditionalTicks(instruction, z_set ) == 0);
    }

    {
      // (0xc2) JP NZ,nn: Absolute jump to 16-bit location if last result was not zero.
      instruction.opcode.opcode = 0xc2;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, z_not_set) == 4);
      CHECK(GetConditionalTicks(instruction, z_set ) == 0);
    }

    {
      // (0xc4) CALL NZ,nn: Call routine at 16-bit location if last result was not zero.
      instruction.opcode.opcode = 0xc4;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, z_not_set) == 12);
      CHECK(GetConditionalTicks(instruction, z_set ) == 0);
    }

    {
      // (0xc8) RET Z: Return if last result was zero.
      instruction.opcode.opcode = 0xc8;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, z_not_set) == 0);
      CHECK(GetConditionalTicks(instruction, z_set ) == 12);
    }

    {
      // (0xca) JP Z,nn: Absolute jump to 16-bit location if last result was zero.
      instruction.opcode.opcode = 0xca;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, z_not_set) == 0);
      CHECK(GetConditionalTicks(instruction, z_set) == 4);
    }

    {
      // (0xcc) CALL Z,nn: Call routine at 16-bit location if last result was zero.
      instruction.opcode.opcode = 0xcc;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, z_not_set) == 0);
      CHECK(GetConditionalTicks(instruction, z_set ) == 12);
    }

    {
      // (0xd0) RET NC: Return if last result caused no carry.
      instruction.opcode.opcode = 0xd0;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, c_not_set) == 12);
      CHECK(GetConditionalTicks(instruction, c_set ) == 0);
    }

    {
      // (0xd2) JP NC,nn: Absolute jump to 16-bit location if last result caused no carry.
      instruction.opcode.opcode = 0xd2;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, c_not_set) == 4);
      CHECK(GetConditionalTicks(instruction, c_set ) == 0);
    }

    {
      // (0xd4) CALL NC,nn: Call routine at 16-bit location if last result caused no carry.
      instruction.opcode.opcode = 0xd4;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, c_not_set) == 12);
      CHECK(GetConditionalTicks(instruction, c_set ) == 0);
    }

    {
      // (0xd8) RET C: Return if last result caused carry.
      instruction.opcode.opcode = 0xd8;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, c_not_set) == 0);
      CHECK(GetConditionalTicks(instruction, c_set ) == 12);
    }

    {
      // (0xda) JP C,nn: Absolute jump to 16-bit location if last result caused carry.
      instruction.opcode.opcode = 0xda;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, c_not_set) == 0);
      CHECK(GetConditionalTicks(instruction, c_set ) == 4);
    }

    {
      // (0xdc) CALL C,nn: Call routine at 16-bit location if last result caused carry.
      instruction.opcode.opcode = 0xdc;
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      CHECK(GetConditionalTicks(instruction, c_not_set) == 12);
      CHECK(GetConditionalTicks(instruction, c_set ) == 0);
    }
  }
}

}  // namespace
}  // namespace test
}  // namespace emulator
}  // namespace rothko
