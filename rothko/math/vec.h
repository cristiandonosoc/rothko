// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <array>
#include <string>

#include "rothko/utils/logging.h"

namespace rothko {

// =================================================================================================
// Math Functions.
// =================================================================================================

float SquareRoot(float);
float Tan(float radian_angle);

// =================================================================================================
// Vectors
// =================================================================================================

// Vec 2 -------------------------------------------------------------------------------------------

template <typename T>
union _v2 {
  struct { T x, y; };
  struct { T u, v; };
  struct { T left, right; };
  struct { T min, max; };
  struct { T width, height; };
  T elements[2];

  // Operators

  _v2() = default;
  _v2(T x, T y) { this->x = x; this->y = y; }

  static _v2 Zero() { return {0, 0}; }

  T& operator[](int index) {
    ASSERT(index >= 0 && index < 2);
    return elements[index];
  }
  const T& operator[](int index) const {
    return (*((_v2*)(this)))[index];
  }

  _v2 operator+(const _v2& o) const { return {x + o.x, y + o.y}; }
  void operator+=(const _v2& o) { x += o.x; y += o.y; }

  _v2 operator-(const _v2& o) const { return {x - o.x, y - o.y}; }
  void operator-=(const _v2& o) { x -= o.x; y -= o.y; }

  bool operator==(const _v2& o) const { return x == o.x && y == o.y; }
  bool operator!=(const _v2& o) const { return x != o.x || y != o.y; }
};

using Int2 = _v2<int>;
using Vec2 = _v2<float>;

template <typename T>
inline bool IsZero(const _v2<T>& v) { return v.x == 0 && v.y == 0; }

template <typename T>
T Dot(const _v2<T>& a, const _v2<T>& b) { return a.x * b.x + a.y * b.y; }

template <typename T>
T LengthSq(const _v2<T>& v) { return Dot(v, v); }

template <typename T>
float Length(const _v2<T>& v) { return SquareRoot(LengthSq(v)); }

Vec2 Normalize(const Vec2& v);

template <typename T>
inline float Sum(const _v2<T>& v) { return v.x + v.y; }

std::string ToString(const Int2&);
std::string ToString(const Vec2&);

// Vec 3 -------------------------------------------------------------------------------------------

template <typename T>
union _v3 {
  struct { T x, y, z; };
  struct { T u, v, w; };
  struct { T r, g, b; };
  T elements[3];

  // Operators

  _v3() = default;
  _v3(T x, T y, T z) { this->x = x; this->y = y; this->z = z; }

  static _v3 Zero() { return {0, 0, 0}; }

  _v3 operator+(const _v3& o) const { return {x + o.x, y + o.y, z + o.z}; }
  void operator+=(const _v3& o) { x += o.x; y += o.y; z += o.z; }

  _v3 operator-(const _v3& o) const { return {x - o.x, y - o.y, z - o.z}; }
  void operator-=(const _v3& o) { x -= o.x; y -= o.y; z -= o.z; }

