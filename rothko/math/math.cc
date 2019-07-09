// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/math/math.h"

#include <cmath>

#include "rothko/utils/strings.h"

namespace rothko {

// Math Functions ----------------------------------------------------------------------------------

float SquareRoot(float f) { return std::sqrt(f); }

float Sin(float radian_angle) { return std::sin(radian_angle); }
float Cos(float radian_angle) { return std::cos(radian_angle); }
float Tan(float radian_angle) { return std::tan(radian_angle); }

//Vectors ------------------------------------------------------------------------------------------

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

// Transformation Matrices -------------------------------------------------------------------------

Mat4 Translate(const Vec3& v) {
  // clang-format off
  return Mat4({  1,   0,   0, 0},
              {  0,   1,   0, 0},
              {  0,   0,   1, 0},
              { v.x, v.y, v.z,   1});
  // clang-format on
}

Mat4 Rotate(const Vec3& v, float radian_angle) {
  float sin  = Sin(radian_angle);
  float cos  = Cos(radian_angle);
  float cosm = (1 - cos);

  // The angle has to be normalized.
  Vec3 u = Normalize(v);

  // clang-format off
  return Mat4(
    {      cos + u.x * u.x * cosm, u.x * u.y * cosm + u.z * sin, u.x * u.z * cosm - u.y * sin,  0},
    {u.y * u.x * cosm - u.z * sin,       cos + u.y * u.y * cosm, u.y * u.z * cosm + u.x * sin,  0},
    {u.z * u.x * cosm + u.y * sin, u.z * u.y * cosm - u.x * sin,       cos + u.z * u.z * cosm,  0},
    {                           0,                            0,                            0,  1});
  // clang-format on
}

Mat4 Scale(const Vec3& v) {
  // clang-format on
  return Mat4({ v.x,   0,   0,   0},
              {   0, v.y,   0,   0},
              {   0,   0, v.z,   0},
              {   0,   0,   0,   1});
  // clang-format off
}

Mat4 LookAt(Vec3 pos, Vec3 target, Vec3 hint_up) {
  // We calculate the new axis for the coordinate system.
  Vec3 forward = Normalize(target - pos);             // Z: Point into the target.
  Vec3 right   = Normalize(Cross(forward, hint_up));  // X: Right to the new Z axis.
  Vec3 up      = Cross(right, forward);               // Y: Simply cross Z and X.

  // NOTE: Each field in this matrix is pre-calculated to be the new matrix that transform the
  //       world to view-space. In reality, this is the result of a two step process:
  //          1. Rotate the world to the new coordinate system (forward, up, right).
  //          2. Translate the world to the camera position.
  //       Then this would result in the following matrix multiplication:
  //
  //       [   1,    0,    0,  0]   [r.x, r.y, r.z,  0]
  //       [   0,    1,    0,  0] * [u.x, u.y, u.z,  0]
  //       [   0,    0,    1,  0]   [f.x, f.y, f.z,  0]
  //       [-p.x, -p.y, -p.z,  0]   [  0,   0,   0,  1]
  //
  //       Thus explaining the final row with the dot products.
  //
  //       NOTE2: The translation is negative because the camera Z axis *points in* the forward
  //              direction, thus making the camera *look in* it's negative -Z axis.
  //       NOTE3: f = forward, u = up, r = right, p = pos.

  // clang-format off
  return Mat4({          right.x,        right.y,           right.z,     0},
              {             up.x,           up.y,              up.z,     0},
              {       -forward.x,     -forward.y,        -forward.z,     0},
              { -Dot(right, pos),  -Dot(up, pos), Dot(forward, pos),     1});
  // clang-format on
}

Mat4 Frustrum(float l, float r, float b, float t, float n, float f) {
  // clang-format off
  return Mat4{{  2 * n / (r - l),                 0,                    0,       0},
              {                0,   2 * n / (t - b),                    0,       0},
              {(r + l) / (r - l), (t + b) / (t - b),   -(f + n) / (f - n),      -1},
              {                0,                 0, -2 * f * n / (f - n),       0}};
  // clang-format on
}

Mat4 Ortho(float l, float r, float b, float t) {
  // clang-format off
  return Mat4({       2 / (r - l),                  0,        0,        0},
              {                 0,        2 / (t - b),        0,        0},
              {                 0,                  0,       -1,        0},
              {-(r + l) / (r - l), -(t + b) / (t - b),        0,        1});
  // clang-format on
}

Mat4 Ortho(float l, float r, float b, float t, float n, float f) {
  // clang-format off
  return Mat4({       2 / (r - l),                  0,                 0,        0},
              {                 0,        2 / (t - b),                 0,        0},
              {                 0,                  0,      -2 / (f - n),        0},
              {-(r + l) / (r - l), -(t + b) / (t - b), -(f + n) / (f- n),        1});
  // clang-format on
}

Mat4 Perspective(float fov, float aspect_ratio, float near, float far) {
  float left, right, top, bottom = {};
  top    = near * Tan(fov / 2.0f);
  bottom = -top;

  right = top * aspect_ratio;
  left  = -right;

  return Frustrum(left, right, bottom, top, near, far);
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
  // clang-format off
  return StringPrintf("(%f, %f, %f, %f), (%f, %f, %f, %f), (%f, %f, %f, %f), (%f, %f, %f, %f)",
                      e[0][0], e[0][1], e[0][2], e[0][3],
                      e[1][0], e[1][1], e[1][2], e[1][3],
                      e[2][0], e[2][1], e[2][2], e[2][3],
                      e[3][0], e[3][1], e[3][2], e[3][3]);
  // clang-format on
}

}  // namespace rothko
