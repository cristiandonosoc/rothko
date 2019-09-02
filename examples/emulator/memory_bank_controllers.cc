// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "memory_bank_controllers.h"

#include <rothko/logging/logging.h>

namespace rothko {
namespace emulator {

// Low Level Memory "API" --------------------------------------------------------------------------
//
// These are the functions of memory manipulation that are common to all Memory Bank Controllers.
// In general, MBCs will do work before calling this (like swapping memory banks).
//
// Defined at the bottom of the file.

namespace {

uint8_t LowLevelRead(Memory* memory, uint64_t address);
void LowLevelWrite(Memory* memory, uint64_t address, uint8_t value);

}  // namespace

// Basic MBC ---------------------------------------------------------------------------------------

namespace {

void BasicMBCWriteByte(Memory* memory, uint64_t address, uint8_t value) {
  LowLevelWrite(memory, address, value);
}

void BasicMBCWriteShort(Memory* memory, uint64_t address, uint16_t value) {
  uint8_t* ptr = (uint8_t*)&value;
  BasicMBCWriteByte(memory, address++, *ptr++);
  BasicMBCWriteByte(memory, address, *ptr);
}

MBCApi GetBasicMBC() {
  MBCApi api = {};
  api.type = MBCType::kBasic;
  api.Read = LowLevelRead;
  api.WriteByte = BasicMBCWriteByte;
  api.WriteShort = BasicMBCWriteShort;

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

uint8_t LowLevelRead(Memory* memory, uint64_t address) {
  uint8_t* base_ptr = (uint8_t*)&memory;
  return base_ptr[address];
}

void LowLevelWrite(Memory* memory, uint64_t address, uint8_t value) {
  uint8_t* base_ptr = (uint8_t*)&memory;

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

  // [0xff10 - 0xff26]: Sound registers.
  if ((0xff10 <= address) && (address <= 0xff26)) {
    // NOTE(Cristian): Memory change in the sound register area are very specific, so all the
    //                 changes to the actual values are made by the APU and sound channels
    //                 themselves.
    // TODO(Cristian): See if the above is the way we want to go.
    /* this.apu.HandleMemoryChange((MMR)address, value); */
    NOT_IMPLEMENTED();
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
    // TODO(Cristian): See if this requires special handling by the APU.
    /* this.apu.HandleWaveWrite(address, value); */
    NOT_IMPLEMENTED();
    return;
  }

  // [0xff40]: LCDC register.
  if (address == 0xff40) {
    base_ptr[address] = value;
    // We handle display memory changes
    // TODO(Cristian): See if the trigger is what we want.
    /* this.display.HandleMemoryChange((MMR)address, value); */
    NOT_IMPLEMENTED();
  }

  // [0xff41-0xff45]: LCD STAT, SCY, SCX, LY, LYC.
  if (address < 0xff46) {
    base_ptr[address] = value;
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

}  // namespace emulator
}  // namespace rothko
