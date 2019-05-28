// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/containers/vector.h"

#include <third_party/catch2/catch.hpp>

#include "rothko/memory/block_allocator.h"

namespace rothko {
namespace test {

namespace {

struct TestAllocator : public BlockAllocator {
  static MemoryBlock SmallestBlock(uint32_t size);

  MemoryBlock Allocate() override;
  void Deallocate(int index) override;
  uint8_t* GetBlockMemory(int index) override;

  int new_block_count = 0;
  int dealloc_count = 0;
};

// Reset the counters.
void Reset(TestAllocator*);

TestAllocator* GetTestAllocator() {
  static TestAllocator allocator;
  return &allocator;
}

struct TestMemoryBlock {
  MemoryBlock block;
  std::unique_ptr<uint8_t[]> memory;
};

TestMemoryBlock CreateMemoryBlock(uint32_t size, int index) {
  TestMemoryBlock test_block;
  test_block.block.allocator = GetTestAllocator();
  test_block.block.index = index;
  test_block.block.size = size;
  test_block.memory = std::make_unique<uint8_t[]>(size);

  return test_block;
}

TestMemoryBlock gMemoryBlocks[] = {
  CreateMemoryBlock( 4 * sizeof(uint64_t) /*  32 */, 0),
  CreateMemoryBlock( 8 * sizeof(uint64_t) /*  64 */, 1),
  CreateMemoryBlock(16 * sizeof(uint64_t) /* 128 */, 2),
  CreateMemoryBlock(32 * sizeof(uint64_t) /* 256 */, 3),
  CreateMemoryBlock(64 * sizeof(uint64_t) /* 512 */, 4),
};

void Reset(TestMemoryBlock*);
MemoryBlock Clone(MemoryBlock*);

}  // namespace

#define VECTOR Vector<uint64_t, TestAllocator>

TEST_CASE("Vector") {
  TestAllocator* test_allocator = GetTestAllocator();

  {
    Reset(test_allocator);

    VECTOR vector;
    REQUIRE(test_allocator->new_block_count == 1);
    REQUIRE(vector.size == VECTOR::kDefaultSize);
    REQUIRE(vector.count == 0);
    REQUIRE(vector.memory_block == gMemoryBlocks[0].block);
  }
  // Vectors deallocate their block upon destruction.
  REQUIRE(test_allocator->dealloc_count == 1);

  {
    Reset(test_allocator);

    VECTOR vector(10);
    REQUIRE(test_allocator->new_block_count == 1);
    REQUIRE(vector.size == 16);
    REQUIRE(vector.count == 0);
    REQUIRE(vector.memory_block == gMemoryBlocks[2].block);
  }
  REQUIRE(test_allocator->dealloc_count == 1);

  {
    Reset(test_allocator);

    VECTOR vector;
    REQUIRE(test_allocator->new_block_count == 1);
    REQUIRE(vector.size == VECTOR::kDefaultSize);
    REQUIRE(vector.count == 0);
    REQUIRE(vector.memory_block == gMemoryBlocks[0].block);

    // Add 4 entries.
    vector.push_back(1);
    vector.push_back(2);
    vector.push_back(3);
    vector.push_back(4);

    REQUIRE(vector.size == VECTOR::kDefaultSize);
    REQUIRE(vector.count == 4);
    REQUIRE(vector.memory_block == gMemoryBlocks[0].block);

    REQUIRE(vector[0] == 1);
    REQUIRE(vector[1] == 2);
    REQUIRE(vector[2] == 3);
    REQUIRE(vector[3] == 4);

    // Verify directly at the memory block.
    uint64_t* ptr = (uint64_t*)gMemoryBlocks[0].memory.get();
    REQUIRE(ptr[0] == 1);
    REQUIRE(ptr[1] == 2);
    REQUIRE(ptr[2] == 3);
    REQUIRE(ptr[3] == 4);

    REQUIRE(test_allocator->new_block_count == 1);
  }
  REQUIRE(test_allocator->dealloc_count == 1);

  {
    Reset(test_allocator);

    VECTOR vector;
    REQUIRE(test_allocator->new_block_count == 1);
    REQUIRE(vector.size == VECTOR::kDefaultSize);
    REQUIRE(vector.count == 0);
    REQUIRE(vector.memory_block == gMemoryBlocks[0].block);

    // Add 4 entries.
    vector.push_back(1);
    vector.push_back(2);
    vector.push_back(3);
    vector.push_back(4);

    REQUIRE(vector.size == VECTOR::kDefaultSize);
    REQUIRE(vector.count == 4);
    REQUIRE(vector.memory_block == gMemoryBlocks[0].block);
    REQUIRE(test_allocator->new_block_count == 1);

    // Adding one more should've changed the block.
    vector.push_back(5);

    REQUIRE(vector.size == 8);
    REQUIRE(vector.count == 5);
    REQUIRE(vector.memory_block == gMemoryBlocks[1].block);

    // The change should've deallocated the previous one.
    REQUIRE(test_allocator->dealloc_count == 1);
    REQUIRE(test_allocator->new_block_count == 2);

    // Getting elements should work just fine.
    REQUIRE(vector[0] == 1);
    REQUIRE(vector[1] == 2);
    REQUIRE(vector[2] == 3);
    REQUIRE(vector[3] == 4);
    REQUIRE(vector[4] == 5);

    // From the pointer it should work too.
    uint64_t* ptr = (uint64_t*)gMemoryBlocks[1].memory.get();
    REQUIRE(ptr[0] == 1);
    REQUIRE(ptr[1] == 2);
    REQUIRE(ptr[2] == 3);
    REQUIRE(ptr[3] == 4);
    REQUIRE(ptr[4] == 5);

    // We add more elements.
    for (uint64_t i = 6; i < 41; i++) {
      vector.push_back(i);
    }


    // We should've crossed three blocks over.
    REQUIRE(test_allocator->dealloc_count == 4);
    REQUIRE(test_allocator->new_block_count == 5);

    REQUIRE(vector.size == 64);
    REQUIRE(vector.count == 40);
    REQUIRE(vector.memory_block == gMemoryBlocks[4].block);

    // All elements should be there.
    ptr = (uint64_t*)gMemoryBlocks[4].memory.get();
    for (uint32_t i = 0; i < 40; i++) {
      REQUIRE(vector.at(i) == i + 1);
      REQUIRE(ptr[i] == i + 1);
    }
  }

  Reset(test_allocator);
}

// TestAllocator Implementation ------------------------------------------------

namespace {

MemoryBlock TestAllocator::SmallestBlock(uint32_t size) {
  TestAllocator* test_allocator = GetTestAllocator();
  test_allocator->new_block_count++;

  for (TestMemoryBlock& test_block : gMemoryBlocks) {
    if (test_block.block.size >= size)
      return Clone(&test_block.block);
  }

  return {};
}

MemoryBlock TestAllocator::Allocate() {
  NOT_REACHED_MSG("This test doesn't use this!");
  exit(1);
  return {};
}

void TestAllocator::Deallocate(int index) {
  ASSERT(index >= 0 && index < ARRAY_SIZE(gMemoryBlocks));
  Reset(gMemoryBlocks + index);
  this->dealloc_count++;
}

uint8_t* TestAllocator::GetBlockMemory(int index) {
  ASSERT(index < ARRAY_SIZE(gMemoryBlocks));
  return gMemoryBlocks[index].memory.get();
}

void Reset(TestAllocator* allocator) {
  allocator->new_block_count = 0;
  allocator->dealloc_count = 0;

  for (TestMemoryBlock& test_block : gMemoryBlocks) {
    Reset(&test_block);
  }
}

void Reset(TestMemoryBlock* test_block) {
  std::memset(test_block->memory.get(), 0, test_block->block.size);
}

MemoryBlock Clone(MemoryBlock* block) {
  MemoryBlock new_block;
  new_block.allocator = block->allocator;
  new_block.size = block->size;
  new_block.index = block->index;

  return new_block;
}

#undef VECTOR

}  // namespace

}  // namesapce test
}  // namespace rothko
