// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/math/math.h>

#include <third_party/catch2/catch.hpp>

namespace Catch {

template <>
struct StringMaker<rothko::Vec4> {
  static std::string convert(const rothko::Vec4& v) {
    return ToString(v);
  }
};

#define DIFF(lhs, rhs) std::abs(rhs - lhs)

#define CHECK_ROW(row, v0, v1, v2, v3) \
  CHECK(DIFF((row)[0], v0) < 0.00001f);  \
  CHECK(DIFF((row)[1], v1) < 0.00001f);  \
  CHECK(DIFF((row)[2], v2) < 0.00001f);  \
  CHECK(DIFF((row)[3], v3) < 0.00001f);

}  // namespace Catch


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

  // clang-format off
  SECTION("Set bits") {
    int test = 0;

    test = 0; SetBit(&test, 0); CHECK(test == 0b00000001);
    test = 0; SetBit(&test, 1); CHECK(test == 0b00000010);
    test = 0; SetBit(&test, 2); CHECK(test == 0b00000100);
    test = 0; SetBit(&test, 3); CHECK(test == 0b00001000);
    test = 0; SetBit(&test, 4); CHECK(test == 0b00010000);
    test = 0; SetBit(&test, 5); CHECK(test == 0b00100000);
    test = 0; SetBit(&test, 6); CHECK(test == 0b01000000);
    test = 0; SetBit(&test, 7); CHECK(test == 0b10000000);

    test = 0b01010101; SetBit(&test, 0); CHECK(test == 0b01010101);
    test = 0b01010101; SetBit(&test, 1); CHECK(test == 0b01010111);
    test = 0b01010101; SetBit(&test, 2); CHECK(test == 0b01010101);
    test = 0b01010101; SetBit(&test, 3); CHECK(test == 0b01011101);
    test = 0b01010101; SetBit(&test, 4); CHECK(test == 0b01010101);
    test = 0b01010101; SetBit(&test, 5); CHECK(test == 0b01110101);
    test = 0b01010101; SetBit(&test, 6); CHECK(test == 0b01010101);
    test = 0b01010101; SetBit(&test, 7); CHECK(test == 0b11010101);
  }

  SECTION("Clear bits") {
    int test = 0;

    test = 0xff; ClearBit(&test, 0); CHECK(test == 0b11111110);
    test = 0xff; ClearBit(&test, 1); CHECK(test == 0b11111101);
    test = 0xff; ClearBit(&test, 2); CHECK(test == 0b11111011);
    test = 0xff; ClearBit(&test, 3); CHECK(test == 0b11110111);
    test = 0xff; ClearBit(&test, 4); CHECK(test == 0b11101111);
    test = 0xff; ClearBit(&test, 5); CHECK(test == 0b11011111);
    test = 0xff; ClearBit(&test, 6); CHECK(test == 0b10111111);
    test = 0xff; ClearBit(&test, 7); CHECK(test == 0b01111111);

    test = 0b01010101; ClearBit(&test, 0); CHECK(test == 0b01010100);
    test = 0b01010101; ClearBit(&test, 1); CHECK(test == 0b01010101);
    test = 0b01010101; ClearBit(&test, 2); CHECK(test == 0b01010001);
    test = 0b01010101; ClearBit(&test, 3); CHECK(test == 0b01010101);
    test = 0b01010101; ClearBit(&test, 4); CHECK(test == 0b01000101);
    test = 0b01010101; ClearBit(&test, 5); CHECK(test == 0b01010101);
    test = 0b01010101; ClearBit(&test, 6); CHECK(test == 0b00010101);
    test = 0b01010101; ClearBit(&test, 7); CHECK(test == 0b01010101);
  }
}

