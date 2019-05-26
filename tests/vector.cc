// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/containers/vector.h"

#include <third_party/catch2/catch.hpp>

namespace rothko {
namespace test {

struct TestAllocator {

  static MemoryBlock SmallestBlock(int size);

};

TEST_CASE("Vector") {


}

}  // namesapce test
}  // namespace rothko
