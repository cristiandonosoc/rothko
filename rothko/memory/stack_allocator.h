// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <assert.h>
#include <stdint.h>

#include <memory>

namespace rothko {

struct StackAllocator {
  uint32_t size = 0;
  uint32_t current = 0;

  std::unique_ptr<uint8_t[]> data_;   // Should not be accessed directly.
};

inline bool Valid(const StackAllocator& sa) { return !!sa.data_; }
inline void Reset(StackAllocator* sa) { sa->current = 0; }

StackAllocator CreateStackAllocator(uint32_t size);

template <typename T>
StackAllocator CreateStackAllocatorFor(uint32_t count) {
  return CreateStackAllocator(sizeof(T) * count);
}

template <typename T>
T* Allocate(StackAllocator* sa, uint32_t count = 1) {
  if (!Valid(*sa))
    return nullptr;

  uint32_t size_to_alloc = sizeof(T) * count;
  if (sa->current + size_to_alloc > sa->size) {
    assert(false);
    return nullptr;
  }


  // Get the current pointer and cast it to the correct type.
  T* ptr = (T*)(sa->data_.get() + sa->current);
  sa->current += size_to_alloc;

  return ptr;
}

}  // namespace rothko
