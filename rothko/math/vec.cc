// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/math/vec.h"

#include <cmath>

#include "rothko/utils/strings.h"

namespace rothko {

// Math Functions ----------------------------------------------------------------------------------

float SquareRoot(float f) {
  return std::sqrt(f);
}

float Tan(float radian_angle) {
  return std::tan(radian_angle);
}

//Vectors -----------------------------------------------------------------------------------------

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
    Vec3 forward = Normalize(pos - target);
    Vec3 right = Normalize(Cross(forward, up));
    Vec3 cam_up = Cross(right, forward);

    return Mat4({  right.x,   right.y,   right.z, 0},
                { cam_up.x,  cam_up.y,  cam_up.z, 0},
                {forward.x, forward.y, forward.z, 0},
                {        0,         0,         0, 1});
}

Mat4 Perspective(float fov, float aspect_ratio, float near, float far) {
  float left, right, top, bottom = {};
  top = near * Tan(fov / 2.0f);
  bottom = -top;

  right = top * aspect_ratio;
  left = -right;

  return Perspective(left, right, bottom, top, near, far);
}

Mat4 Perspective(float l, float r, float b, float t, float n, float f) {
  return Mat4{{2 * n / (r - l),               0,  (r + l) / (r - l),                    0},
              {              0, 2 * n / (t - b),  (t + b) / (t - b),                    0},
              {              0,               0, -(f + n) / (f - n), -2 * f * n / (f - n)},
              {              0,               0,                 -1,                    0}};
}



// Printing ----------------------------------------------------------------------------------------

std::string ToString(const Int2& v) { return StringPrintf("(%d, %d)", v.x, v.y); }
std::string ToString(const Vec2& v) { return StringPrintf("(%f, %f)", v.x, v.y); }

std::string ToString(const Int3& v) { return StringPrintf("(%d, %d, %d)", v.x, v.y, v.z); }
std::string ToString(const Vec3& v) { return StringPrintf("(%f, %f, %f)", v.x, v.y, v.z); }

std::string ToString(const Int4& v) { return StringPrintf("(%d, %d, %d, %d)", v.x, v.y, v.z, v.w); }
std::string ToString(const Vec4& v) { return StringPrintf("(%f, %f, %f, %f)", v.x, v.y, v.z, v.w); }

std::string ToString(const Mat4& m) {
  auto& e = m.elements;
  return StringPrintf("(%f, %f, %f, %f), (%f, %f, %f, %f), (%f, %f, %f, %f), (%f, %f, %f, %f)",
                      e[0][0], e[1][0], e[2][0], e[3][0],
                      e[0][1], e[1][1], e[2][1], e[3][1],
                      e[0][2], e[1][2], e[2][2], e[3][2],
                      e[0][3], e[1][3], e[2][3], e[3][3]);
}

}  // namespace rothko