  void operator==(const _v3& o) const { return x == o.x && y == o.y && z == o.z; }
  void operator!=(const _v3& o) const { return x != o.x || y != o.y || z != o.z; }
};

using Int3 = _v3<int>;
using Vec3 = _v3<float>;

template <typename T>
inline bool IsZero(const _v3<T>& v) { return v.x == 0 && v.y == 0 && v.z == 0; }

template <typename T>
_v3<T> Cross(const _v3<T>& v1, const _v3<T>& v2) {
  Vec3 result;

  result.x = (v1.y * v2.z) - (v1.z * v2.y);
  result.y = (v1.z * v2.x) - (v1.x * v2.z);
  result.z = (v1.x * v2.y) - (v1.y * v2.x);

  return result;
}

template <typename T>
T Dot(const _v3<T>& a, const _v3<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

template <typename T>
T LengthSq(const _v3<T>& v) { return Dot(v, v); }

template <typename T>
float Length(const _v3<T>& v) { return SquareRoot(LengthSq(v)); }

Vec3 Normalize(const Vec3& v);

template <typename T>
inline float Sum(const _v3<T>& v) { return v.x + v.y + v.z; }

std::string ToString(const Int3&);
std::string ToString(const Vec3&);

// Vec 4 -------------------------------------------------------------------------------------------

template<typename T>
struct _v4 {
  struct { T x, y, z, w; };
  struct { T r, g, b, a; };
  T elements[4];

  // Operators

  _v4() = default;
  _v4(T _x, T _y, T _z, T _w) { x = _x; y = _y; z = _z; w = _w; }

  _v4 operator+(const _v4 &o) const { return {x + o.x, y + o.y, z + o.z, w + o.w}; }
  void operator+=(const _v4& o) { x += o.x; y += o.y; z += o.z; w += o.w; }

  _v4 operator-(const _v4 &o) const { return {x - o.x, y - o.y, z - o.z, w - o.w}; }
  void operator-=(const _v4& o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; }

  void operator==(const _v4& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
  void operator!=(const _v4& o) const { return x != o.x || y != o.y || z != o.z || w != o.w; }
};

using Int4 = _v4<int>;
using Vec4 = _v4<float>;

template <typename T>
T Dot(const _v4<T>& a, const _v4<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

template <typename T>
T LengthSq(const _v4<T>& v) { return Dot(v, v); }

template <typename T>
float Length(const _v4<T>& v) { return SquareRoot(LengthSq(v)); }

Vec4 Normalize(const Vec4& v);

template <typename T>
inline float Sum(const _v4<T>& v) { return v.x + v.y + v.z + v.w; }

std::string ToString(const Int4&);
std::string ToString(const Vec4&);

// =================================================================================================
// Matrices
//
// Matrices are implemented as column mayor.
// =================================================================================================

// Mat 2 -------------------------------------------------------------------------------------------

template <typename T>
union _mat2 {
  // Members.

  _v2<T> cols[2] = {};
  float elements[2][2];

  // Constructors.

  _mat2() = default;
  _mat2(_v2<T> r1, _v2<T> r2) {
    cols[0] = {r1[0], r2[0]};
    cols[1] = {r1[1], r2[1]};
  }

  // Operators

  _v2<T>& operator[](int index) {
    ASSERT(index >= 0 && index < 2);
    return cols[index];
  }

  _v2<T> operator*(const _v2<T>& v) const {
    return {cols[0][0] * v[0] + cols[1][0] * v[1],
            cols[0][1] * v[0] + cols[1][1] * v[1]};
  }
};

using IntMat2 = _mat2<int>;
using Mat2 = _mat2<float>;

// Mat4 ------------------------------------------------------------------------

template <typename T>
union _mat4 {
  // Members.

  _v4<T> cols[4] = {};
  float elements[4][4];

  // Constructors.

  _mat4() = default;
  _mat4(_v4<T> r1, _v4<T> r2, _v4<T> r3, _v4<T> r4) {
    cols[0] = {r1[0], r2[0], r3[0], r4[0]};
    cols[1] = {r1[1], r2[1], r3[1], r4[1]};
    cols[2] = {r1[2], r2[2], r3[2], r4[2]};
    cols[3] = {r1[0], r2[0], r3[0], r4[0]};
  }

  _mat4 Identity() { return {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}; }

  // Operators.

  bool operator==(const _mat4& o) const {
    return cols[0] == o.cols[0] && cols[1] == o.cols[1] &&
           cols[2] == o.cols[2] && cols[3] == o.cols[3];
  }
  bool operator!=(const _mat4& o) const {
    return !(*this == o);
  }
};

using IntMat4 = _mat4<int>;
using Mat4 = _mat4<float>;

// Usually |up| points upward. Our Y vector represents up.
Mat4 LookAt(Vec3 pos, Vec3 target, Vec3 up = {0, 1, 0});

// Returns |Perspective| after calculating the correct values.
Mat4 Perspective(float fov, float aspect_ratio, float near, float far);
Mat4 Perspective(float left, float right, float top, float bottom, float near, float far);

template <typename T>
void SetRowCol(_mat4<T>* m, T x, T y) {
  (*m)[y][x];
}


}  // rothko
