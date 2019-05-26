// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <cstring>
#include <utility>

#include "rothko/memory/memory_block.h"
#include "rothko/utils/macros.h"

namespace rothko {

template <typename T, typename Allocator>
struct Vector {
  Vector(int size = 4);
  ~Vector() = default;
  DELETE_COPY_AND_ASSIGN(Vector);
  DECLARE_MOVE_AND_ASSIGN(Vector);

  T& operator[](int index);
  T& at(int index);

  MemoryBlock memory_block;
  T* data = nullptr;
  uint32_t size = 0;
  uint32_t count = 0;
};

// API -------------------------------------------------------------------------

#define PREAMBLE  template <typename T, typename Allocator>
#define VECTOR Vector<T, Allocator>

PREAMBLE bool Valid(VECTOR*);
PREAMBLE void PushBack(VECTOR*);
PREAMBLE T PopBack(VECTOR*);

// Implementation --------------------------------------------------------------

namespace internal {

PREAMBLE void Resize(VECTOR* vector, int size_needed) {
  // We need to allocate more space.
  int bytes_needed = size_needed * sizeof(T);

  MemoryBlock new_block = Allocator::SmallestBlock(bytes_needed);

  vector->size = new_block.size / sizeof(T);

  // We copy over the current contents.
  uint8_t* new_data = Data(&new_block);
  std::memcpy(new_data, vector->data, vector->count * sizeof(T));

  // Replace the block (will deallocate the old one).
  vector->memory_block = std::move(new_block);
  vector->data = new_data;
}

PREAMBLE void Clear(VECTOR* vector) {
  vector->memory_block = {};
  vector->data = nullptr;
  vector->size = 0;
  vector->count = 0;
}

PREAMBLE void Move(VECTOR* from, VECTOR* to) {
  to->memory_block = std::move(from->memory_block);
  to->data = from->data;
  to->size = from->size;
  to->count = from->count;
  Clear(from);
}

}  // namespace internal

PREAMBLE VECTOR::Vector(int size) : size(size) {
  Resize(this, size);
}

PREAMBLE VECTOR::Vector(Vector&& other) {
  Move(&other, this);
}

PREAMBLE VECTOR& VECTOR::operator=(Vector&& other) {
  if (this == &other)
    return *this;

  if (Valid(this))
    Clear(this);

  Move(&other, this);
  return *this;
}

PREAMBLE bool Valid(VECTOR* vector) {
  return !!vector->data;
}

PREAMBLE void PushBack(VECTOR* vector, T t) {
  ASSERT(Valid(vector));
  if (vector->size == vector->count) {
    // Double the vector each time.
    int size_needed = 2 * vector->size;
    Resize(vector, size_needed);
  }

  // We now can push the object in.
  vector->data[vector->count] = std::move(t);
  vector->count++;
}

PREAMBLE T PopBack(VECTOR* vector) {
  ASSERT(Valid(vector));
  ASSERT(vector->count > 0);
  T t = std::move(vector->data[vector->count]);
  vector->count--;
  return t;
}

PREAMBLE T& VECTOR::operator[](int index) {
  return this->at(index);
}

PREAMBLE T& VECTOR::at(int index) {
  ASSERT(Valid(this));
  ASSERT(index >= 0 && index < this->count);
  return this->data[index];
}

#undef PREAMBLE

}  // namespace rothko
