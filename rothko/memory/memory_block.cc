// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/memory/memory_block.h"

#include "rothko/utils/logging.h"
#include "rothko/memory/block_allocator.h"

namespace rothko {

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

bool MemoryBlock::operator==(const MemoryBlock& other) const {
  return this->allocator == other.allocator &&
         this->index == other.index &&
         this->size == other.size;
}

bool MemoryBlock::operator!=(const MemoryBlock& other) const {
  return !(this->operator==(other));
}

}  // namespace rothko
