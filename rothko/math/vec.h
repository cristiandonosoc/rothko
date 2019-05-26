// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <array>
#include <string>

#include "rothko/utils/logging.h"
#include "rothko/utils/strings.h"

namespace rothko {

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

  // * is dot product.
  T operator*(const _v2& o) const { return x * o.x + y * o.y; }

  bool operator==(const _v2& o) const { return x == o.x && y == o.y; }
  bool operator!=(const _v2& o) const { return x != o.x || y != o.y; }
};

using Int2 = _v2<int>;
using Vec2 = _v2<float>;

template <typename T>
inline bool IsZero(const _v2<T>& v) { return v.x == 0 && v.y == 0; }

template <typename T>
inline std::string ToString(const _v2<T>& v) {
  return StringPrintf("(%f, %f)", (float)v.x, (float)v.y);
}

// Vec 3 -----------------------------------------------------------------------

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

  void operator==(const _v3& o) const {
    return x == o.x && y == o.y && z == o.z;
  }
  void operator!=(const _v3& o) const {
    return x != o.x || y != o.y || z != o.z;
  }
};

using Int3 = _v3<int>;
using Vec3 = _v3<float>;

template <typename T>
inline bool IsZero(const _v3<T>& v) { return v.x == 0 && v.y == 0 && v.z == 0; }

template <typename T>
inline std::string ToString(const _v3<T>& v) {
  return StringPrintf("(%f, %f, %f)", (float)v.x, (float)v.y, (float)v.z);
}

// Vec 4 -----------------------------------------------------------------------

template<typename T>
struct _v4 {
  struct { T x, y, z, w; };
  struct { T r, g, b, a; };
  T elements[4];

  // Operators

  _v4() = default;
  _v4(T _x, T _y, T _z, T _w) { x = _x; y = _y; z = _z; w = _w; }

  _v4 operator+(const _v4 &o) const {
    return {x + o.x, y + o.y, z + o.z, w + o.w};
  }
  void operator+=(const _v4& o) { x += o.x; y += o.y; z += o.z; w += o.w; }

  void operator==(const _v4& o) const {
    return x == o.x && y == o.y && z == o.z && w == o.w;
  }
  void operator!=(const _v4& o) const {
    return x != o.x || y != o.y || z != o.z || w != o.w;
  }
};

using Int4 = _v4<int>;
using Vec4 = _v4<float>;

template <typename T>
inline std::string ToString(const _v4<T>& v) {
  return StringPrintf("(%f, %f, %f, %f)",
                      (float)v.x, (float)v.y, (float)v.z, (float)v.w);
}

template <typename T>
inline float Sum(const _v4<T>& v) {
  return v.x + v.y + v.z + v.w;
}

// Matrices --------------------------------------------------------------------
//
// Matrices are implemented as column mayor.

template <typename T>
union _mat2 {
  _v2<T> columns[2];

  // Operators -----------------------------------------------------------------

  _mat2() = default;
  _mat2(_v2<T> c1, _v2<T> c2) { columns[0] = c1; columns[1] = c2; }

  static _mat2 FromRows(_v2<T> r1, _v2<T> r2) {
    return _mat2({r1[0], r2[0]}, {r1[1], r2[1]});
  }

  _v2<T>& operator[](int index) {
    ASSERT(index >= 0 && index < 2);
    return columns[index];
  }

  _v2<T> operator*(const _v2<T>& v) const {
    return {columns[0][0] * v[0] + columns[1][0] * v[1],
            columns[0][1] * v[0] + columns[1][1] * v[1]};
  }
};

template <typename T>
std::array<T, 4> ToRowArray(_mat2<T>* mat) {
  std::array<T, 4> array;
  array[0] = (*mat)[0][0];
  array[1] = (*mat)[1][0];
  array[2] = (*mat)[0][1];
  array[3] = (*mat)[1][1];
  return array;
}

using IntMat2 = _mat2<int>;
using Mat2 = _mat2<float>;

}  // rothko
