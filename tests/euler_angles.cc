// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <third_party/catch2/catch.hpp>

#include <rothko/math/math.h>

namespace rothko {

namespace {

#define DIFF(lhs, rhs) std::abs(rhs - lhs)

#define COMPARE_VECTORS(lhs, rhs, epsilon)         \
  {                                                \
    Vec3 lhs_vec = (lhs);                          \
    Vec3 rhs_vec = (rhs);                          \
    CHECK(DIFF(lhs_vec.x, rhs_vec.x) < epsilon); \
    CHECK(DIFF(lhs_vec.y, rhs_vec.y) < epsilon); \
    CHECK(DIFF(lhs_vec.z, rhs_vec.z) < epsilon); \
  }

constexpr float kCos45 = kSqrt2 / 2;
/* constexpr float kSin45 = Math::kSqrt2 / 2; */

}  // namespace

TEST_CASE("Direction From Euler") {
  Vec3 dir;
  Vec3 expected;
  float epsilon = 0.05f;

  SECTION("Only pitch") {
    dir = DirectionFromEulerDeg(0.0f, 0.0f);
    expected = {1.0f, 0.0f, 0.0f};
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(45.0f, 0.0f);
    expected = {kCos45, kCos45, 0.0f};
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(180.0f, 0.0f);
    expected =  {-1.0f, 0.0f, 0.0f};
    COMPARE_VECTORS(dir, expected, epsilon);
  }

  SECTION("Only yaw") {
    dir = DirectionFromEulerDeg(0.0f, 45.0f);
    expected = { kCos45, 0.0f, kCos45 };
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(0.0f, 180.0f);
    expected =  {-1.0f, 0.0f, 0.0f};
    COMPARE_VECTORS(dir, expected, epsilon);
  }

  SECTION("Pitch & Yaw") {
    dir = DirectionFromEulerDeg(45.0f, 45.0f);
    expected = { 0.5f , kSqrt2 / 2.0f, 0.5f };
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(-45.0f, 45.0f);
    expected = { 0.5f , -kSqrt2 / 2.0f, 0.5f };
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(45.0f + 90.0f, 45.0f);
    expected = { -0.5f , kSqrt2 / 2.0f, -0.5f };
    COMPARE_VECTORS(dir, expected, epsilon);
  }
}

TEST_CASE("Euler from direction") {
  Vec2 angles;
  SECTION("On the XZ plane") {
    angles = EulerFromDirectionDeg(Normalize({1.0f, 0.0f, 0.0f}));
    CHECK(angles.x == Approx(0.0f));
    CHECK(angles.y == Approx(0.0f));

    angles = EulerFromDirectionDeg(Normalize({1.0f, 0.0f, 1.0f}));
    CHECK(angles.x == Approx(0.0f));
    CHECK(angles.y == Approx(45.0f));

    angles = EulerFromDirectionDeg(Normalize({0.0f, 0.0f, 1.0f}));
    CHECK(angles.x == Approx(0.0f));
    CHECK(angles.y == Approx(90.0f));

    angles = EulerFromDirectionDeg(Normalize({-1.0f, 0.0f, 1.0f}));
    CHECK(angles.x == Approx(0.0f));
    CHECK(angles.y == Approx(135.0f));

    angles = EulerFromDirectionDeg(Normalize({-1.0f, 0.0f, 0.0f}));
    CHECK(angles.x == Approx(0.0f));
    CHECK(angles.y == Approx(180.0f));

    angles = EulerFromDirectionDeg(Normalize({-1.0f, 0.0f, -1.0f}));
    CHECK(angles.x == Approx(0.0f));
    CHECK(angles.y == Approx(225.0f));

    angles = EulerFromDirectionDeg(Normalize({0.0f, 0.0f, -1.0f}));
    CHECK(angles.x == Approx(0.0f));
    CHECK(angles.y == Approx(270.0f));

    angles = EulerFromDirectionDeg(Normalize({1.0f, 0.0f, -1.0f}));
    CHECK(angles.x == Approx(0.0f));
    CHECK(angles.y == Approx(315.0f));
  }

  SECTION("Upper angles") {
    angles = EulerFromDirectionDeg(Normalize({1.0f, 1.0f, 0.0f}));
    CHECK(angles.x == Approx(45.0f));
    CHECK(angles.y == Approx(0.0f));

    angles = EulerFromDirectionDeg(Normalize({1.0f, 1.0f, 1.0f}));
    CHECK(angles.x == Approx(35.264f));
    CHECK(angles.y == Approx(45.0f));

    angles = EulerFromDirectionDeg(Normalize({0.0f, 1.0f, 1.0f}));
    CHECK(angles.x == Approx(45.0f));
    CHECK(angles.y == Approx(90.0f));

    angles = EulerFromDirectionDeg(Normalize({-1.0f, 1.0f, 1.0f}));
    CHECK(angles.x == Approx(35.264f));
    CHECK(angles.y == Approx(135.0f));

    angles = EulerFromDirectionDeg(Normalize({-1.0f, 1.0f, 0.0f}));
    CHECK(angles.x == Approx(45.0f));
    CHECK(angles.y == Approx(180.0f));

    angles = EulerFromDirectionDeg(Normalize({-1.0f, 1.0f, -1.0f}));
    CHECK(angles.x == Approx(35.264f));
    CHECK(angles.y == Approx(225.0f));

    angles = EulerFromDirectionDeg(Normalize({0.0f, 1.0f, -1.0f}));
    CHECK(angles.x == Approx(45.0f));
    CHECK(angles.y == Approx(270.0f));

    angles = EulerFromDirectionDeg(Normalize({1.0f, 1.0f, -1.0f}));
    CHECK(angles.x == Approx(35.264f));
    CHECK(angles.y == Approx(315.0f));
  }

  SECTION("Lower angles") {
    angles = EulerFromDirectionDeg(Normalize({1.0f, -1.0f, 0.0f}));
    CHECK(angles.x == Approx(315.0f));
    CHECK(angles.y == Approx(0.0f));

    angles = EulerFromDirectionDeg(Normalize({1.0f, -1.0f, 1.0f}));
    CHECK(angles.x == Approx(324.7356));
    CHECK(angles.y == Approx(45.0f));

    angles = EulerFromDirectionDeg(Normalize({0.0f, -1.0f, 1.0f}));
    CHECK(angles.x == Approx(315.0f));
    CHECK(angles.y == Approx(90.0f));

    angles = EulerFromDirectionDeg(Normalize({-1.0f, -1.0f, 1.0f}));
    CHECK(angles.x == Approx(324.7356));
    CHECK(angles.y == Approx(135.0f));

    angles = EulerFromDirectionDeg(Normalize({-1.0f, -1.0f, 0.0f}));
    CHECK(angles.x == Approx(315.0f));
    CHECK(angles.y == Approx(180.0f));

    angles = EulerFromDirectionDeg(Normalize({-1.0f, -1.0f, -1.0f}));
    CHECK(angles.x == Approx(324.7356));
    CHECK(angles.y == Approx(225.0f));

    angles = EulerFromDirectionDeg(Normalize({0.0f, -1.0f, -1.0f}));
    CHECK(angles.x == Approx(315.0f));
    CHECK(angles.y == Approx(270.0f));

    angles = EulerFromDirectionDeg(Normalize({1.0f, -1.0f, -1.0f}));
    CHECK(angles.x == Approx(324.7356));
    CHECK(angles.y == Approx(315.0f));
  }
}

}  // namespace rothko
