// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/containers/vector.h"

#include <third_party/catch2/catch.hpp>

#include "rothko/memory/block_allocator.h"

namespace rothko {
namespace test {

namespace {

struct TestMemoryBlock {
  TestMemoryBlock(BlockAllocator* allocator, uint32_t size, int index)
      : memory(std::make_unique<uint8_t[]>(size)),
      block(allocator, index, size) {}

  std::unique_ptr<uint8_t[]> memory;
  MemoryBlock block;
};

struct TestAllocator : public BlockAllocator {
  static MemoryBlock SmallestBlock(uint32_t size);

  MemoryBlock Allocate() override;
  void Deallocate(int index) override;
  uint8_t* GetBlockMemory(int index) override;

  TestMemoryBlock gMemoryBlocks[5] = {
    TestMemoryBlock(this,  4 * sizeof(uint64_t) /*  32 */, 0),
    TestMemoryBlock(this,  8 * sizeof(uint64_t) /*  64 */, 1),
    TestMemoryBlock(this, 16 * sizeof(uint64_t) /* 128 */, 2),
    TestMemoryBlock(this, 32 * sizeof(uint64_t) /* 256 */, 3),
    TestMemoryBlock(this, 64 * sizeof(uint64_t) /* 512 */, 4),
  };

  int new_block_count = 0;
  int dealloc_count = 0;
  int total_new = 0;
  int total_dealloc = 0;
};

// Reset the counters.
void Reset(TestAllocator*);

TestAllocator* GetTestAllocator() {
  static TestAllocator allocator;
  return &allocator;
}

void Reset(TestMemoryBlock*);
MemoryBlock Clone(MemoryBlock*);

}  // namespace

#define VECTOR Vector<uint64_t, TestAllocator>

TEST_CASE("Vector") {
  TestAllocator* test_allocator = GetTestAllocator();
  for (auto& block : test_allocator->gMemoryBlocks) {
    ASSERT(block.memory);
  }

  // Default case.
  {
    Reset(test_allocator);

    VECTOR vector;
    REQUIRE(test_allocator->new_block_count == 1);
    REQUIRE(vector.size == VECTOR::kDefaultSize);
    REQUIRE(vector.count == 0);
    REQUIRE(vector.memory_block == test_allocator->gMemoryBlocks[0].block);
  }
  // Vectors deallocate their block upon destruction.
  REQUIRE(test_allocator->dealloc_count == 1);

  // Big initial allocation.
  {
    Reset(test_allocator);

    VECTOR vector(10);
    REQUIRE(test_allocator->new_block_count == 1);
    REQUIRE(vector.size == 16);
    REQUIRE(vector.count == 0);
    REQUIRE(vector.memory_block == test_allocator->gMemoryBlocks[2].block);
  }
  REQUIRE(test_allocator->dealloc_count == 1);

  // Push back into the vector.
  {
    Reset(test_allocator);

    VECTOR vector;
    REQUIRE(test_allocator->new_block_count == 1);
    REQUIRE(vector.size == VECTOR::kDefaultSize);
    REQUIRE(vector.count == 0);
    REQUIRE(vector.memory_block == test_allocator->gMemoryBlocks[0].block);

    // Add 4 entries.
    vector.push_back(1);
    vector.push_back(2);
    vector.push_back(3);
    vector.push_back(4);

    REQUIRE(vector.size == VECTOR::kDefaultSize);
    REQUIRE(vector.count == 4);
    REQUIRE(vector.memory_block == test_allocator->gMemoryBlocks[0].block);

    REQUIRE(vector[0] == 1);
    REQUIRE(vector[1] == 2);
    REQUIRE(vector[2] == 3);
    REQUIRE(vector[3] == 4);

    // Verify directly at the memory block.
    uint64_t* ptr = (uint64_t*)test_allocator->gMemoryBlocks[0].memory.get();
    REQUIRE(ptr[0] == 1);
    REQUIRE(ptr[1] == 2);
    REQUIRE(ptr[2] == 3);
    REQUIRE(ptr[3] == 4);

    REQUIRE(test_allocator->new_block_count == 1);
  }
  REQUIRE(test_allocator->dealloc_count == 1);

  // Grow the vector many times.
  {
    Reset(test_allocator);

    VECTOR vector;
    REQUIRE(test_allocator->new_block_count == 1);
    REQUIRE(vector.size == VECTOR::kDefaultSize);
    REQUIRE(vector.count == 0);
    REQUIRE(vector.memory_block == test_allocator->gMemoryBlocks[0].block);

    // Add 4 entries.
    vector.push_back(1);
    vector.push_back(2);
    vector.push_back(3);
    vector.push_back(4);

    REQUIRE(vector.size == VECTOR::kDefaultSize);
    REQUIRE(vector.count == 4);
    REQUIRE(vector.memory_block == test_allocator->gMemoryBlocks[0].block);
    REQUIRE(test_allocator->new_block_count == 1);

    // Adding one more should've changed the block.
    vector.push_back(5);

    REQUIRE(vector.size == 8);
    REQUIRE(vector.count == 5);
    REQUIRE(vector.memory_block == test_allocator->gMemoryBlocks[1].block);

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
    uint64_t* ptr = (uint64_t*)test_allocator->gMemoryBlocks[1].memory.get();
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
    REQUIRE(vector.memory_block == test_allocator->gMemoryBlocks[4].block);

    // All elements should be there.
    ptr = (uint64_t*)test_allocator->gMemoryBlocks[4].memory.get();
    for (uint32_t i = 0; i < 40; i++) {
      REQUIRE(vector.at(i) == i + 1);
      REQUIRE(ptr[i] == i + 1);
    }
  }

  // Modify elements.
  {
    Reset(test_allocator);

    VECTOR vector;

    // Add 4 entries.
    vector.push_back(1);
    vector.push_back(2);
    vector.push_back(3);
    vector.push_back(4);

    // Verify directly at the memory block.
    REQUIRE(vector.memory_block == test_allocator->gMemoryBlocks[0].block);
    uint64_t* ptr = (uint64_t*)test_allocator->gMemoryBlocks[0].memory.get();
    REQUIRE(ptr[0] == 1);
    REQUIRE(ptr[1] == 2);
    REQUIRE(ptr[2] == 3);
    REQUIRE(ptr[3] == 4);

    // Modify some elements.
    vector[1] = 18;
    vector[3] = 7;

    // Verify the change.
    REQUIRE(vector.memory_block == test_allocator->gMemoryBlocks[0].block);
    REQUIRE(ptr[0] == 1);
    REQUIRE(ptr[1] == 18);
    REQUIRE(ptr[2] == 3);
    REQUIRE(ptr[3] == 7);
  }

  // Iterators.
  {
    Reset(test_allocator);

    VECTOR vector(10);
    REQUIRE(test_allocator->new_block_count == 1);
    REQUIRE(test_allocator->dealloc_count == 0);
    for (int i = 0; i < 10; i++) {
      vector.push_back(i);
    }
    REQUIRE(test_allocator->new_block_count == 1);
    REQUIRE(test_allocator->dealloc_count == 0);

    // Verify iterator.
    int i = 0;
    for (auto& elem : vector) {
      REQUIRE(elem == i);
      i++;
    }

    // Other kind of iterator.
    i = 0;
    for (auto it = vector.begin(); it != vector.end(); it++, i++) {
      REQUIRE(*it == i);
    }

    // Modification.
    auto it = vector.begin();
    it += 3;
    *it = 22;
    it += 3;
    *it = 44;
    it += 3;
    *it = 66;

    REQUIRE(vector[0] == 0);
    REQUIRE(vector[1] == 1);
    REQUIRE(vector[2] == 2);
    REQUIRE(vector[3] == 22);
    REQUIRE(vector[4] == 4);
    REQUIRE(vector[5] == 5);
    REQUIRE(vector[6] == 44);
    REQUIRE(vector[7] == 7);
    REQUIRE(vector[8] == 8);
    REQUIRE(vector[9] == 66);
  }

  Reset(test_allocator);
  // Everything allocated should be freed after all vectors are gone.
  REQUIRE(test_allocator->total_new == test_allocator->total_dealloc);

  for (auto& block : test_allocator->gMemoryBlocks) {
    ASSERT(block.memory);
  }
}

// TestAllocator Implementation ------------------------------------------------

namespace {

MemoryBlock TestAllocator::SmallestBlock(uint32_t size) {
  TestAllocator* test_allocator = GetTestAllocator();
  test_allocator->new_block_count++;
  test_allocator->total_new++;

  for (TestMemoryBlock& test_block : test_allocator->gMemoryBlocks) {
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
  this->total_dealloc++;
}

uint8_t* TestAllocator::GetBlockMemory(int index) {
  ASSERT(index < ARRAY_SIZE(gMemoryBlocks));
  return gMemoryBlocks[index].memory.get();
}

void Reset(TestAllocator* test_allocator) {
  test_allocator->new_block_count = 0;
  test_allocator->dealloc_count = 0;

  for (TestMemoryBlock& test_block : test_allocator->gMemoryBlocks) {
    Reset(&test_block);
  }
}

void Reset(TestMemoryBlock* test_block) {
  ASSERT(test_block->memory);
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