TEST_CASE("Mask") {
  SECTION("Get") {
    uint32_t bits = 0x12348a0f;

    CHECK(GetMask(bits,  0u, 0xfu) == 0xf);
    CHECK(GetMask(bits,  0u, 0b11u) == 0b11);
    CHECK(GetMask(bits,  4u, 0xfu) == 0x0);
    CHECK(GetMask(bits,  8u, 0xfu) == 0xa);
    CHECK(GetMask(bits, 12u, 0xfu) == 0x8);
    CHECK(GetMask(bits, 16u, 0xfu) == 0x4);
    CHECK(GetMask(bits, 20u, 0xfu) == 0x3);
    CHECK(GetMask(bits, 24u, 0xfu) == 0x2);
    CHECK(GetMask(bits, 28u, 0xfu) == 0x1);
  }

  SECTION("Set") {
    uint32_t test = 0;
    test = 0; SetMask(&test,  0u, 0xfu, 0xfu); CHECK(test == 0x0000000f);
    test = 0; SetMask(&test,  4u, 0xfu, 0xfu); CHECK(test == 0x000000f0);
    test = 0; SetMask(&test,  8u, 0xfu, 0xfu); CHECK(test == 0x00000f00);
    test = 0; SetMask(&test, 12u, 0xfu, 0xfu); CHECK(test == 0x0000f000);
    test = 0; SetMask(&test, 16u, 0xfu, 0xfu); CHECK(test == 0x000f0000);
    test = 0; SetMask(&test, 20u, 0xfu, 0xfu); CHECK(test == 0x00f00000);
    test = 0; SetMask(&test, 24u, 0xfu, 0xfu); CHECK(test == 0x0f000000);
    test = 0; SetMask(&test, 28u, 0xfu, 0xfu); CHECK(test == 0xf0000000);
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

TEST_CASE("Mat3") {
  //clang-format off
  Mat3 mat = {{  6,  1,  1},
              {  4, -2,  5},
              {  2,  8,  7}};
  // clang-format on

  SECTION("Determinant") {
    float d = Determinant(mat);
    REQUIRE(d == -306);
  }
}

// clang-format off
TEST_CASE("Mat4") {
  SECTION("Storage") {
    Mat4 mat = {{ 1,  2,  3,  4},
                { 5,  6,  7,  8},
                { 9, 10, 11, 12},
                {13, 14, 15, 16}};

    // The API treats it as row-major, but they're stored column-major.
    CHECK(mat.cols[0] == Vec4{ 1,  5,  9, 13});
    CHECK(mat.cols[1] == Vec4{ 2,  6, 10, 14});
    CHECK(mat.cols[2] == Vec4{ 3,  7, 11, 15});
    CHECK(mat.cols[3] == Vec4{ 4,  8, 12, 16});
  }

  SECTION("V3 multiplication") {
    Mat4 mat = {{ 1,  2,  3,  4},
                { 5,  6,  7,  8},
                { 9, 10, 11, 12},
                {13, 14, 15, 16}};

    Vec3 v = Vec3{2, 3, 4};
    Vec4 res = mat * v;
    CHECK(res[0] == 24);    // 1*2 + 2*3 + 3*4 + 4*1
    CHECK(res[1] == 64);    // 5*2 + 6*3 + 7*4 + 8*1
    CHECK(res[2] == 104);   // 9*2 + 10*3 + 11*4 + 12*1
    CHECK(res[3] == 144);   // 13*2 + 14*3 + 15*4 + 16*1
  }

  SECTION("v4 multiplication") {
    Mat4 mat = {{ 1,  2,  3,  4},
                { 5,  6,  7,  8},
                { 9, 10, 11, 12},
                {13, 14, 15, 16}};

    Vec4 v = Vec4{1, 2, 3, 4};
    Vec4 res = mat * v;
    CHECK(res[0] == 30);    // 1*1 + 2*2 + 3*3 + 4*4
    CHECK(res[1] == 70);    // 5*1 + 6*2 + 7*3 + 8*4
    CHECK(res[2] == 110);   // 9*1 + 10*2 + 11*3 + 12*4
    CHECK(res[3] == 150);   // 13*1 + 14*2 + 15*3 + 16*4
  }

  SECTION("mat4 multiplication") {
    Mat4 mat  = {{ 1,  2,  3,  4},
                 { 5,  6,  7,  8},
                 { 9, 10, 11, 12},
                 {13, 14, 15, 16}};
    Mat4 mat2 = {{ 2,  3,  4,  5},
                 { 6,  7,  8,  9},
                 {10, 11, 12, 13},
                 {14, 15, 16, 17}};
    Mat4 mat3 = {{ 3,  4,  5,  6},
                 { 7,  8,  9, 10},
                 {11, 12, 13, 14},
                 {15, 16, 17, 18}};

    Mat4 res1_2 = mat * mat2;
    CHECK(res1_2.row(0) == Vec4{100, 110, 120, 130});
    CHECK(res1_2.row(1) == Vec4{228, 254, 280, 306});
    CHECK(res1_2.row(2) == Vec4{356, 398, 440, 482});
    CHECK(res1_2.row(3) == Vec4{484, 542, 600, 658});

    // A * B * C = (A * B) * C = A * (B * C);

    Mat4 res12_3 = res1_2 * mat3;
    CHECK(res12_3.row(0) == Vec4{ 4340,  4800,  5260,  5720});
    CHECK(res12_3.row(1) == Vec4{10132, 11200, 12268, 13336});
    CHECK(res12_3.row(2) == Vec4{15924, 17600, 19276, 20952});
    CHECK(res12_3.row(3) == Vec4{21716, 24000, 26284, 28568});

    Mat4 res123 = mat * mat2 * mat3;
    CHECK(res123.row(0) == Vec4{ 4340,  4800,  5260,  5720});
    CHECK(res123.row(1) == Vec4{10132, 11200, 12268, 13336});
    CHECK(res123.row(2) == Vec4{15924, 17600, 19276, 20952});
    CHECK(res123.row(3) == Vec4{21716, 24000, 26284, 28568});
  }

  SECTION("Float multiplication") {
    Mat4 mat = {{ 1,  2,  3,  4},
                { 5,  6,  7,  8},
                { 9, 10, 11, 12},
                {13, 14, 15, 16}};

    Mat4 m = mat * 2;
    CHECK(m.row(0) == Vec4{ 2,  4,  6,  8});
    CHECK(m.row(1) == Vec4{10, 12, 14, 16});
    CHECK(m.row(2) == Vec4{18, 20, 22, 24});
    CHECK(m.row(3) == Vec4{26, 28, 30, 32});

    m *= 2;
    CHECK(m.row(0) == Vec4{ 4,  8, 12, 16});
    CHECK(m.row(1) == Vec4{20, 24, 28, 32});
    CHECK(m.row(2) == Vec4{36, 40, 44, 48});
    CHECK(m.row(3) == Vec4{52, 56, 60, 64});
  }

  SECTION("Determinant") {
    Mat4 m{{1, 3, 5, 9},
           {1, 3, 1, 7},
           {4, 3, 9, 7},
           {5, 2, 0, 9}};
    float det = Determinant(m);
    REQUIRE(det == -376);
  }

  SECTION("Adjugate") {
    Mat4 m{{  1,  1,  1, -1},
           {  1,  1, -1,  1},
           {  1, -1,  1,  1},
           { -1,  1,  1,  1}};

    Mat4 adjugate = Adjugate(m);
    CHECK(adjugate.row(0) == Vec4{ -4, -4, -4,  4});
    CHECK(adjugate.row(1) == Vec4{ -4, -4,  4, -4});
    CHECK(adjugate.row(2) == Vec4{ -4,  4, -4, -4});
    CHECK(adjugate.row(3) == Vec4{  4, -4, -4, -4});
  }

  SECTION("Adjugate2") {
    Mat4 m{{ 2, 0, 0, 3},
           { 0, 4, 0, 5},
           { 0, 0, 6, 7},
           { 0, 0, 0, 1}};

    Mat4 adjugate = Adjugate(m);
    CHECK(adjugate.row(0) == Vec4{ 24,  0,  0, -72});
    CHECK(adjugate.row(1) == Vec4{  0, 12,  0, -60});
    CHECK(adjugate.row(2) == Vec4{  0,  0,  8, -56});
    CHECK(adjugate.row(3) == Vec4{  0,  0,  0,  48});
  }

  SECTION("Inverse") {
    Mat4 m{{  1,  1,  1, -1},
           {  1,  1, -1,  1},
           {  1, -1,  1,  1},
           { -1,  1,  1,  1}};

    float q = 0.25f;
    Mat4 inverse = Inverse(m);
    CHECK(inverse.row(0) == Vec4{  q,  q,  q, -q});
    CHECK(inverse.row(1) == Vec4{  q,  q, -q,  q});
    CHECK(inverse.row(2) == Vec4{  q, -q,  q,  q});
    CHECK(inverse.row(3) == Vec4{ -q,  q,  q,  q});
  }

  SECTION("Inverse2") {
    Mat4 m{{ 2, 0, 0, 3},
           { 0, 4, 0, 5},
           { 0, 0, 6, 7},
           { 0, 0, 0, 1}};

    Mat4 inverse = Inverse(m);
    CHECK_ROW(inverse.row(0), 0.5f,     0,         0, -3.0f/2.0f);
    CHECK_ROW(inverse.row(1),    0, 0.25f,         0, -5.0f/4.0f);
    CHECK_ROW(inverse.row(2),    0,     0, 1.0f/6.0f, -7.0f/6.0f);
    CHECK_ROW(inverse.row(3),    0,     0,         0,          1);

    Mat4 identity = m * inverse;
    CHECK_ROW(identity.row(0), 1, 0, 0, 0);
    CHECK_ROW(identity.row(1), 0, 1, 0, 0);
    CHECK_ROW(identity.row(2), 0, 0, 1, 0);
    CHECK_ROW(identity.row(3), 0, 0, 0, 1);
  }

  SECTION("Transpose") {
    Mat4 mat = {{ 1,  2,  3,  4},
                { 5,  6,  7,  8},
                { 9, 10, 11, 12},
                {13, 14, 15, 16}};

    Mat4 transpose = Transpose(mat);

    CHECK_ROW(transpose.row(0),  1,  5,  9, 13);
    CHECK_ROW(transpose.row(1),  2,  6, 10, 14);
    CHECK_ROW(transpose.row(2),  3,  7, 11, 15);
    CHECK_ROW(transpose.row(3),  4,  8, 12, 16);
  }
}
// clang-format on

}  // namespace
}  // namespace test
}  // namespace rothko
