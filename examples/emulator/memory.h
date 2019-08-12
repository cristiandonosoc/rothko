// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/logging/logging.h>
#include <rothko/utils/types.h>
#include <stddef.h>
#include <stdint.h>

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

#define OAM_PALLETE_NUMBER_GBC(flags) (flags & 0b00000111)
#define OAM_TILE_VRAM_BANK(flags)     (flags & 0b00001000)
#define OAM_PALLETE_NUMBER(flags)     (flags & 0b00010000)
#define OAM_X_FLIP(flags)             (flags & 0b00100000)
#define OAM_Y_FLIP(flags)             (flags & 0b01000000)
#define OAM_OBJ_TO_BG_PRIORITY(flags) (flags & 0b10000000)

// Memory ------------------------------------------------------------------------------------------

// VRAM = Video RAM.
struct VRAM {
  // 0x8000-0x8fff  Tilemap0 - Overlaps with tilemap1. (0-255)
  // 0x8800-0x97ff  Tilemap1 - Overlaps with tilemap0. (127-383)
  Tile tiles[384];

  // Tilemaps.
  // These tiles are shared by both background and window. Which one it is and how the map to the
  // actual tiles stored in |tiles| is determined by the |lcdc| register in |MappedIO|.

  // 0x9800-0x9bff  Tilemap 0.
  uint8_t tilemap0[32 * 32];
  // 0x9c00-0x9fff  Tilemap 1.
  uint8_t tilemap1[32 * 32];
};

// MappedIO ----------------------------------------------------------------------------------------
//
// MappedIO are memory mapped memory to functionality registers (sound, joystick, etc.).

struct MappedIO {
    uint8_t joypad;   // 0xff00: Joystick. TODO(Cristian): Do access macros.

    // Serial communication registers.
    uint8_t sb;       // 0xff01: Serial transfer data (R/W).
    uint8_t sc;       // 0xff02: Serial transfer control. // TODO(Cristian): Do access macros.

    uint8_t __pad0;

    // Timer registers.
    uint8_t div;      // 0xff04: Frequency divider; upper 8 bits of the 16 bit clock counter.
    uint8_t tima;     // 0xff05: Timer counter.
    uint8_t tma;      // 0xff06:  Timer modulo.
    uint8_t tac;      // 0xff07:  Timer controller.

    uint8_t __pad1[7];

    uint8_t ifr;      // 0xff0f: Interrupt frag.  // TODO(Cristian): Do access macros.

    // sound registers.
    uint8_t nr10;     // 0xff10
    uint8_t nr11;     // 0xff11
    uint8_t nr12;     // 0xff12
    uint8_t nr13;     // 0xff13
    uint8_t nr14;     // 0xff14

    uint8_t __pad2;

    uint8_t nr21;     // 0xff16
    uint8_t nr22;     // 0xff17
    uint8_t nr23;     // 0xff18
    uint8_t nr24;     // 0xff19

    uint8_t nr30;     // 0xff1a
    uint8_t nr31;     // 0xff1b
    uint8_t nr32;     // 0xff1c
    uint8_t nr33;     // 0xff1d
    uint8_t nr34;     // 0xff1e

    uint8_t __pad3;

    uint8_t nr41;     // 0xff20
    uint8_t nr42;     // 0xff21
    uint8_t nr43;     // 0xff22
    uint8_t nr44;     // 0xff23

    uint8_t nr50;     // 0xff24
    uint8_t nr51;     // 0xff25
    uint8_t nr52;     // 0xff26

    uint8_t __pad4[9];

    // 0xff30 - 0xff3f: Waveform storage for arbitrary sound data.
    //                  Holds 32 4-bit samples that are play back upper 4 bits first.
    uint8_t wfram[16];

