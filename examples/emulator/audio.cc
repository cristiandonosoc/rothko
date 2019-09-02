// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "audio.h"

namespace rothko {
namespace emulator {

void OnAudioIO(Gameboy* gameboy, uint64_t address) {
  (void)gameboy;
  (void)address;
  // [0xff10 - 0xff26]: Sound registers.
  /* this.apu.HandleMemoryChange((MMR)address, value); */

  // [0xff30-0xff3f]: Waveform data.
  /* this.apu.HandleWaveWrite(address, value); */
}

}  // namespace emulator
}  // namespace rothko
