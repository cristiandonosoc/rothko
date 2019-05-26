// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/memory/block_allocator.h"

namespace rothko {

// MemoryBlock -----------------------------------------------------------------

namespace {

void Clear(MemoryBlock* block) {
  block->allocator = nullptr;
  block->index = -1;
  block->size = 0;
}

void Move(MemoryBlock* from, MemoryBlock* to) {
  to->allocator = from->allocator;
  to->index = from->index;
  to->size = from->size;
  Clear(from);
}

}  // namespace

MemoryBlock::~MemoryBlock() {
  if (!Valid(this))
    return;
  this->allocator->Deallocate(this->index);
}

MemoryBlock::MemoryBlock(MemoryBlock&& other) {
  Move(&other, this);
}

MemoryBlock&
MemoryBlock::operator=(MemoryBlock&& other) {
  if (this == &other)
    return *this;

  if (Valid(this)) {
    this->allocator->Deallocate(this->index);
    Clear(this);
  }

  Move(&other, this);
  return *this;
}

bool Valid(MemoryBlock* block) {
  return !!block->allocator && block->index >= 0 && block->size != 0;
}

uint8_t* Data(MemoryBlock* block) {
  ASSERT(Valid(block));
  return block->allocator->GetBlockMemory(block->index);
}

}  // namespace rothko
