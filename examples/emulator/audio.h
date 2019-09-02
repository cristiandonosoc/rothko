// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace rothko {
namespace emulator {

struct Gameboy;

struct Audio {

};

void OnAudioIO(Gameboy*, uint64_t address);

}  // namespace emulator
}  // namespace rothko

