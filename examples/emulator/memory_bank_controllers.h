// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace rothko {
namespace emulator {

struct Memory;

enum class MBCType {
  kBasic,
  kLast,
};
const char* ToString(MBCType);

struct MBCApi{
  using ReadFunction = uint8_t (*)(Memory*, uint64_t address);
  template <typename T>
  using WriteFunction = void (*)(Memory*, uint64_t address, T value);

  MBCType type = MBCType::kLast;

  ReadFunction Read = nullptr;
  WriteFunction<uint8_t> WriteByte = nullptr;
  WriteFunction<uint16_t> WriteShort = nullptr;
};
inline bool Valid(const MBCApi& mbc) { return mbc.type != MBCType::kLast; }

MBCApi GetMBCApi(MBCType);

}  // namespace emulator
}  // namespace rothko
