// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <utility>

namespace rothko {
namespace emulator {

struct Instruction;
struct Gameboy;

void ExecuteInstruction(Gameboy*, const Instruction&);

}  // namespace emulator
}  // namespace rothko
