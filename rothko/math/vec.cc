// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/math/vec.h"

#include <cmath>

namespace rothko {

// Math Functions ----------------------------------------------------------------------------------

float SquareRoot(float f) {
  return std::sqrt(f);
}


// Vectors -----------------------------------------------------------------------------------------

Vec2 Normalize(const Vec2& v) {
  Vec2 result = {};

  float length = Length(v);

  if (length != 0.0f) {
    result.x = v.x / length;
    result.y = v.y / length;
  }

  return result;
}

Vec3 Normalize(const Vec3& v) {
  Vec3 result = {};

  float length = Length(v);

  if (length != 0.0f) {
    result.x = v.x / length;
    result.y = v.y / length;
    result.z = v.z / length;
  }

  return result;
}

Vec4 Normalize(const Vec4& v) {
  Vec4 result = {};

  float length = Length(v);

  if (length != 0.0f) {
    result.x = v.x / length;
    result.y = v.y / length;
    result.z = v.z / length;
    result.w = v.w / length;
  }

  return result;
}

// Matrix ------------------------------------------------------------------------------------------

Mat4 LookAt(Vec3 pos, Vec3 target, Vec3 up) {

}

}  // namespace rothko
