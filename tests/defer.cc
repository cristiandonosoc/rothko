// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/utils/defer.h"

#include <third_party/catch2/catch.hpp>

namespace rothko {
namespace test {

TEST_CASE("Defer") {
  bool called = false;

  {
    auto defer = Defer([&called]() { called = true; });
  }

  REQUIRE(called);
}

TEST_CASE("Move defer") {
  int call_count = 0;

  {
    auto defer1 = Defer([&call_count]() { call_count++; });
    auto defer2 = std::move(defer1);
    auto defer3 = std::move(defer2);
    auto defer4 = std::move(defer3);
  }

  REQUIRE(call_count == 1);
}

}  // namespace test
}  // namespace rothko
