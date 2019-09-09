// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "catridge.h"

#include <rothko/logging/logging.h>
#include <rothko/utils/file.h>

#include "gameboy.h"

namespace rothko {
namespace emulator {

// Load --------------------------------------------------------------------------------------------

namespace {

// Each ROM has to have a scrolling nintendo graphic hardcoded in the addresses 0x104-0x133.
// If it's not there, the gameboy will not run.
// We use it here to verify that the ROM is valid.
uint8_t kNintendoGraphic[48] = {
  0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73,
  0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f,
  0x88, 0x89, 0x00, 0x0e, 0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd,
  0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc,
  0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e,
};
static_assert(sizeof(kNintendoGraphic) == 48);

GameboyType GetGameboyType(uint8_t t);
CatridgeType GetCatridgeType(uint8_t t);

uint32_t GetRomSize(uint8_t);
uint32_t GetRamSize(uint8_t);  // Returns 0xff on failure.

}  // namespace

bool Load(Catridge* catridge, const uint8_t* data, size_t data_size) {
  // Verify the nintendo graphic.
  if (memcmp((void*)(data + 0x104), kNintendoGraphic, sizeof(kNintendoGraphic)) != 0) {
      ERROR(App, "Catridge does not have valid nintendo graphic.");
      return false;
  }

  GameboyType gameboy_type = GetGameboyType(data[0x146]);
  if (gameboy_type == GameboyType::kLast) {
    ERROR(App, "Invalid Gameboy type value: 0x%x", (uint8_t)gameboy_type);
    return false;
  }

  CatridgeType catridge_type = GetCatridgeType(data[0x147]);
  if (catridge_type == CatridgeType::kLast) {
    ERROR(App, "Invalid catridge type value: 0x%x", (uint8_t)catridge_type);
    return false;
  }

  uint32_t rom_size = GetRomSize(data[0x148]);
  uint32_t ram_size = GetRamSize(data[0x149]);
  if (rom_size == 0 || ram_size == 0xff) {
    ERROR(App, "Invalid rom/ram size values. Rom: 0x%x, Ram: 0x%x", data[0x148], data[0x149]);
    return false;
  }

  *catridge = {};

  // Safe loading of the title.
  char title_buf[17] = {};
  memcpy(title_buf, data + 0x134, 16);
  catridge->title = title_buf;

  catridge->gameboy_type = gameboy_type;
  catridge->catridge_type = catridge_type;
  catridge->rom_size = rom_size;
  catridge->ram_size = ram_size;

  catridge->data.resize(data_size);
  catridge->data.insert(catridge->data.end(), data, data + data_size);

  return true;
}

bool Load(Catridge* catridge, const std::string& path) {
  std::vector<uint8_t> data;
  if (!ReadWholeFile(path, &data)) {
    ERROR(App, "Could not read ROM %s", path.c_str());
    return false;
  }

  return Load(catridge, data.data(), data.size());
}



// Low Level Memory "API" --------------------------------------------------------------------------
//
// These are the functions of memory manipulation that are common to all Memory Bank Controllers.
// In general, MBCs will do work before calling this (like swapping memory banks).
//
// Defined at the bottom of the file.

namespace {

uint8_t LowLevelReadByte(Gameboy* gameboy, uint16_t address);

uint16_t LowLevelReadShort(Gameboy* gameboy, uint16_t address) {
  uint16_t res = LowLevelReadByte(gameboy, address++);
  res += (((uint16_t)LowLevelReadByte(gameboy, address++)) << 8);
  return res;
}

void LowLevelWriteByte(Gameboy* gameboy, uint16_t address, uint8_t value);

inline void LowLevelWriteShort(Gameboy* gameboy, uint16_t address, uint16_t value) {
  uint8_t* ptr = (uint8_t*)&value;
  LowLevelWriteByte(gameboy, address++, *ptr++);
  LowLevelWriteByte(gameboy, address, *ptr);
}

}  // namespace

// Basic MBC ---------------------------------------------------------------------------------------

namespace {

inline MBCApi GetBasicMBC() {
  MBCApi api = {};
  api.type = MBCType::kBasic;
  api.ReadByte = LowLevelReadByte;
  api.ReadShort = LowLevelReadShort;
  api.WriteByte = LowLevelWriteByte;
  api.WriteShort = LowLevelWriteShort;

  return api;
}

}  // namespace

// GETMBCApi ---------------------------------------------------------------------------------------

MBCApi GetMBCApi(MBCType type) {
  switch (type) {
    case MBCType::kBasic: return GetBasicMBC();
    case MBCType::kLast: break;
  }

  NOT_REACHED();
  return {};
}

// ****************************************** INTERNALS ********************************************

namespace {

uint8_t LowLevelReadByte(Gameboy* gameboy, uint16_t address) {
  uint8_t* base_ptr = (uint8_t*)&gameboy->memory;
  return base_ptr[address];
}

void LowLevelWriteByte(Gameboy* gameboy, uint16_t address, uint8_t value) {
  uint8_t* base_ptr = (uint8_t*)&gameboy->memory;

  // [0x0000 - 0x7fff]: ROM Bank 0, ROM Bank 1
  // Do nothing, not writeable!
  if (address < 0x8000)
    return;

  // [0x8000 - 0xbfff]: VRAM, Cartridge RAM
  if (address < 0xc000) {
    base_ptr[address] = value;
    return;
  }

  // [0xc000 - 0xdfff]: Internal RAM
  else if (address < 0xe000) {
    base_ptr[address] = value;

    // Internal RAM is 8kb, but RAM Echo is only 7.5kb .Copy to RAM Echo, add 8kb offset.
    if (address < 0xde00)
      base_ptr[address + 0x2000] = value;
    return;
  }

  // [0xE000 - 0xFDFF]: RAM Echo.
  if (address < 0xfe00) {
    base_ptr[address] = value;
    base_ptr[address - 0x2000] = value;  // 8kb offset
    return;
  }

  // [0xfe00 - 0xfe9f]: Sprite Attributes Memory (OAM).
  if (address < 0xfea0) {
    base_ptr[address] = value;
    // TODO(cristian): This is an old GB# TODO. Perhaps it's not worth it today.
    // TODO(Cristian): This is to see if OAM table is changed without DMA.
    //                 Perhaps the only way to access it is through DMA.
    //                 If that is the case, sprite sorting is greatly simplified
    return;
  }


  // [0xfea0 - 0xfeff]: Empty but unusable for I/O.
  if (address < 0xff00) {
      base_ptr[address] = value;
      return;
  }

  // [0xff00]: Joypad (R/W).
  if (address == 0xff00) {
    // Only the bits 4 and 5 are writable in P1.
    base_ptr[address] &= value & 0b00110000;

    // TODO(Cristian): Request an interrupt if necessary.
    /* this.gameboy.InterruptController.UpdateKeypadState(); */
    return;
  }

  // [0xff01-0xff02]: Serial communication. Not supported.
  // [0xff03]: Unused.
  if (address < 0xff04) {
    base_ptr[address] = value;
    return;
  }

  // [0xff04]: DIV register. Any write to DIV resets the register.
  if (address == 0xff04) {
    base_ptr[address] = 0;
    return;
  }

  // [0xff05-0xff07]: TIMA, TMA, TAC.
  if (address < 0xff08) {
    // TODO(Cristian): Plumb this to the CPU? Or have it poll when needed?
    NOT_IMPLEMENTED();
    return;
  }

  // [0xff08-0xff0e]: Unused.
  if (address < 0xff0f) {
    base_ptr[address] = value;
    return;
  }

  // [0xff0f]: Interrupt flag.
  if (address == 0xff0f) {
    // Only 4 bottom bits are required.
    base_ptr[address] = value | 0xe0;
    // TODO(Cristia): Trigger interrupt check event.
    /* this.cpu.CheckForInterruptRequired(); */
    NOT_IMPLEMENTED();
    return;
  }

  // ******* SOUND *******

  // [0xff10 - 0xff26]: Sound registers.
  if (address < 0xff27) {
    base_ptr[address] = value;
    OnAudioIO(gameboy, address);
    return;
  }

  // [0xff27-0xff2f]: Unused.
  if (address < 0xff2f) {
    base_ptr[address] = value;
    return;
  }

  // [0xff30-0xff3f]: Waveform data.
  if (address < 0xff40) {
    base_ptr[address] = value;
    OnAudioIO(gameboy, address);
    return;
  }

  // ******* VIDEO *******

  // [0xff40]: LCDC register.
  if (address == 0xff40) {
    base_ptr[address] = value;
    // We handle display memory changes
    // TODO(Cristian): See if the trigger is what we want.
    OnDisplayIO(gameboy, address);
    return;
  }

  // [0xff41-0xff45]: LCD STAT, SCY, SCX, LY, LYC.
  if (address < 0xff46) {
    base_ptr[address] = value;
    OnDisplayIO(gameboy, address);
    return;
  }

  // [0xff46]: Start DMA Transfer.
  if (address == 0xff46) {
    NOT_IMPLEMENTED();
    return;
  }

  // [0xff47-0xff4b]: BGP, OBP0, OBP1, WY, WX.
  if (address < 0xff4c) {
    base_ptr[address] = value;
    OnDisplayIO(gameboy, address);
    return;
  }

  // [0xff4c-0xff7f]: Unused.
  if (address < 0xff80) {
    base_ptr[address] = value;
    return;
  }

  // [0xff80-0xfffe]: High RAM (HRAM).
  if (address < 0xffff) {
    base_ptr[address] = value;
    return;
  }

  // [0xffff]: Interrupt enable.
  ASSERT(address == 0xffff);
  base_ptr[address] = value;

  // Trigger memory event check
  // TODO(Cristian): See how we want to communicate these events.
  /* this.cpu.CheckForInterruptRequired(); */
}

}  // namespace

// Extras ------------------------------------------------------------------------------------------

const char* ToString(CatridgeType type) {
  switch (type) {
    case CatridgeType::kROM_ONLY: return "ROM ONLY";
    case CatridgeType::kROM_MBC1: return "ROM MBC1";
    case CatridgeType::kROM_MBC1_RAM: return "ROM MBC1 RAM";
    case CatridgeType::kROM_MBC1_RAM_BATT: return "ROM MBC1 RAM BATT";
    case CatridgeType::kROM_MBC2: return "ROM MBC2";
    case CatridgeType::kROM_MBC2_BATTERY: return "ROM MBC2 BATTERY";
    case CatridgeType::kROM_RAM: return "ROM RAM";
    case CatridgeType::kROM_RAM_BATTERY: return "ROM RAM BATTERY";
    case CatridgeType::kROM_MMM01: return "ROM MMM01";
    case CatridgeType::kROM_MMM01_SRAM: return "ROM MMM01 SRAM";
    case CatridgeType::kROM_MMM01_SRAM_BATT: return "ROM MMM01 SRAM BATT";
    case CatridgeType::kROM_MBC3_TIMER_BATT: return "ROM MBC3 TIMER BATT";
    case CatridgeType::kROM_MBC3_TIMER_RAM_BATT: return "ROM MBC3 TIMER RAM BATT";
    case CatridgeType::kROM_MBC3: return "ROM MBC3";
    case CatridgeType::kROM_MBC3_RAM: return "ROM MBC3 RAM";
    case CatridgeType::kROM_MBC3_RAM_BATT: return "ROM MBC3 RAM BATT";
    case CatridgeType::kROM_MBC5: return "ROM MBC5";
    case CatridgeType::kROM_MBC5_RAM: return "ROM MBC5 RAM";
    case CatridgeType::kROM_MBC5_RAM_BATT: return "ROM MBC5 RAM BATT";
    case CatridgeType::kROM_MBC5_RUMBLE: return "ROM MBC5 RUMBLE";
    case CatridgeType::kROM_MBC5_RUMBLE_SRAM: return "ROM MBC5 RUMBLE SRAM";
    case CatridgeType::kROM_MBC5_RUMBLE_SRAM_BATT: return "ROM MBC5 RUMBLE SRAM BATT";
    case CatridgeType::kPocket_Camera: return "Pocket Camera";
    case CatridgeType::kBandai_TAMA5: return "Bandai TAMA5";
    case CatridgeType::kHudson_HuC3: return "Hudson HuC3";
    case CatridgeType::kHudson_HuC1: return "Hudson HuC1";
    case CatridgeType::kLast: return "<last>";
  }

  NOT_REACHED();
  return "<unknown>";
}

const char* ToString(GameboyType type) {
  switch (type) {
    case GameboyType::kGameboy: return "Gameboy";
    case GameboyType::kSuperGameboy: return "Super Gameboy";
    case GameboyType::kLast: return "<last>";
  }

  NOT_REACHED();
  return "<unknown>";
}

namespace {

GameboyType GetGameboyType(uint8_t value) {
  GameboyType type = (GameboyType)value;
  switch (type) {
    case GameboyType::kGameboy:
    case GameboyType::kSuperGameboy:
    case GameboyType::kLast: return type;
  }

  return GameboyType::kLast;
}

CatridgeType GetCatridgeType(uint8_t value) {
  CatridgeType type = (CatridgeType)value;
  switch (type) {
    case CatridgeType::kROM_ONLY:
    case CatridgeType::kROM_MBC1:
    case CatridgeType::kROM_MBC1_RAM:
    case CatridgeType::kROM_MBC1_RAM_BATT:
    case CatridgeType::kROM_MBC2:
    case CatridgeType::kROM_MBC2_BATTERY:
    case CatridgeType::kROM_RAM:
    case CatridgeType::kROM_RAM_BATTERY:
    case CatridgeType::kROM_MMM01:
    case CatridgeType::kROM_MMM01_SRAM:
    case CatridgeType::kROM_MMM01_SRAM_BATT:
    case CatridgeType::kROM_MBC3_TIMER_BATT:
    case CatridgeType::kROM_MBC3_TIMER_RAM_BATT:
    case CatridgeType::kROM_MBC3:
    case CatridgeType::kROM_MBC3_RAM:
    case CatridgeType::kROM_MBC3_RAM_BATT:
    case CatridgeType::kROM_MBC5:
    case CatridgeType::kROM_MBC5_RAM:
    case CatridgeType::kROM_MBC5_RAM_BATT:
    case CatridgeType::kROM_MBC5_RUMBLE:
    case CatridgeType::kROM_MBC5_RUMBLE_SRAM:
    case CatridgeType::kROM_MBC5_RUMBLE_SRAM_BATT:
    case CatridgeType::kPocket_Camera:
    case CatridgeType::kBandai_TAMA5:
    case CatridgeType::kHudson_HuC3:
    case CatridgeType::kHudson_HuC1:
    case CatridgeType::kLast:
      return type;
  }

  return CatridgeType::kLast;
}

uint32_t GetRomSize(uint8_t value) {
  switch (value) {
    case 0x00: return KILOBYTES(32);    // 256 Kbit =  32 KByte =   2 banks
    case 0x01: return KILOBYTES(64);    // 512 Kbit =  64 KByte =   4 banks
    case 0x02: return KILOBYTES(128);   //   1 Mbit = 128 KByte =   8 banks
    case 0x03: return KILOBYTES(256);   //   2 Mbit = 256 KByte =  16 banks
    case 0x04: return KILOBYTES(512);   //   4 Mbit = 512 KByte =  32 banks
    case 0x05: return MEGABYTES(1);     //   8 Mbit =   1 MByte =  64 banks
    case 0x06: return MEGABYTES(2);     //  16 Mbit =   2 MByte = 128 banks
    case 0x52: return KILOBYTES(1152);  //   9 Mbit = 1.1 MByte =  72 banks
    case 0x53: return KILOBYTES(1280);  //  10 Mbit = 1.2 MByte =  80 banks
    case 0x54: return KILOBYTES(1536);  //  12 Mbit = 1.5 MByte =  96 banks
    default: return 0;
  }
}

uint32_t GetRamSize(uint8_t value) {
  switch (value) {
    case 0x00: return 0;
    case 0x01: return KILOBYTES(2);    // 1 bank.
    case 0x02: return KILOBYTES(8);    // 1 banks.
    case 0x03: return KILOBYTES(32);   // 4 banks.
    case 0x04: return KILOBYTES(128);  // 16 banks.
    default: return 0xff;
  }
}

}  // namespace

}  // namespace emulator
}  // namespace rothko
