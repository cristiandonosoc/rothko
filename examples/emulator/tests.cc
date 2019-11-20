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

// Conditional Ticks -------------------------------------------------------------------------------

TEST_CASE("Conditional Ticks") {
  SECTION("Z Flag") {
    uint8_t z_not_set = 0;
    uint8_t z_set = 0;
    SetBit(&z_set, kCPUFlagsZIndex);

    ConditionalTicks neg_ct = COND_TICKS_NEGATIVE_FLAG(Z, kReturnedTicks);
    ConditionalTicks pos_ct = COND_TICKS_POSITIVE_FLAG(Z, kReturnedTicks);

    CHECK(GetConditionalTicks(neg_ct, z_not_set) == kReturnedTicks);
    CHECK(GetConditionalTicks(neg_ct, z_set) == 0);

    CHECK(GetConditionalTicks(pos_ct, z_not_set) == 0);
    CHECK(GetConditionalTicks(pos_ct, z_set) == kReturnedTicks);
  }

  SECTION("C Flag") {
    uint8_t c_not_set = 0;
    uint8_t c_set = 0;
    SetBit(&c_set, kCPUFlagsCIndex);

    ConditionalTicks neg_ct = COND_TICKS_NEGATIVE_FLAG(C, kReturnedTicks);
    ConditionalTicks pos_ct = COND_TICKS_POSITIVE_FLAG(C, kReturnedTicks);

    CHECK(GetConditionalTicks(neg_ct, c_not_set) == kReturnedTicks);
    CHECK(GetConditionalTicks(neg_ct, c_set) == 0);

    CHECK(GetConditionalTicks(pos_ct, c_not_set) == 0);
    CHECK(GetConditionalTicks(pos_ct, c_set) == kReturnedTicks);
  }
}


// clang-format off
constexpr uint8_t kKnownNormalOpcodes[] = {
  0x20, 0x28, 0x30, 0x38, 0xc0, 0xc2, 0xc4, 0xc8,
  0xca, 0xcc, 0xd0, 0xd2, 0xd4, 0xd8, 0xda, 0xdc,
};
// clang-format on

inline bool IsWithinKnownNormalOpcodes(uint8_t opcode) {
  for (uint8_t known_opcode : kKnownNormalOpcodes) {
    if (opcode == known_opcode)
      return true;
  }

  return false;
}

TEST_CASE("Instruction Conditional Ticks") {
  SECTION("Non-conditional normal instructions should give 0") {
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

  SECTION("CB instructions all return 0") {
    Instruction instruction;
    instruction.opcode.high = 0xcb;
    for (uint16_t opcode = 0; opcode < 0x100; opcode++) {
      INFO("Opcode 0x" << std::hex << instruction.opcode.opcode);
      instruction.opcode.low = opcode;

      // Both no flags and all set flags should return 0.
      CHECK(GetConditionalTicks(instruction, 0) == 0);
      CHECK(GetConditionalTicks(instruction, 0xff) == 0);
    }
  }

  SECTION("Conditional normal instructions should return ticks with correct flags") {
    Instruction instruction;

    uint8_t z_not_set = 0;
    uint8_t z_set = 0;
    SetBit(&z_set, kCPUFlagsZIndex);

    uint8_t c_not_set = 0;
    uint8_t c_set = 0;
    SetBit(&c_set, kCPUFlagsCIndex);

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

// Instruction Fetch -------------------------------------------------------------------------------

inline void SetData(uint8_t* data, uint8_t a, uint8_t b, uint8_t c) {
  data[0] = a;
  data[1] = b;
  data[2] = c;
}

constexpr uint8_t kInvalidOpcodes[] = {
    0xD3, 0xDB, 0xDD, 0xE3,
    0xE4, 0xEB, 0xEC, 0xED,
    0xF4, 0xFC, 0xFD,
};

TEST_CASE("FetchAndDecode") {
  SECTION("Valid normal decoding") {
    uint8_t data[3];
    Instruction instruction;

    // NOTE: FetchAndDecode will insert in the operands even if they're not needed.

    // 0x00: NOP.
    SetData(data, 0x00, 0x0a, 0x0b);
    REQUIRE(FetchAndDecode(&instruction, data) == true);
    CHECK(instruction.opcode.opcode == 0x0000);
    CHECK(instruction.length == 1u);
    CHECK(instruction.ticks == 4u);
    CHECK(*(uint16_t*)instruction.operands == 0x0b0a);


    // 0x03: INC BC.
    SetData(data, 0x03, 0xa0, 0x0b);
    REQUIRE(FetchAndDecode(&instruction, data) == true);
    CHECK(instruction.opcode.opcode == 0x0003);
    CHECK(instruction.length == 1u);
    CHECK(instruction.ticks == 8u);
    CHECK(*(uint16_t*)instruction.operands == 0x0ba0);

    // 0x3e: LD L, d8.
    SetData(data, 0x3e, 0xad, 0xde);
    REQUIRE(FetchAndDecode(&instruction, data) == true);
    CHECK(instruction.opcode.opcode == 0x003e);
    CHECK(instruction.length == 2u);
    CHECK(instruction.ticks == 8u);
    CHECK(*(uint16_t*)instruction.operands == 0xdead);

    // 0ca: JP Z, a16.
    SetData(data, 0xca, 0xef, 0xbe);
    REQUIRE(FetchAndDecode(&instruction, data) == true);
    CHECK(instruction.opcode.opcode == 0x00ca);
    CHECK(instruction.length == 3u);
    CHECK(instruction.ticks == 12u);  // This is conditional, but that's not part of decode.
    CHECK(*(uint16_t*)instruction.operands == 0xbeef);
  }

  SECTION("Invalid decoding should fail") {
    uint8_t data[3];
    Instruction instruction;

    for (uint8_t invalid_opcode : kInvalidOpcodes) {
      INFO("FetchAndDecode 0x" << std::hex << invalid_opcode);

      SetData(data, invalid_opcode, 0xff, 0xff);
      CHECK(FetchAndDecode(&instruction, data) == false);
    }
  }

  SECTION("CB instructions are all same length") {
    uint8_t data[3];
    Instruction instruction;

    for (uint16_t i = 0; i < 0x100; i++) {
      uint16_t cb_opcode = 0xcb00 | i;

      INFO("FetchAndDecode 0x" << std::hex << cb_opcode);
      SetData(data, 0xcb, (uint8_t)i, 0xff);

      REQUIRE(FetchAndDecode(&instruction, data) == true);
      REQUIRE(instruction.opcode.opcode == cb_opcode);
      REQUIRE(IsCBInstruction(instruction) == true);
      CHECK(instruction.length == 2u);

      // cb instructions have no operands.
      CHECK(*(uint16_t*)instruction.operands == 0u);
    }
  }
}

}  // namespace
}  // namespace test
}  // namespace emulator
}  // namespace rothko
