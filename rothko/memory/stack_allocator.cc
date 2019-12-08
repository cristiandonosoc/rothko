// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/memory/stack_allocator.h"


namespace rothko {

StackAllocator CreateStackAllocator(uint32_t size) {
  StackAllocator sa = {};
  sa.data_ = std::make_unique<uint8_t[]>(size);
  sa.size = size;
  return sa;
}

}  // namespace rothko
