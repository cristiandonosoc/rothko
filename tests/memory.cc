// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/memory/block_allocator.h"

#include <third_party/catch2/catch.hpp>

namespace rothko {
namespace test {
namespace {

constexpr uint64_t BlockSize = 64u;

TEST_CASE("BlockAllocator") {
  SizedBlockAllocator<BlockSize> allocator;

  {
    // Allocate 64 blocks.
    MemoryBlock blocks[allocator.kBlockCount] = {};
    for (int i = 0; i < (int)allocator.kBlockCount; i++) {
      auto block = Allocate(&allocator);
      REQUIRE(allocator.used_blocks == i + 1);

      REQUIRE(Valid(&block));
      blocks[i] = std::move(block);

      REQUIRE(!Valid(&block));
      REQUIRE(Valid(&blocks[i]));
    }

    // Allocating another should fail.
    {
      auto block = Allocate(&allocator);
      REQUIRE(!Valid(&block));
      REQUIRE(allocator.used_blocks == allocator.kBlockCount);
    }

    // Destroying and invalid block shouldn't have deallocated.
    REQUIRE(allocator.used_blocks == allocator.kBlockCount);

    // We free a couple of blocks.
    int deallocs = 0;
    int current_allocs = allocator.used_blocks;
    for (int i = 26; i < 34; i++) {
      REQUIRE(Valid(&blocks[i]));
      blocks[i] = {};
      REQUIRE(!Valid(&blocks[i]));

      deallocs++;
      REQUIRE(allocator.used_blocks == current_allocs - deallocs);
    }

    // We reallocate them.
    int allocs = 0;
    current_allocs = allocator.used_blocks;
    for (int i = 26; i < 34; i++) {
      auto block = Allocate(&allocator);

      allocs++;
      REQUIRE(Valid(&block));
      REQUIRE(allocator.used_blocks == current_allocs + allocs);

      REQUIRE(!Valid(&blocks[i]));
      blocks[i] = std::move(block);
      REQUIRE(!Valid(&block));
      REQUIRE(Valid(&blocks[i]));
      REQUIRE(allocator.used_blocks == current_allocs + allocs);
    }

    // Allocating another should fail.
    {
      auto block = Allocate(&allocator);
      REQUIRE(!Valid(&block));
      REQUIRE(allocator.used_blocks == allocator.kBlockCount);
    }

  }

  // All blocks went out of scope, so they've should've been deallocated.
  REQUIRE(allocator.used_blocks == 0);
}

}  // namespace
}  // namespace test
}  // namespace rothko
