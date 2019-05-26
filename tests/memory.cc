// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <third_party/catch2/catch.hpp>

#include "rothko/memory/block_allocator.h"

namespace rothko {
namespace test {

constexpr uint64_t BlockSize = 64u;

TEST_CASE("BlockAllocator") {
  BlockAllocator<BlockSize> allocator;

  {
    // Allocate 64 blocks.
    BlockAllocator<BlockSize>::MemoryBlock blocks[allocator.kBlockCount] = {};
    for (int i = 0; i < (int)allocator.kBlockCount; i++) {
      auto block = Allocate(&allocator);
      REQUIRE(allocator.used_blocks == i + 1);

      REQUIRE(block.valid());
      blocks[i] = std::move(block);

      REQUIRE(!block.valid());
      REQUIRE(blocks[i].valid());
    }

    // Allocating another should fail.
    {
      auto block = Allocate(&allocator);
      REQUIRE(!block.valid());
      REQUIRE(allocator.used_blocks == allocator.kBlockCount);
    }

    // Destroying and invalid block shouldn't have deallocated.
    REQUIRE(allocator.used_blocks == allocator.kBlockCount);

    // We free a couple of blocks.
    int deallocs = 0;
    int current_allocs = allocator.used_blocks;
    for (int i = 26; i < 34; i++) {
      REQUIRE(blocks[i].valid());
      blocks[i] = {};
      REQUIRE(!blocks[i].valid());

      deallocs++;
      REQUIRE(allocator.used_blocks == current_allocs - deallocs);
    }

    // We reallocate them.
    int allocs = 0;
    current_allocs = allocator.used_blocks;
    for (int i = 26; i < 34; i++) {
      auto block = Allocate(&allocator);

      allocs++;
      REQUIRE(block.valid());
      REQUIRE(allocator.used_blocks == current_allocs + allocs);

      REQUIRE(!blocks[i].valid());
      blocks[i] = std::move(block);
      REQUIRE(!block.valid());
      REQUIRE(blocks[i].valid());
      REQUIRE(allocator.used_blocks == current_allocs + allocs);
    }



    // Allocating another should fail.
    {
      auto block = Allocate(&allocator);
      REQUIRE(!block.valid());
      REQUIRE(allocator.used_blocks == allocator.kBlockCount);
    }

  }

  // All blocks went out of scope, so they've should've been deallocated.
  REQUIRE(allocator.used_blocks == 0);
}

}  // namespace test
}  // namespace rothko
