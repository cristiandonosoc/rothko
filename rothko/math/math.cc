// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/math/math.h"

#include <cmath>

#include "rothko/utils/strings.h"

#include "rothko/logging/logging.h"

namespace rothko {

// Math Functions ==================================================================================

float SquareRoot(float f) { return std::sqrt(f); }

float Sin(float radian_angle) { return std::sin(radian_angle); }
float Asin(float radian_angle) { return std::asin(radian_angle); }

float Cos(float radian_angle) { return std::cos(radian_angle); }
float Acos(float radian_angle) { return std::acos(radian_angle); }

float Tan(float radian_angle) { return std::tan(radian_angle); }
float Atan(float radian_angle) { return std::atan(radian_angle); }

float Atan2(float x, float y) { return std::atan2(x, y); }

//Vectors ==========================================================================================

// Vec2 --------------------------------------------------------------------------------------------

Vec2 Normalize(const Vec2& v) {
  Vec2 result = {};

  float length = Length(v);

  if (length != 0.0f) {
    result.x = v.x / length;
    result.y = v.y / length;
  }

  return result;
}

// Vec3 --------------------------------------------------------------------------------------------

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

// Vec4 --------------------------------------------------------------------------------------------

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

// Transformation Matrices =========================================================================

Mat4 FromRows(Vec3 x, Vec3 y, Vec3 z) {
  return {{x.x, x.y, x.z,  0},
          {y.x, y.y, y.z,  0},
          {z.x, z.y, z.z,  0},
          {  0,   0,   0,  1}};
}

Mat4 FromColumns(Vec3 x, Vec3 y, Vec3 z) {
  return {{x.x, y.y, z.z,  0},
          {x.x, y.y, z.z,  0},
          {x.x, y.y, z.z,  0},
          {  0,   0,   0,  1}};
}


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

Vec3 Rotate(Vec3 pos, float radian_x, float radian_y) {
  if (radian_x != 0) {
    pos = RotateX(pos, radian_x);
  }

  if (radian_y != 0) {
    pos = RotateY(pos, radian_y);
  }
  return pos;
}

Mat4 Scale(const Vec3& v) {
  // clang-format on
  return {{ v.x,   0,   0,   0},
          {   0, v.y,   0,   0},
          {   0,   0, v.z,   0},
          {   0,   0,   0,   1}};
  // clang-format off
}

Mat4 LookAt(Vec3 pos, Vec3 target, Vec3 hint_up) {
  // We calculate the new axis for the coordinate system.
  Vec3 forward = Normalize(pos - target);             // Z: Point into the target.
  Vec3 right   = Normalize(Cross(hint_up, forward));  // X: Right to the new Z axis.
  Vec3 up      = Cross(forward, right);               // Y: Simply cross Z and X.
                                                      //    Comes out normalized.
  // NOTE: Each field in this matrix is pre-calculated to be the new matrix that transform the
  //       world to view-space. In reality, this is the result of a two step process:
  //          1. Rotate the world to the new coordinate system (forward, up, right).
  //          2. Translate the world to the camera position.
  //       Then this would result in the following matrix multiplication:
  //
  //       [r.x, r.y, r.z,  0]   [   1,    0,    0,  0]
  //       [u.x, u.y, u.z,  0] * [   0,    1,    0,  0]
  //       [f.x, f.y, f.z,  0]   [   0,    0,    1,  0]
  //       [  0,   0,   0,  1]   [-p.x, -p.y, -p.z,  0]
  //
  //       Thus explaining the final row with the dot products.
  //
  //       NOTE2: The translation is negative because the camera Z axis *points in* the forward
  //              direction, thus making the camera *look in* it's negative -Z axis.
  //       NOTE3: f = forward, u = up, r = right, p = pos.

  // clang-format off
  /* return {{          right.x,           up.x,          forward.z,     0}, */
  /*         {          right.y,           up.y,          forward.z,     0}, */
  /*         {          right.z,           up.z,          forward.z,     0}, */
  /*         { -Dot(right, pos),  -Dot(up, pos), -Dot(forward, pos),     1}}; */

  return {{          right.x,        right.y,            right.z,   -Dot(right, pos)},
          {             up.x,           up.y,               up.z,      -Dot(up, pos)},
          {        forward.x,      forward.y,          forward.z, -Dot(forward, pos)},
          {                0,              0,                  0,                  1}};
  // clang-format on
}

Mat4 Frustrum(float l, float r, float b, float t, float n, float f) {
  // clang-format off
  return {{  2 * n / (r - l),                 0,                    0,                    0},
          {                0,   2 * n / (t - b),                    0,                    0},
          {(r + l) / (r - l), (t + b) / (t - b),   -(f + n) / (f - n), -2 * f * n / (f - n)},
          {                0,                 0,                   -1,                    0}};
  // clang-format on
}

Mat4 Ortho(float l, float r, float b, float t) {
  // clang-format off
  return {{       2 / (r - l),                  0,                 0,  -(r + l) / (r - l)},
          {                 0,        2 / (t - b),                 0,  -(t + b) / (t - b)},
          {                 0,                  0,                -1,                   0},
          {                 0,                  0,                 0,                   1}};

  // clang-format on
}

Mat4 Ortho(float l, float r, float b, float t, float n, float f) {
  // clang-format off
  return {{       2 / (r - l),                  0,                 0,  -(r + l) / (r - l)},
          {                 0,        2 / (t - b),                 0,  -(t + b) / (t - b)},
          {                 0,                  0,      -2 / (f - n),  -(f + n) / (f - n)},
          {                 0,                  0,                 0,                   1}};
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

// Printing ========================================================================================

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

// Euler Angles ====================================================================================

Vec3 DirectionFromEuler(float pitch, float yaw) {
  Vec3 direction;
  direction.x = Cos(pitch) * Cos(yaw);
  direction.y = Sin(pitch);
  direction.z = Cos(pitch) * Sin(yaw);
  return Normalize(direction);
}

Vec2 EulerFromDirection(const Vec3& direction) {
  Vec2 result;
  // Pitch.
  result.x = Asin(direction.y);

  // Yaw.
  result.y = Atan2(direction.z, direction.x);
  return result;
}

// Quaternion ======================================================================================

Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t) {
  float cos_angle = Dot(q1, q2);
  float angle = Acos(cos_angle);

  float s1 = Sin((1.0f - t) * angle);
  float s2 = Sin(t * angle);
  float i = 1.0f / Sin(angle);

  Quaternion left = q1 * s1;
  Quaternion right = q2 * s2;

  Quaternion res = left + right;
  res *= i;

  return res;
}

/* Mat4 ToMat4(const Quaternion& q) { */
/*   Quaternion n = Normalize(q); */

/*   float xx = n.x * n.x; */
/*   float yy = n.y * n.y; */
/*   float zz = n.z * n.z; */
/*   float xy = n.x * n.y; */
/*   float xz = n.x * n.z; */
/*   float yz = n.y * n.z; */
/*   float xw = n.x * n.w; */
/*   float yw = n.y * n.w; */
/*   float zw = n.z * n.w; */

/*   // clang-format off */
/*   return Mat4({1 - 2 * yy - 2 * zz,     2 * xy - 2 * zw,     2 * xz + 2 * yw,         0}, */
/*               {    2 * xy + 2 * zw, 1 - 2 * xx - 2 * zz,     2 * yz - 2 * xw,         0}, */
/*               {    2 * xz - 2 * yw,     2 * yz + 2 * xw, 1 - 2 * xx - 2 * yy,         0}, */
/*               {                  0,                   0,                   0,         0}); */
/*   // clang-format on */
/* } */

}  // namespace rothko
