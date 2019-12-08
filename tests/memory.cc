// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/memory/block_allocator.h"
#include "rothko/memory/stack_allocator.h"

#include <third_party/catch2/catch.hpp>

namespace rothko {
namespace test {
namespace {

TEST_CASE("StackAllocator") {
  constexpr uint32_t kElemCount = 8;
  StackAllocator sa = CreateStackAllocatorFor<uint32_t>(kElemCount);
  REQUIRE(sa.size == sizeof(uint32_t) * kElemCount);

  uint32_t* ptr = (uint32_t*)sa.data_.get();
  for (uint32_t i = 0; i < kElemCount; i++) {
    *ptr++ = i + 1;
  }

  uint32_t* array = Allocate<uint32_t>(&sa, 5);
  REQUIRE(sa.current == sizeof(uint32_t) * 5);
  CHECK(array[0] == 1);
  CHECK(array[1] == 2);
  CHECK(array[2] == 3);
  CHECK(array[3] == 4);
  CHECK(array[4] == 5);

  CHECK(sa.current == sizeof(uint32_t) * 5);
  CHECK(Allocate<uint32_t>(&sa, 10) == nullptr);

  CHECK(*Allocate<uint32_t>(&sa) == 6);
  REQUIRE(sa.current == sizeof(uint32_t) * 6);
  CHECK(*Allocate<uint32_t>(&sa) == 7);
  REQUIRE(sa.current == sizeof(uint32_t) * 7);
  CHECK(*Allocate<uint32_t>(&sa) == 8);
  REQUIRE(sa.current == sizeof(uint32_t) * 8);
  CHECK(Allocate<uint32_t>(&sa) == nullptr);
  CHECK(Allocate<uint32_t>(&sa) == nullptr);
  CHECK(Allocate<uint32_t>(&sa) == nullptr);
  CHECK(Allocate<uint32_t>(&sa) == nullptr);

  Reset(&sa);
  CHECK(sa.current == 0);
}

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
