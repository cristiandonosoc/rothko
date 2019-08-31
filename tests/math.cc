// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/math/math.h>

#include <third_party/catch2/catch.hpp>

namespace rothko {
namespace test {
namespace {

TEST_CASE("Bits") {
  SECTION("Get bits") {
    uint8_t bits = 0b01010101;

    CHECK(GetBit(bits, 0) == 1);
    CHECK(GetBit(bits, 1) == 0);
    CHECK(GetBit(bits, 2) == 1);
    CHECK(GetBit(bits, 3) == 0);
    CHECK(GetBit(bits, 4) == 1);
    CHECK(GetBit(bits, 5) == 0);
    CHECK(GetBit(bits, 6) == 1);
    CHECK(GetBit(bits, 7) == 0);
  }

  SECTION("Set bits") {
    CHECK(SetBit(0, 0) == 0b00000001);
    CHECK(SetBit(0, 1) == 0b00000010);
    CHECK(SetBit(0, 2) == 0b00000100);
    CHECK(SetBit(0, 3) == 0b00001000);
    CHECK(SetBit(0, 4) == 0b00010000);
    CHECK(SetBit(0, 5) == 0b00100000);
    CHECK(SetBit(0, 6) == 0b01000000);
    CHECK(SetBit(0, 7) == 0b10000000);

    uint8_t bits = 0b01010101;

    CHECK(SetBit(bits, 0) == 0b01010101);
    CHECK(SetBit(bits, 1) == 0b01010111);
    CHECK(SetBit(bits, 2) == 0b01010101);
    CHECK(SetBit(bits, 3) == 0b01011101);
    CHECK(SetBit(bits, 4) == 0b01010101);
    CHECK(SetBit(bits, 5) == 0b01110101);
    CHECK(SetBit(bits, 6) == 0b01010101);
    CHECK(SetBit(bits, 7) == 0b11010101);
  }

  SECTION("Clear bits") {
    CHECK(ClearBit(0xff, 0) == 0b11111110);
    CHECK(ClearBit(0xff, 1) == 0b11111101);
    CHECK(ClearBit(0xff, 2) == 0b11111011);
    CHECK(ClearBit(0xff, 3) == 0b11110111);
    CHECK(ClearBit(0xff, 4) == 0b11101111);
    CHECK(ClearBit(0xff, 5) == 0b11011111);
    CHECK(ClearBit(0xff, 6) == 0b10111111);
    CHECK(ClearBit(0xff, 7) == 0b01111111);

    uint8_t bits = 0b01010101;

    CHECK(ClearBit(bits, 0) == 0b01010100);
    CHECK(ClearBit(bits, 1) == 0b01010101);
    CHECK(ClearBit(bits, 2) == 0b01010001);
    CHECK(ClearBit(bits, 3) == 0b01010101);
    CHECK(ClearBit(bits, 4) == 0b01000101);
    CHECK(ClearBit(bits, 5) == 0b01010101);
    CHECK(ClearBit(bits, 6) == 0b00010101);
    CHECK(ClearBit(bits, 7) == 0b01010101);
  }
}

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
  /* printf("%s\n", ToString(mat).c_str()); */

  SECTION("Storage") {
    // The API treats it as row-major, but they're stored column-major.
    REQUIRE(mat.cols[0] == Vec4{ 1,  5,  9, 13});
    REQUIRE(mat.cols[1] == Vec4{ 2,  6, 10, 14});
    REQUIRE(mat.cols[2] == Vec4{ 3,  7, 11, 15});
    REQUIRE(mat.cols[3] == Vec4{ 4,  8, 12, 16});
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
    Mat4 mat2 = {{ 2,  3,  4,  5},
                 { 6,  7,  8,  9},
                 {10, 11, 12, 13},
                 {14, 15, 16, 17}};
    Mat4 mat3 = {{ 3,  4,  5,  6},
                 { 7,  8,  9, 10},
                 {11, 12, 13, 14},
                 {15, 16, 17, 18}};

    Mat4 res1_2 = mat * mat2;
    REQUIRE(GetRow(res1_2, 0) == Vec4{100, 110, 120, 130});
    REQUIRE(GetRow(res1_2, 1) == Vec4{228, 254, 280, 306});
    REQUIRE(GetRow(res1_2, 2) == Vec4{356, 398, 440, 482});
    REQUIRE(GetRow(res1_2, 3) == Vec4{484, 542, 600, 658});

    // A * B * C = (A * B) * C = A * (B * C);

    Mat4 res12_3 = res1_2 * mat3;
    REQUIRE(GetRow(res12_3, 0) == Vec4{ 4340,  4800,  5260,  5720});
    REQUIRE(GetRow(res12_3, 1) == Vec4{10132, 11200, 12268, 13336});
    REQUIRE(GetRow(res12_3, 2) == Vec4{15924, 17600, 19276, 20952});
    REQUIRE(GetRow(res12_3, 3) == Vec4{21716, 24000, 26284, 28568});

    Mat4 res123 = mat * mat2 * mat3;
    REQUIRE(GetRow(res123, 0) == Vec4{ 4340,  4800,  5260,  5720});
    REQUIRE(GetRow(res123, 1) == Vec4{10132, 11200, 12268, 13336});
    REQUIRE(GetRow(res123, 2) == Vec4{15924, 17600, 19276, 20952});
    REQUIRE(GetRow(res123, 3) == Vec4{21716, 24000, 26284, 28568});
  }
}

}  // namespace
}  // namespace test
}  // namespace rothko
