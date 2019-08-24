// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <third_party/catch2/catch.hpp>

#include <rothko/graphics/commands.h>

namespace rothko {
namespace {

TEST_CASE("Primitive setup") {
  SECTION("LINES_LINE_WIDTH") {
    uint64_t ctx = 0xff0;
    (void)ctx;
    uint64_t new_ctx = SetLineWidth(ctx, 3);
    CHECK(new_ctx == 0xff3);
    CHECK(GetLineWidht(new_ctx) == 3);
  }
}

}  // namespace
}  // namespace rothko