    // LCD registers.
    uint8_t lcdc;     // 0xff40: LCD Control.
// Whether to display the background. If 0, the background is white.
#define LCDC_BG_DISPLAY(lcdc)                           (lcdc & 0b00000001)
// Whether to show OAM sprites.
#define LCDC_OBJ_SPRITE_ENABLE(lcdc)                    (lcdc & 0b00000010)
// Size of sprites. 0 = 8x8 pixels, 1 = 8x16 pixels.
#define LCDC_OBJ_SPRITE_SIZE(lcdc)                      (lcdc & 0b00000100)
// What tilemap to use for background. 0 = |tilemap0|, 1 = |tilemap1|.
#define LCDC_BG_TILE_MAP_DISPLAY_SELECT(lcdc)           (lcdc & 0b00001000)
// What tile mapping the tilemaps point to within |tiles|.
// 0 = Tiles [0, 256). The tilemap value is interpret as uint8_t [0, 256).
// 1 = Tiles [128, 384). The tilemap value is interpreted as int8_t [-128, 128), and are an offset
//     from tile 256.
#define LCDC_BG_WINDOW_TILE_DATA_SELECT(lcdc)           (lcdc & 0b00010000)
// Whether to show the window display.
#define LCDC_WINDOW_DISPLAY_ENABLE(lcdc)                (lcdc & 0b00100000)
// What tilemap to use for the window. 0 = |tilemap0|, 1 = |tilemap1|.
#define LCDC_WINDOW_TILE_MAP_DISPLAY_SELECT(lcdc)       (lcdc & 0b01000000)
// Whether the display is enabled or not.
#define LCDC_DISPLAY_ENABLE(lcdc)                       (lcdc & 0b10000000)

    uint8_t stat;     // 0xff41: LCD Status. TODO(Cristian): Do access macros.
    uint8_t scy;      // 0xff42: BG Scroll Y. Window automatically wraps borders.
    uint8_t scx;      // 0xff43: BG Scroll X. Window automatically wraps borders.
    uint8_t ly;       // 0xff44: LCD Y-coord. Indicates which line is being transfered.
                      //         Values 144-153 indicate V-Blank period.
    uint8_t lyc;      // 0xff45: LY Compare. When |ly| == |lyc|, a bit in |stat| is set.
    uint8_t dma;      // 0xff46: DMA Transfer address.

    uint8_t bgp;      // 0xff47: BG Palette data. Determines colors for BG and window pixels.
                      //         Bit 0-1 - Color for shade number 0
                      //         Bit 2-3 - Color for shade number 1
                      //         Bit 4-5 - Color for shade number 2
                      //         Bit 6-7 - Color for shade number 3

    // For |obp0| and |obp1|, work the same as |bgp|, except the lower 2 bits are not used, as
    // sprite data 00 is transparent.
    uint8_t obp0;     // 0xff48: Object Palette 0. Colors for sprite pixels in palette 0.
    uint8_t obp1;     // 0xff49: Object Palette 1. Colors for sprite pixels in palette 1.

    // Window is visible (if enable) when |wx| in [0, 166] and |wy| in [0, 143].
    // The window is offset, so a position of (7, 0) locates the window in the upper left corner,
    // completely covering normal background.
    uint8_t wy;       // 0xff4a: Window Y.
    uint8_t wx;       // 0xff4b: Window X - 7.

    uint8_t __pad_final[52];
};
// Extracts palette colors |bgp|, |obp0| and |obp1|. Valid indices are 0-3.
// NOTE: |obp0| and |obp1| lower 2 bits (PALLETE_COLOR(<reg>, 0) is unused, as those bits are
//       reserved for transparent color.
inline uint32_t PaletteColor(uint8_t reg, uint32_t index) { return reg >> (2u * index) & 0b11u; }

// GB Memory Layout --------------------------------------------------------------------------------

struct Memory {
  uint8_t rom_bank0[KILOBYTES(16)];
  uint8_t rom_banks[KILOBYTES(16)];

  VRAM vram;

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

  // 0xff00-0xff7f    I/O Ports (128 bytes).
  MappedIO mapped_io;

  // 0xff80-0xfffe    High RAM (HRAM)
  uint8_t hram[127];

  // 0xffff           Interrupt Enable Register
  uint8_t interrupt_enable_register;
};

// At address 0x104 the nintendo header is located and should always be there.
inline bool Loaded(const Memory& memory) {
  uint32_t val = *(uint32_t*)(memory.rom_bank0 + 0x104);
  return val == 0x6666edce;
}

