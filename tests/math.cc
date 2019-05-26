// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/math/vec.h>

#include <third_party/catch2/catch.hpp>

namespace rothko {

TEST_CASE("Vectors") {
  SECTION("v2") {
    Int2 a = {1, 2};
    Int2 b = {3, 4};

    {
      Int2 res = a + b;
      REQUIRE(res.x == 4);
      REQUIRE(res.y == 6);
    }

    {
      Int2 res = a;
      res += b;
      REQUIRE(res.x == 4);
      REQUIRE(res.y == 6);
    }

    {
      Int2 res = a - b;
      REQUIRE(res.x == -2);
      REQUIRE(res.y == -2);
    }

    {
      Int2 res = a;
      res -= b;
      REQUIRE(res.x == -2);
      REQUIRE(res.y == -2);
    }

    {
      int res = a * b;
      REQUIRE(res == (3 + 8));
    }
  }
}

TEST_CASE("Mat2") {
  SECTION("Matrix vector multiplication") {
    IntMat2 mat = {{1, 2}, {3, 4}};
    Int2 v = {1, 1};

    {
      Int2 res = mat * v;
      REQUIRE(res.x == 4);
      REQUIRE(res.y == 6);
    }

    mat = IntMat2::FromRows({1, 3}, {2, 4});
    {
      Int2 res = mat * v;
      REQUIRE(res.x == 4);
      REQUIRE(res.y == 6);
    }
  }
}

}  // namespace rothko
