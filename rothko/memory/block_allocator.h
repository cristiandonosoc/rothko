// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <inttypes.h>
#include <stdint.h>

#include <mutex>

#include "rothko/memory/memory_block.h"
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
// MemoryBlock Allocate(SizedBlockAllocator*);
// -> Returns an invalid block if no more space.
//
// void Deallocate(SizedBlockAllocator*);

struct BlockAllocator {
  virtual MemoryBlock Allocate() = 0;
  virtual void Deallocate(int index) = 0;
  virtual uint8_t* GetBlockMemory(int index) = 0;
};

template <uint64_t BlockSize>
struct SizedBlockAllocator : public BlockAllocator {
  static constexpr uint64_t kBlockSize = BlockSize;
  static constexpr uint64_t kBlockCount = 64u;

  // Block Allocators do not move.
  SizedBlockAllocator();
  DELETE_COPY_AND_ASSIGN(SizedBlockAllocator);
  DELETE_MOVE_AND_ASSIGN(SizedBlockAllocator);

  // BlockAllocator interface.
  MemoryBlock Allocate() override;
  void Deallocate(int index) override;
  uint8_t* GetBlockMemory(int index) override;

  // Fields.
  uint8_t memory[kBlockSize * kBlockCount];
  // Allows for 64 blocks.
  // The bit set means the block is free.
  uint64_t block_bitset GUARDED_BY(mutex) = U64_ALL_ONES();
  int used_blocks = 0;

  std::mutex mutex;
};

// Implementations -------------------------------------------------------------

static constexpr uint64_t kAllOnes = 0xffff'ffff'ffff'fffflu;

template <uint64_t BlockSize>
SizedBlockAllocator<BlockSize>::SizedBlockAllocator()
    : block_bitset(kAllOnes), used_blocks(0) {}

template <uint64_t BlockSize>
MemoryBlock Allocate(SizedBlockAllocator<BlockSize>* allocator) {
  int free_block_index = 0;
  {
    std::lock_guard<std::mutex>(allocator->mutex);
    if (allocator->used_blocks == allocator->kBlockCount)
      return {};  // Invalid block;

    free_block_index = FindFirstSet(allocator->block_bitset);
    allocator->block_bitset &= ~(((uint64_t)1u) << (free_block_index - 1));
    allocator->used_blocks++;
  }

  /* static int alloc_count = 0; */
  /* alloc_count++; */
  /* LOG(DEBUG, "Alloc %d: Block bitset: 0x%zx", alloc_count, */
  /*                                             allocator->block_bitset); */

  ASSERT(free_block_index > 0);

  MemoryBlock block = {};
  block.allocator = allocator;
  block.index = free_block_index - 1;
  block.size = allocator->kBlockSize;

  return block;
}

template <uint64_t BlockSize>
MemoryBlock SizedBlockAllocator<BlockSize>::Allocate() {
  return ::rothko::Allocate(this);
}

template <uint64_t BlockSize>
void Deallocate(SizedBlockAllocator<BlockSize>* allocator, int index) {
  ASSERT(index >= 0 && index < 64);

  /* static int dealloc_count = 0; */
  /* dealloc_count++; */
  /* LOG(DEBUG, "Dealloc %d: Block bitset: 0x%zx", dealloc_count, */
  /*                                               allocator->block_bitset); */

  bool was_set = false;
  {
    std::lock_guard<std::mutex>(allocator->mutex);
    was_set = ((allocator->block_bitset & (1llu << index)) == 0);
    allocator->block_bitset |= (((uint64_t)1llu) << index);
    allocator->used_blocks--;
  }

  ASSERT(was_set);
}

template <uint64_t BlockSize>
void SizedBlockAllocator<BlockSize>::Deallocate(int index) {
  ::rothko::Deallocate(this, index);
}

template <uint64_t BlockSize>
uint8_t* SizedBlockAllocator<BlockSize>::GetBlockMemory(int index) {
  ASSERT(index >= 0 && index < 64);
  return memory + index * kBlockSize;
}

}  // namespace rothko
