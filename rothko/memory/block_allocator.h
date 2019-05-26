// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>
#include <inttypes.h>

#include <mutex>

#include "rothko/utils/intrinsics.h"
#include "rothko/utils/logging.h"
#include "rothko/utils/macros.h"
#include "rothko/utils/thread_annotations.h"

namespace rothko {

// API (Implemented at the end because of ugly template code.
//
// These are member functions because of template deduction failure.
//
// bool MemoryBlock::valid()
// uint8_t MemoryBlock::data();
//
// MemoryBlock Allocate(BlockAllocator*);
// -> Returns an invalid block if no more space.
//
// void Deallocate(BlockAllocator*);

template <uint64_t BlockSize>
struct BlockAllocator {
  static constexpr uint64_t kBlockSize = BlockSize;
  static constexpr uint64_t kBlockCount = 64u;

  // Block Allocators do not move.
  BlockAllocator();
  DELETE_COPY_AND_ASSIGN(BlockAllocator);
  DELETE_MOVE_AND_ASSIGN(BlockAllocator);

  struct MemoryBlock {
    static constexpr uint64_t kSize = BlockSize;

    MemoryBlock() = default;
    ~MemoryBlock();
    DELETE_COPY_AND_ASSIGN(MemoryBlock);
    DECLARE_MOVE_AND_ASSIGN(MemoryBlock);

    bool valid() const;
    uint8_t* data();

    BlockAllocator* allocator = nullptr;
    int32_t index = -1;
  };

  // Fields.
  uint8_t memory[kBlockSize * kBlockCount];
  // Allows for 64 blocks.
  // The bit set means the block is free.
  uint64_t block_bitset GUARDED_BY(mutex) = U64_ALL_ONES();
  int used_blocks = 0;

  std::mutex mutex;
};

// Implementations -------------------------------------------------------------

#define MEMORY_BLOCK(BlockSize) typename BlockAllocator<BlockSize>::MemoryBlock

constexpr uint64_t kAllOnes = -1;

template <uint64_t BlockSize>
BlockAllocator<BlockSize>::BlockAllocator()
    : block_bitset(kAllOnes), used_blocks(0) {}

template <uint64_t BlockSize>
MEMORY_BLOCK(BlockSize)
Allocate(BlockAllocator<BlockSize>* allocator) {
  int free_block_index = 0;
  {
    std::lock_guard<std::mutex>(allocator->mutex);
    if (allocator->used_blocks == allocator->kBlockCount)
      return {};  // Invalid block;

    free_block_index = FIND_FIRST_SET(allocator->block_bitset);
    allocator->block_bitset &= ~(((uint64_t)1u) << (free_block_index - 1));
    allocator->used_blocks++;
  }

  /* static int alloc_count = 0; */
  /* alloc_count++; */
  /* LOG(DEBUG, "Alloc %d: Block bitset: 0x%zx", alloc_count, */
  /*                                             allocator->block_bitset); */

  ASSERT(free_block_index > 0);

  typename BlockAllocator<BlockSize>::MemoryBlock block = {};
  block.allocator = allocator;
  block.index = free_block_index - 1;

  return block;
}

template <uint64_t BlockSize>
void Deallocate(BlockAllocator<BlockSize>* allocator, int index) {
  ASSERT(index >= 0 && index < 64);

  /* static int dealloc_count = 0; */
  /* dealloc_count++; */
  /* LOG(DEBUG, "Dealloc %d: Block bitset: 0x%zx", dealloc_count, */
  /*                                               allocator->block_bitset); */

  bool was_set = false;
  {
    std::lock_guard<std::mutex>(allocator->mutex);
    was_set = ((allocator->block_bitset & (1lu << index)) == 0);
    allocator->block_bitset |= (((uint64_t)1lu) << index);
    allocator->used_blocks--;
  }

  ASSERT(was_set);
}

// MemoryBlock -----------------------------------------------------------------

namespace {

template <uint64_t BlockSize>
void Clear(MEMORY_BLOCK(BlockSize)* block) {
  block->allocator = nullptr;
  block->index = -1;
}

template <uint64_t BlockSize>
void Move(MEMORY_BLOCK(BlockSize)* from, MEMORY_BLOCK(BlockSize)* to) {
  to->allocator = from->allocator;
  to->index = from->index;
  Clear<BlockSize>(from);
}

}  // namespace

template <uint64_t BlockSize>
BlockAllocator<BlockSize>::MemoryBlock::~MemoryBlock() {
  if (!this->valid())
    return;
  Deallocate<BlockSize>(this->allocator, this->index);
}

template <uint64_t BlockSize>
BlockAllocator<BlockSize>::MemoryBlock::MemoryBlock(MemoryBlock&& other) {
  Move<BlockSize>(&other, this);
}

template <uint64_t BlockSize>
MEMORY_BLOCK(BlockSize)&
BlockAllocator<BlockSize>::MemoryBlock::operator=(MemoryBlock&& other) {
  if (this == &other)
    return *this;
  if (this->valid()) {
    Deallocate<BlockSize>(this->allocator, this->index);
    Clear<BlockSize>(this);
  }

  Move<BlockSize>(&other, this);
  return *this;
}

template <uint64_t BlockSize>
bool BlockAllocator<BlockSize>::MemoryBlock::valid() const {
  return !!this->allocator && this->index >= 0;
}

template <uint64_t BlockSize>
uint8_t* BlockAllocator<BlockSize>::MemoryBlock::data() {
  ASSERT(this->valid());
  uint8_t* base_ptr = (uint8_t*)this->allocator;
  return base_ptr + (this->index * this->size);
}

}  // namespace rothko
