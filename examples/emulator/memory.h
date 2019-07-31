// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <rothko/utils/types.h>

namespace rothko {
namespace emulator {

// Memory layout for the gameboy:
//
// 0x0000-0x3fff    16KB ROM Bank 00     (in cartridge, fixed at bank 00)
// 0x4000-0x7fff    16KB ROM Bank 01..NN (in cartridge, switchable bank number)
// 0x8000-0x9fff    8KB Video RAM (VRAM) (switchable bank 0-1 in CGB Mode)
// - 0x8000-0x8fff  Tilemap0 - Overlaps with tilemap1.
// - 0x8800-0x97ff  Tilemap1 - Overlaps with tilemap0.
// - 0x9800-0x9bff  Background map 0.
// - 0x9c00-0x9fff  Background map 1.
// 0xa000-0xbfff    8KB External RAM     (in cartridge, switchable bank, if any)
// 0xc000-0xcfff    4KB Work RAM Bank 0 (WRAM)
// 0xd000-0xdfff    4KB Work RAM Bank 1 (WRAM)  (switchable bank 1-7 in CGB Mode)
// 0xe000-0xfdff    Same as C000-DDFF (ECHO)    (typically not used)
// 0xfe00-0xfe9f    Sprite Attribute Table (OAM)
// 0xfea0-0xfeff    Not Usable
// 0xff00-0xff7f    I/O Ports
// 0xff80-0xfffe    High RAM (HRAM)
// 0xffff           Interrupt Enable Register

// A tile is a 8x8 pixels. Each pixel is
constexpr int kBitsPerPixel = 2;
constexpr int kPixelsPerTileSide = 8;
struct Tile {
  uint8_t data[16];
};
static_assert(sizeof(Tile) == 16);

// OAM ---------------------------------------------------------------------------------------------

// OAM is the Sprite Attribute entry, that describes a single sprite being drawn.
struct OAMEntry {
  // Vertical position on the screen (minus 16).
  // This means that y = 0 or y >= 160 hides the sprite.
  uint8_t y;

  // Horizontal position on the screen (minus 8).
  // This means that x = 0 or x >= 168 hides the sprite.
  uint8_t x;

  // Unsigned index into tilemap0.
  uint8_t tile_number;

  // Attributes/Flags. Use the getter macros below.
  //  Bit2-0 Palette number  **CGB Mode Only**     (OBP0-7)
  //  Bit3   Tile VRAM-Bank  **CGB Mode Only**     (0=Bank 0, 1=Bank 1)
  //  Bit4   Palette number  **Non CGB Mode Only** (0=OBP0, 1=OBP1)
  //  Bit5   X flip          (0=Normal, 1=Horizontally mirrored)
  //  Bit6   Y flip          (0=Normal, 1=Vertically mirrored)
  //  Bit7   OBJ-to-BG Priority (0=OBJ Above BG, 1=OBJ Behind BG color 1-3)
  //    (Used for both BG and Window. BG color 0 is always behind OBJ)
  uint8_t flags;
};
static_assert(sizeof(OAMEntry) == 4);

#define OAM_PALLETE_NUMBER_GBC(flags) (flags & 0b00000111)
#define OAM_TILE_VRAM_BANK(flags)     (flags & 0b00001000)
#define OAM_PALLETE_NUMBER(flags)     (flags & 0b00010000)
#define OAM_X_FLIP(flags)             (flags & 0b00100000)
#define OAM_Y_FLIP(flags)             (flags & 0b01000000)
#define OAM_OBJ_TO_BG_PRIORITY(flags) (flags & 0b10000000)

// Memory ------------------------------------------------------------------------------------------

// VRAM = Video RAM.
struct VRAM {
  // 0x8000-0x8fff  Tilemap0 - Overlaps with tilemap1.
  // 0x8800-0x97ff  Tilemap1 - Overlaps with tilemap0.
  Tile tiles[384];
  // 0x9800-0x9bff  Background map 0.
  uint8_t background_map0[32 * 32];
  // 0x9c00-0x9fff  Background map 1.
  uint8_t background_map1[32 * 32];
};

// IOPorts are memory mapped memory to functionality registers (sound, joystick, etc.).
struct IOPorts {
  // TODO(Cristian): Fill in.
};

struct Memory {
  uint8_t rom_bank0[KILOBYTES(16)];
  uint8_t rom_banks[KILOBYTES(16)];

  VRAM vram;

  static_assert(sizeof(VRAM) == KILOBYTES(8));

  // 0xa000-0xbfff    8KB External RAM     (in cartridge, switchable bank, if any)
  uint8_t external_ram[KILOBYTES(8)];

  // 0xc000-0xcfff    4KB Work RAM Bank 0 (WRAM)
  // 0xd000-0xdfff    4KB Work RAM Bank 1 (WRAM)  (switchable bank 1-7 in CGB Mode)
  uint8_t work_ram[KILOBYTES(8)];

  // 0xe000-0xfdff    Same as C000-DDFF (ECHO)    (typically not used)
  //                  This is replicated from |work_ram| due to gameboy wiring.
  // The last 512 bytes are wired differently.
  uint8_t echo[KILOBYTES(8) - 512];

  // 0xfe00-0xfe9f    Sprite Attribute Table (OAM)
  OAMEntry oam_table[40];

  // 0xfea0-0xfeff    Not Usable
  uint8_t unused[96];

  // 0xff00-0xff7f    I/O Ports
  union {
    IOPorts io_ports;
    uint8_t data[128];
  } io_ports;

  // 0xff80-0xfffe    High RAM (HRAM)
  uint8_t hram[127];

  // 0xffff           Interrupt Enable Register
  uint8_t interrupt_enable_register;
};
static_assert(sizeof(Memory) == KILOBYTES(64));

}  // namespace emulator
}  // namespace rothko
