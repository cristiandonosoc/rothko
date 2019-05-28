// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <cstring>
#include <utility>

#include "rothko/memory/memory_block.h"
#include "rothko/utils/logging.h"
#include "rothko/utils/macros.h"

namespace rothko {

template <typename T, typename Allocator>
struct Vector {
  static constexpr uint32_t kDefaultSize = 4;

  Vector(uint32_t size = kDefaultSize);
  ~Vector() = default;
  DELETE_COPY_AND_ASSIGN(Vector);
  DECLARE_MOVE_AND_ASSIGN(Vector);

  T& operator[](uint32_t index);
  T& at(uint32_t index);

  void push_back(T);
  T pop_back();

  MemoryBlock memory_block;
  T* data = nullptr;
  uint32_t size = 0;
  uint32_t count = 0;
};

// API -------------------------------------------------------------------------

#define PREAMBLE  template <typename T, typename Allocator>
#define VECTOR Vector<T, Allocator>

PREAMBLE bool Valid(VECTOR*);

// Implementation --------------------------------------------------------------

namespace internal {

PREAMBLE void Resize(VECTOR* vector, uint32_t size_needed) {
  // We need to allocate more space.
  int bytes_needed = size_needed * sizeof(T);

  MemoryBlock new_block = Allocator::SmallestBlock(bytes_needed);

  vector->size = new_block.size / sizeof(T);

  // We copy over the current contents.
  uint8_t* new_data = Data(&new_block);
  std::memcpy(new_data, vector->data, vector->count * sizeof(T));

  // Replace the block (will deallocate the old one).
  vector->memory_block = std::move(new_block);
  vector->data = (T*)new_data;
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

PREAMBLE VECTOR::Vector(uint32_t size) : size(size) {
  ::rothko::internal::Resize(this, size);
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

PREAMBLE void VECTOR::push_back(T t) {
  ASSERT(Valid(this));
  if (this->size == this->count) {
    // Double the vector each time.
    int size_needed = 2 * this->size;
    ::rothko::internal::Resize(this, size_needed);
  }

  // We now can push the object in.
  this->data[this->count] = std::move(t);
  this->count++;
}

PREAMBLE T VECTOR::pop_back() {
  ASSERT(Valid(this));
  ASSERT(this->count > 0);
  T t = std::move(this->data[this->count]);
  this->count--;
  return t;
}

PREAMBLE T& VECTOR::operator[](uint32_t index) {
  return this->at(index);
}

PREAMBLE T& VECTOR::at(uint32_t index) {
  ASSERT(Valid(this));
  ASSERT(index >= 0 && index < this->count);
  return this->data[index];
}

#undef PREAMBLE
#undef VECTOR

}  // namespace rothko
