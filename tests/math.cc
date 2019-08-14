// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/math/math.h>

#include <third_party/catch2/catch.hpp>

namespace rothko {

namespace {

TEST_CASE("Vec2")
{
  SECTION("ADDITION") {
    Vec2 v1{1, 2};
    Vec2 v2{3, 4};

    Vec2 res = v1 + v2;
    REQUIRE(res.x == 4);
    REQUIRE(res.y == 6);

    res += {2, 2};
    REQUIRE(res.x == 6);
    REQUIRE(res.y == 8);
  }

  SECTION("SUBSTRACTION") {
    Vec2 v1{1, 2};
    Vec2 v2{3, 4};

    Vec2 res = v1 - v2;
    REQUIRE(res.x == -2);
    REQUIRE(res.y == -2);

    res -= {2, 2};
    REQUIRE(res.x == -4);
    REQUIRE(res.y == -4);
  }

  SECTION("MULTIPLICATION") {
    Vec2 v1{1, 2};
    Vec2 v2{3, 4};

    Vec2 res = v1 * v2;
    REQUIRE(res.x == 3);
    REQUIRE(res.y == 8);

    res *= {2, 2};
    REQUIRE(res.x == 6);
    REQUIRE(res.y == 16);
  }

  SECTION("DIVIDE") {
    Vec2 v1{1, 2};
    Vec2 v2{3, 4};

    Vec2 res = v1 / v2;
    REQUIRE(res.x == 1.0f / 3.0f);
    REQUIRE(res.y == 2.0f / 4.0f);

    res /= {2, 2};
    REQUIRE(res.x == 1.0f / (3.0f * 2.0f));
    REQUIRE(res.y == 2.0f / (4.0f * 2.0f));
  }

  SECTION("MISC") {
    Vec2 v1{2, 2};

    REQUIRE(v1 == Vec2{2, 2});
    REQUIRE(IsZero(Vec2{0, 0}));
  }
}

TEST_CASE("Vec3") {
  SECTION("CROSS") {
    Vec3 a = {2, 3, 4};
    Vec3 b = {5, 6, 7};

    Vec3 c = Cross(a, b);
    REQUIRE(c.x == -3);
    REQUIRE(c.y == 6);
    REQUIRE(c.z == -3);
  }
}

TEST_CASE("Mat4") {
  Mat4 mat = {{ 1,  2,  3,  4},
              { 5,  6,  7,  8},
              { 9, 10, 11, 12},
              {13, 14, 15, 16}};

  SECTION("Storage") {
    // The API treats it as row-major, but they're stored column-major.
    REQUIRE(mat.cols[0] == Vec4{ 1,  2,  3,  4});
    REQUIRE(mat.cols[1] == Vec4{ 5,  6,  7,  8});
    REQUIRE(mat.cols[2] == Vec4{ 9, 10, 11, 12});
    REQUIRE(mat.cols[3] == Vec4{13, 14, 15, 16});
  }

  SECTION("GetRow") {
    REQUIRE(GetRow(mat, 0) == Vec4{ 1,  2,  3,  4});
    REQUIRE(GetRow(mat, 1) == Vec4{ 5,  6,  7,  8});
    REQUIRE(GetRow(mat, 2) == Vec4{ 9, 10, 11, 12});
    REQUIRE(GetRow(mat, 3) == Vec4{13, 14, 15, 16});
  }

  SECTION("GetCol") {
    REQUIRE(GetCol(mat, 0) == Vec4{ 1,  5,  9, 13});
    REQUIRE(GetCol(mat, 1) == Vec4{ 2,  6, 10, 14});
    REQUIRE(GetCol(mat, 2) == Vec4{ 3,  7, 11, 15});
    REQUIRE(GetCol(mat, 3) == Vec4{ 4,  8, 12, 16});
  }

  SECTION("V3 multiplication") {
    Vec3 v = Vec3{2, 3, 4};
    Vec4 res = mat * v;
    REQUIRE(res[0] == 24);    // 1*2 + 2*3 + 3*4 + 4*1
    REQUIRE(res[1] == 64);    // 5*2 + 6*3 + 7*4 + 8*1
    REQUIRE(res[2] == 104);   // 9*2 + 10*3 + 11*4 + 12*1
    REQUIRE(res[3] == 144);   // 13*2 + 14*3 + 15*4 + 16*1
  }

  SECTION("v4 multiplication") {
    Vec4 v = Vec4{1, 2, 3, 4};
    Vec4 res = mat * v;
    REQUIRE(res[0] == 30);    // 1*1 + 2*2 + 3*3 + 4*4
    REQUIRE(res[1] == 70);    // 5*1 + 6*2 + 7*3 + 8*4
    REQUIRE(res[2] == 110);   // 9*1 + 10*2 + 11*3 + 12*4
    REQUIRE(res[3] == 150);   // 13*1 + 14*2 + 15*3 + 16*4
  }

  SECTION("mat4 multiplication") {
    Mat4 mat2 = {{ 1,  2,  3,  4},
                 { 5,  6,  7,  8},
                 { 9, 10, 11, 12},
                 {13, 14, 15, 16}};

    Mat4 res = mat * mat2;

    REQUIRE(GetRow(res, 0) == Vec4{ 90, 100, 110, 120});
    REQUIRE(GetRow(res, 1) == Vec4{202, 228, 254, 280});
    REQUIRE(GetRow(res, 2) == Vec4{314, 356, 398, 440});
    REQUIRE(GetRow(res, 3) == Vec4{426, 484, 542, 600});
  }
}

}  // namespace

}  // namespace rothko
