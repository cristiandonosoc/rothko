// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <third_party/catch2/catch.hpp>

#include <rothko/graphics/commands.h>

namespace rothko {
namespace test {
namespace {

TEST_CASE("Lines") {
  SECTION("Line Width") {
    uint64_t ctx = U64_ALL_ONES();
    uint64_t new_ctx = lines::SetLineWidth(ctx, 3);
    CHECK(new_ctx == 0xffff'ffff'ffff'fffb);
    CHECK(lines::GetLineWidth(new_ctx) == 3);
  }
}

TEST_CASE("Line Strip") {
  SECTION("Restart Index") {
    uint64_t ctx = (uint64_t)-1;
    REQUIRE(line_strip::GetRestartIndex(ctx) == (uint32_t)-1);

    uint64_t new_ctx = line_strip::SetRestartIndex(ctx, 0xf0ff0);
    REQUIRE(new_ctx == 0xffff'ffff'000f'0ff0);
    REQUIRE(line_strip::GetRestartIndex(new_ctx) == 0xf0ff0);
  }
}

}  // namespace
}  // namespace test
}  // namespace rothko
