// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>
#include <vector>

namespace rothko {
namespace emulator {

struct Gameboy;

// Address 0x146 of the catridge data.
enum class GameboyType : uint8_t {
  kGameboy = 0x00,
  kSuperGameboy = 0x03,

  kLast,
};
const char* ToString(GameboyType);

// Address 0x147 of the catridge data.
enum class CatridgeType : uint8_t {
  kROM_ONLY = 0x00,
  kROM_MBC1 = 0x01,
  kROM_MBC1_RAM = 0x02,
  kROM_MBC1_RAM_BATT = 0x03,
  kROM_MBC2 = 0x05,
  kROM_MBC2_BATTERY = 0x06,
  kROM_RAM = 0x08,
  kROM_RAM_BATTERY = 0x09,
  kROM_MMM01 = 0x0B,
  kROM_MMM01_SRAM = 0x0C,
  kROM_MMM01_SRAM_BATT = 0x0D,
  kROM_MBC3_TIMER_BATT = 0x0F,
  kROM_MBC3_TIMER_RAM_BATT = 0x10,
  kROM_MBC3 = 0x11,
  kROM_MBC3_RAM = 0x12,
  kROM_MBC3_RAM_BATT = 0x13,
  kROM_MBC5 = 0x19,
  kROM_MBC5_RAM = 0x1A,
  kROM_MBC5_RAM_BATT = 0x1B,
  kROM_MBC5_RUMBLE = 0x1C,
  kROM_MBC5_RUMBLE_SRAM = 0x1D,
  kROM_MBC5_RUMBLE_SRAM_BATT = 0x1E,
  kPocket_Camera = 0x1F,
  kBandai_TAMA5 = 0xFD,
  kHudson_HuC3 = 0xFE,
  kHudson_HuC1 = 0xFF,

  kLast = 0xAA,
};
const char* ToString(CatridgeType);

// MBC (Memory Bank Controllers) -------------------------------------------------------------------
//
// MBCs are somewhat akin to "drivers". There are different formats a catridge can have within a GB,
// each behaving differently to read/write commands. Some are basically ROM, others have more
// memory that have to be swapped in/out of the address space.

struct MBCApi{
  template <typename T>
  using ReadFunction = T (*)(Gameboy*, uint16_t address);

  template <typename T>
  using WriteFunction = void (*)(Gameboy*, uint16_t address, T value);

  CatridgeType type = CatridgeType::kLast;

  ReadFunction<uint8_t> ReadByte = nullptr;
  ReadFunction<uint16_t> ReadShort = nullptr;

  WriteFunction<uint8_t> WriteByte = nullptr;
  WriteFunction<uint16_t> WriteShort = nullptr;
};
inline bool Valid(const MBCApi& mbc) { return mbc.type != CatridgeType::kLast; }

// Catridge ----------------------------------------------------------------------------------------

struct Catridge {
  std::string title;  // Extracted from address 0x134-0x142 of the catridge data.

  GameboyType gameboy_type = GameboyType::kLast;
  CatridgeType catridge_type = CatridgeType::kLast;

  uint32_t rom_size = 0;   // In bytes. Address 0x148.
  uint32_t ram_size = 0;   // In bytes. Address 0x149.

  MBCApi mbc = {};

  std::vector<uint8_t> data;
};

// Verifies that the catridge header info is valid.
// Will copy |data| if it's valid.
bool Load(Catridge*, const uint8_t* data, size_t size);
bool Load(Catridge*, const std::string& path);

inline bool Valid(const Catridge& c) { return c.catridge_type != CatridgeType::kLast; }

}  // namespace emulator
}  // namespace rothko