// Static assert checks ----------------------------------------------------------------------------

static_assert(sizeof(Memory) == KILOBYTES(64));

static_assert(sizeof(VRAM) == KILOBYTES(8));

static_assert(sizeof(Tile) == 16);

static_assert(sizeof(OAMEntry) == 4);

static_assert(sizeof(MappedIO) == 128);
static_assert(0xff00 + offsetof(MappedIO, joypad) == 0xff00);
static_assert(0xff00 + offsetof(MappedIO, sb) == 0xff01);
static_assert(0xff00 + offsetof(MappedIO, sc) == 0xff02);
static_assert(0xff00 + offsetof(MappedIO, div) == 0xff04);
static_assert(0xff00 + offsetof(MappedIO, tima) == 0xff05);
static_assert(0xff00 + offsetof(MappedIO, tma) == 0xff06);
static_assert(0xff00 + offsetof(MappedIO, tac) == 0xff07);
static_assert(0xff00 + offsetof(MappedIO, ifr) == 0xff0f);
static_assert(0xff00 + offsetof(MappedIO, nr10) == 0xff10);
static_assert(0xff00 + offsetof(MappedIO, nr11) == 0xff11);
static_assert(0xff00 + offsetof(MappedIO, nr12) == 0xff12);
static_assert(0xff00 + offsetof(MappedIO, nr13) == 0xff13);
static_assert(0xff00 + offsetof(MappedIO, nr14) == 0xff14);
static_assert(0xff00 + offsetof(MappedIO, nr21) == 0xff16);
static_assert(0xff00 + offsetof(MappedIO, nr22) == 0xff17);
static_assert(0xff00 + offsetof(MappedIO, nr23) == 0xff18);
static_assert(0xff00 + offsetof(MappedIO, nr24) == 0xff19);
static_assert(0xff00 + offsetof(MappedIO, nr30) == 0xff1a);
static_assert(0xff00 + offsetof(MappedIO, nr31) == 0xff1b);
static_assert(0xff00 + offsetof(MappedIO, nr32) == 0xff1c);
static_assert(0xff00 + offsetof(MappedIO, nr33) == 0xff1d);
static_assert(0xff00 + offsetof(MappedIO, nr34) == 0xff1e);
static_assert(0xff00 + offsetof(MappedIO, nr41) == 0xff20);
static_assert(0xff00 + offsetof(MappedIO, nr42) == 0xff21);
static_assert(0xff00 + offsetof(MappedIO, nr43) == 0xff22);
static_assert(0xff00 + offsetof(MappedIO, nr44) == 0xff23);
static_assert(0xff00 + offsetof(MappedIO, nr50) == 0xff24);
static_assert(0xff00 + offsetof(MappedIO, nr51) == 0xff25);
static_assert(0xff00 + offsetof(MappedIO, nr52) == 0xff26);
static_assert(0xff00 + offsetof(MappedIO, wfram) == 0xff30);
static_assert(0xff00 + offsetof(MappedIO, lcdc) == 0xff40);
static_assert(0xff00 + offsetof(MappedIO, stat) == 0xff41);
static_assert(0xff00 + offsetof(MappedIO, scy) == 0xff42);
static_assert(0xff00 + offsetof(MappedIO, scx) == 0xff43);
static_assert(0xff00 + offsetof(MappedIO, ly) == 0xff44);
static_assert(0xff00 + offsetof(MappedIO, lyc) == 0xff45);
static_assert(0xff00 + offsetof(MappedIO, dma) == 0xff46);
static_assert(0xff00 + offsetof(MappedIO, bgp) == 0xff47);
static_assert(0xff00 + offsetof(MappedIO, obp0) == 0xff48);
static_assert(0xff00 + offsetof(MappedIO, obp1) == 0xff49);
static_assert(0xff00 + offsetof(MappedIO, wy) == 0xff4a);
static_assert(0xff00 + offsetof(MappedIO, wx) == 0xff4b);

}  // namespace emulator
}  // namespace rothko
