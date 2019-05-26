// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "warhol/utils/macros.h"

namespace rothko {

struct BlockAllocator;

struct MemoryBlock {
  MemoryBlock() = default;
  ~MemoryBlock();
  DELETE_COPY_AND_ASSIGN(MemoryBlock);
  DECLARE_MOVE_AND_ASSIGN(MemoryBlock);

  BlockAllocator* allocator = nullptr;
  int32_t index = -1;
  uint32_t size = 0;
};

bool Valid(MemoryBlock*);
uint8_t* Data(MemoryBlock*);



}  // namespace rothko
