// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <random>
#include <string>

// This is Rothko's math definitions and functions. This includes generic math functions (sin, cos),
// vectors and matrices, transformations and whatnot.
//
// NOTE: This is not meant (as of now) to be a comprehensive math library, but rather it grows
//       according to the needs of the engine.

namespace rothko {

// =================================================================================================
// Math Functions.
// =================================================================================================

#ifndef ABS
#define ABS(x) (((x) < 0) ? (x) : (-(x)))
#endif

constexpr float kPI = 3.14159265359f;
constexpr double kPI64 = 3.14159265358979323846;
constexpr float kSqrt2 = 1.4142135623730950488f;
constexpr float kRadians45 = kPI / 4.0f;
constexpr float kRadians90 = kPI / 2.0f;
constexpr float kRadians180 = kPI;
constexpr float kRadians360 = 2.0f * kPI;

float Sqrt(float);

float Sin(float radian_angle);
float Asin(float radian_angle);

float Cos(float radian_angle);
float Acos(float radian_angle);

float Tan(float radian_angle);
float Atan(float radian_angle);

// Atan2 uses both |x| and |y| to determine the quadrant. They're *not* angles.
// Think of it as a unit vector in a unit circle.
float Atan2(float x, float y);

inline float ToRadians(float degrees) { return degrees * (kPI / 180.0f); }
inline float ToDegrees(float radians) {
  float deg = 180.0f * radians / kPI;
  if (deg < 0.0f)
    deg += 360.0f;
  return deg;
}

#define IS_EVEN(x) ((x) % 2 == 0)
#define IS_ODD(x) ((x) % 2 == 1)

// Returns [min, max].
// TODO(Cristian): Implement a better random.
inline int Random(int min, int max) {
  int diff = max - min + 1;     // rand() will return [0, diff)
  return rand() % diff + min;   // [min, min + diff + 1) = [min, max + 1)
}

// |t| is interpolation value. t is in the [0, 1] range.
inline float Lerp(float a, float b, float t) { return (1.0f - t) * a - t * b; }

// =================================================================================================
// Bits
// =================================================================================================

template <typename T>
inline T GetBit(T reg, int bit) { return (reg >> bit) & 0b1; }

template <typename T>
inline T SetBit(T reg, int bit) { return reg | (0b1 << bit); }

template <typename T>
inline T ClearBit(T reg, int bit) { return reg & ~(0b1 << bit); }

// =================================================================================================
// Vectors
// =================================================================================================

// Vec 2 -------------------------------------------------------------------------------------------

template <typename T>
union _v2 {
  // Members.

  struct { T x, y; };
  struct { T u, v; };
  struct { T left, right; };
  struct { T min, max; };
  struct { T width, height; };
  T elements[2];

  // Constructors.

  _v2() = default;
  _v2(T _x, T _y) { x = _x; y = _y; }

  // Int2 <-> Vec2 conversions.
  template <typename U>
  explicit _v2(const _v2<U>& v) { x = (T)v.x, y = (T)v.y; }

  static _v2 Zero() { return {0, 0}; }

  // Operators

  T& operator[](int index) { return elements[index]; }
  T operator[](int index) const { return elements[index]; }

  _v2 operator+(const _v2& o) const { return {x + o.x, y + o.y}; }
  void operator+=(const _v2& o) { x += o.x; y += o.y; }

  _v2 operator-(const _v2& o) const { return {x - o.x, y - o.y}; }
  void operator-=(const _v2& o) { x -= o.x; y -= o.y; }
  _v2 operator-() { return {-x, -y}; }  // Unary minus operator.

  _v2 operator*(const _v2& o) const { return {x * o.x, y * o.y}; }
  void operator*=(const _v2& o) { x *= o.x; y *= o.y; }

  _v2 operator*(const T& t) const { return {x * t, y * t}; }
  void operator*=(const T& t) { x *= t; y *= t; }

  _v2 operator/(const _v2& o) const { return {x / o.x, y / o.y}; }
  void operator/=(const _v2& o) { x /= o.x; y /= o.y; }

  _v2 operator/(const T& t) const { return {x / t, y / t}; }
  void operator/=(const T& t) { x /= t; y /= t; }

  bool operator==(const _v2& o) const { return x == o.x && y == o.y; }
  bool operator!=(const _v2& o) const { return x != o.x || y != o.y; }
};

using Int2 = _v2<int>;
using Vec2 = _v2<float>;

template <typename T>
_v2<T> Abs(const _v2<T>& v) { return {ABS(v.x), ABS(v.y)}; }

template <typename T>
inline bool IsZero(const _v2<T>& v) { return v.x == 0 && v.y == 0; }

template <typename T>
T Dot(const _v2<T>& a, const _v2<T>& b) { return a.x * b.x + a.y * b.y; }

template <typename T>
T LengthSq(const _v2<T>& v) { return Dot(v, v); }

template <typename T>
float Length(const _v2<T>& v) { return Sqrt(LengthSq(v)); }

Vec2 Normalize(const Vec2& v);

template <typename T>
inline float Sum(const _v2<T>& v) { return v.x + v.y; }

inline Int2 ToInt2(const Vec2& v) { return {(int)v.x, (int)v.y}; }

std::string ToString(const Int2&);
std::string ToString(const Vec2&);

// Vec 3 -------------------------------------------------------------------------------------------

template <typename T>
union _v3 {
  // Members.

  struct { T x, y, z; };
  struct { T u, v, w; };
  struct { T r, g, b; };
  T elements[3];

  // Constructors.

  _v3() = default;
  _v3(T _x, T _y, T _z) { x = _x; y = _y; z = _z; }

  // Int3 <-> Vec3 conversions.
  template <typename U>
  explicit _v3(const _v3<U>& v) { x = (T)v.x, y = (T)v.y; z = (T)v.z; }

  static _v3 Zero() { return {0, 0, 0}; }

  static _v3 Up() { return {0, 1, 0}; }

  // Operators

  T& operator[](int index) { return elements[index]; }
  T operator[](int index) const { return elements[index]; }

  _v3 operator+(const _v3& o) const { return {x + o.x, y + o.y, z + o.z}; }
  void operator+=(const _v3& o) { x += o.x; y += o.y; z += o.z; }

  _v3 operator-(const _v3& o) const { return {x - o.x, y - o.y, z - o.z}; }
  void operator-=(const _v3& o) { x -= o.x; y -= o.y; z -= o.z; }
  _v3 operator-() { return {-x, -y, -z}; }  // Unary minus operator.

  _v3 operator*(const _v3& o) const { return {x * o.x, y * o.y, z * o.z}; }
  void operator*=(const _v3& o) { x *= o.x; y *= o.y; z *= o.z; }

  _v3 operator*(const T& t) const { return {x * t, y * t, z * t}; }
  void operator*=(const T& t) { x *= t; y *= t; z *= t; }

  _v3 operator/(const _v3& o) const { return {x / o.x, y / o.y, z / o.z}; }
  void operator/=(const _v3& o) { x /= o.x; y /= o.y; z /= o.z; }

  _v3 operator/(const T& t) const { return {x / t, y / t, z / t}; }
  void operator/=(const T& t) { x /= t; y /= t; z /= t; }

  bool operator==(const _v3& o) const { return x == o.x && y == o.y && z == o.z; }
  bool operator!=(const _v3& o) const { return x != o.x || y != o.y || z != o.z; }
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
_v3<T> Abs(const _v3<T>& v) { return {ABS(v.x), ABS(v.y), ABS(v.z)}; }

template <typename T>
T Dot(const _v3<T>& a, const _v3<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

template <typename T>
T LengthSq(const _v3<T>& v) { return Dot(v, v); }

template <typename T>
float Length(const _v3<T>& v) { return Sqrt(LengthSq(v)); }

template <typename T, typename Func>
_v3<T> Map(const _v3<T>& v, Func f) { return _v3<T>(f(v.x), f(v.y), f(v.z)); }

Vec3 Normalize(const Vec3& v);

template <typename T>
inline float Sum(const _v3<T>& v) { return v.x + v.y + v.z; }

std::string ToString(const Int3&);
std::string ToString(const Vec3&);


// Vec 4 -------------------------------------------------------------------------------------------

template<typename T>
union _v4 {
  // Members.

  struct { T x, y, z, w; };
  struct { T r, g, b, a; };
  T elements[4];

  // Constructors.

  _v4() = default;
  _v4(T _x, T _y, T _z, T _w) { x = _x; y = _y; z = _z; w = _w; }

  // Int4 <-> Vec4 conversions.
  template <typename U>
  explicit _v4(const _v4<U>& v) { x = (T)v.x, y = (T)v.y; z = (T)v.z; w = (T)v.w; }

  static _v4 Zero() { return {0, 0, 0, 0}; }

  // Operators

  T& operator[](int index) { return elements[index]; }
  T operator[](int index) const { return elements[index]; }

  _v4 operator+(const _v4 &o) const { return {x + o.x, y + o.y, z + o.z, w + o.w}; }
  void operator+=(const _v4& o) { x += o.x; y += o.y; z += o.z; w += o.w; }

  _v4 operator-(const _v4 &o) const { return {x - o.x, y - o.y, z - o.z, w - o.w}; }
  void operator-=(const _v4& o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; }
  _v4 operator-() { return {-x, -y, -z, -w}; }  // Unary minus operator.

  _v4 operator*(const _v4 &o) const { return {x * o.x, y * o.y, z * o.z, w * o.w}; }
  void operator*=(const _v4& o) { x *= o.x; y *= o.y; z *= o.z; w *= o.w; }

  _v4 operator*(const T& t) const { return {x * t, y * t, z * t, w * t}; }
  void operator*=(const T& t) { x *= t; y *= t; z *= t; w *= t; }

  _v4 operator/(const _v4 &o) const { return {x / o.x, y / o.y, z / o.z, w / o.w}; }
  void operator/=(const _v4& o) { x /= o.x; y /= o.y; z /= o.z; w /= o.w; }

  _v4 operator/(const T& t) const { return {x / t, y / t, z / t, w / t}; }
  void operator/=(const T& t) { x /= t; y /= t; z /= t; w /= t; }

  bool operator==(const _v4& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
  bool operator!=(const _v4& o) const { return x != o.x || y != o.y || z != o.z || w != o.w; }
};

using Int4 = _v4<int>;
using Vec4 = _v4<float>;

template <typename T>
_v4<T> Abs(const _v4<T>& v) { return {ABS(v.x), ABS(v.y), ABS(v.z), ABS(v.w)}; }

template <typename T>
T Dot(const _v4<T>& a, const _v4<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

template <typename T>
T LengthSq(const _v4<T>& v) { return Dot(v, v); }

template <typename T>
float Length(const _v4<T>& v) { return Sqrt(LengthSq(v)); }

Vec4 Normalize(const Vec4& v);

inline Vec3 ToVec3(const Vec4& v) { return Vec3{v.x, v.y, v.z}; }
inline Vec4 ToVec4(const Vec3& v, float w = 1.0f) { return Vec4(v.x, v.y, v.z, w); }

template <typename T>
inline float Sum(const _v4<T>& v) { return v.x + v.y + v.z + v.w; }

std::string ToString(const Int4&);
std::string ToString(const Vec4&);

// =================================================================================================
// Matrices
//
// Matrices are implemented as column mayor. This means that a direct iteration over the members
// (for int i = 0; i < 16; i++) will show the transpose matrix if you expected a list of for rows.
//
// This is mainly because OpenGL is column major.
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

  static _mat2 Identity() { return {{1, 0}, {0, 1}}; }

  // Operators

  _v2<T>& operator[](int index) { return cols[index]; }
  const _v2<T>& operator[](int index) const { return cols[index]; }
};

using IntMat2 = _mat2<int>;
using Mat2 = _mat2<float>;

// Mat3 --------------------------------------------------------------------------------------------

template <typename T>
union _mat3 {
  // Members.

  _v3<T> cols[3] = {};
  float elements[3][3];

  // Constructors.

  _mat3() = default;
  _mat3(_v3<T> r0, _v3<T> r1, _v3<T> r2) {
    // As this is column major, each row given becomes each column.
    // clang-format off
    cols[0][0] = r0[0]; cols[1][0] = r0[1]; cols[2][0] = r0[2];
    cols[0][1] = r1[0]; cols[1][1] = r1[1]; cols[2][1] = r1[2];
    cols[0][2] = r2[0]; cols[1][2] = r2[1]; cols[2][2] = r2[2];
    // clang-format on
  }

  static _mat3 Identity() { return {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}; }




  float& get(int x, int y) { return elements[y][x]; }
  float get(int x, int y) const { return elements[y][x]; }

  void set(int x, int y, T val) { elements[y][x] = val; }

  _v3<T> row(int index) const { return {cols[0][index], cols[1][index], cols[2][index]}; }

  // Operators.
  _v3<T> operator*(const _v3<T>& vec) const {
    _v3<T> r0 = row(0); _v3<T> r1 = row(1); _v3<T> r2 = row(2);
    return _v3<T>{Dot(r0, vec), Dot(r1, vec), Dot(r2, vec)};
  }
};

using IntMat3 = _mat3<int>;
using Mat3 = _mat3<float>;

float Determinant(const Mat3&);

// Mat4 --------------------------------------------------------------------------------------------

// clang-format off
template <typename T>
union _mat4 {
  // Members.

  _v4<T> cols[4] = {};
  float elements[4][4];

  // Constructors.

  _mat4() = default;
  _mat4(_v4<T> r0, _v4<T> r1, _v4<T> r2, _v4<T> r3) {
    // As this is column major, each row given becomes each column.
    cols[0][0] = r0[0]; cols[1][0] = r0[1]; cols[2][0] = r0[2]; cols[3][0] = r0[3];
    cols[0][1] = r1[0]; cols[1][1] = r1[1]; cols[2][1] = r1[2]; cols[3][1] = r1[3];
    cols[0][2] = r2[0]; cols[1][2] = r2[1]; cols[2][2] = r2[2]; cols[3][2] = r2[3];
    cols[0][3] = r3[0]; cols[1][3] = r3[1]; cols[2][3] = r3[2]; cols[3][3] = r3[3];
  }

  static _mat4 Identity() { return {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}; }

  // Operators.

  float& get(int x, int y) { return elements[y][x]; }
  float get(int x, int y) const { return elements[y][x]; }

  void set(int x, int y, T val) { elements[y][x] = val; }

  _v4<T> row(int index) const {
    return {cols[0][index], cols[1][index], cols[2][index], cols[3][index]};
  }

  bool operator==(const _mat4& o) const {
    return cols[0] == o.cols[0] && cols[1] == o.cols[1] &&
           cols[2] == o.cols[2] && cols[3] == o.cols[3];
  }

  bool operator!=(const _mat4& o) const { return !(*this == o); }

  // Vec3 appends a 1.0f to the w coordinate.
  _v4<T> operator*(const _v3<T>& v) const {
    _v4<T> vec = {v.x, v.y, v.z, 1.0f};
    return (*this) * vec;
  }

  _v4<T> operator*(const _v4<T>& vec) const {
    _v4<T> r0 = row(0); _v4<T> r1 = row(1); _v4<T> r2 = row(2); _v4<T> r3 = row(3);
    return _v4<T>{Dot(r0, vec), Dot(r1, vec), Dot(r2, vec), Dot(r3, vec)};
  }

  _mat4<T> operator*(const _mat4<T>& m) const {
    _mat4<T> res = {};
    _v4<T> r0 = row(0); _v4<T> r1 = row(1); _v4<T> r2 = row(2); _v4<T> r3 = row(3);
    res.cols[0] = {Dot(r0, m.cols[0]), Dot(r1, m.cols[0]), Dot(r2, m.cols[0]), Dot(r3, m.cols[0])};
    res.cols[1] = {Dot(r0, m.cols[1]), Dot(r1, m.cols[1]), Dot(r2, m.cols[1]), Dot(r3, m.cols[1])};
    res.cols[2] = {Dot(r0, m.cols[2]), Dot(r1, m.cols[2]), Dot(r2, m.cols[2]), Dot(r3, m.cols[2])};
    res.cols[3] = {Dot(r0, m.cols[3]), Dot(r1, m.cols[3]), Dot(r2, m.cols[3]), Dot(r3, m.cols[3])};

    return res;
  }

  void operator*=(const _mat4<T>& m) {
    _v4<T> r0 = row(0); _v4<T> r1 = row(1); _v4<T> r2 = row(2); _v4<T> r3 = row(3);
    cols[0] = {Dot(r0, m.cols[0]), Dot(r1, m.cols[0]), Dot(r2, m.cols[0]), Dot(r3, m.cols[0])};
    cols[1] = {Dot(r0, m.cols[1]), Dot(r1, m.cols[1]), Dot(r2, m.cols[1]), Dot(r3, m.cols[1])};
    cols[2] = {Dot(r0, m.cols[2]), Dot(r1, m.cols[2]), Dot(r2, m.cols[2]), Dot(r3, m.cols[2])};
    cols[3] = {Dot(r0, m.cols[3]), Dot(r1, m.cols[3]), Dot(r2, m.cols[3]), Dot(r3, m.cols[3])};
  };

  _mat4<T> operator*(T t) const {
    _mat4<T> result = *this;
    for (int i = 0; i < 4; i++) {
      result.cols[i] *= t;
    }
    return result;
  };

  void operator*=(T t) {
    for (int i = 0; i < 4; i++) {
      cols[i] *= t;
    }
  }
};
// clang-format on

using IntMat4 = _mat4<int>;
using Mat4 = _mat4<float>;

Mat4 Adjugate(const Mat4&);

float Determinant(const Mat4&);

Mat4 Inverse(const Mat4&);

template <typename T>
void SetRowCol(_mat4<T>* m, T x, T y) { (*m)[y][x]; }

inline Mat3 ToMat3(const Mat4& m) {
  Mat3 result;
  result.cols[0] = ToVec3(m.cols[0]);
  result.cols[1] = ToVec3(m.cols[1]);
  result.cols[2] = ToVec3(m.cols[2]);

  return result;
}

std::string ToString(const Mat4&);

Mat4 Transpose(const Mat4&);

// =================================================================================================
// Transformation Matrices.
// =================================================================================================

// Generates the following matrix:
//
// | -- x -- |
// | -- y -- |
// | -- z -- |
// | 0 0 0 1 |
Mat4 FromRows(Vec3 x, Vec3 y, Vec3 z);
inline Mat4 FromRowsNormalized(Vec3 x, Vec3 y, Vec3 z) {
  return FromRows(Normalize(x), Normalize(y), Normalize(z));
}

// Generates the following matrix:
//
// | | | | 0 |
// | x y z 0 |
// | | | | 0 |
// | | | | 1 |
Mat4 FromColumns(Vec3 x, Vec3 y, Vec3 z);
inline Mat4 FromColumnsNormalized(Vec3 x, Vec3 y, Vec3 z) {
  return FromColumns(Normalize(x), Normalize(y), Normalize(z));
}

Mat4 Translate(const Vec3& v);

// Rotate |radian_angle| around |v|.
Mat4 Rotate(const Vec3& v, float radian_angle);

// If |x| or |y| is zero, no rotation would be made in that axis.
// That means that if no value is given, |pos| would be given just out.
Vec3 Rotate(Vec3 pos, float radian_x, float radian_y);

inline Vec3 RotateX(Vec3 pos, float radian_angle) {
  Vec4 res = Rotate({1, 0, 0}, radian_angle) * pos;
  return {res.x, res.y, res.z};
}

inline Vec3 RotateY(Vec3 pos, float radian_angle) {
  Vec4 res = Rotate({0, 1, 0}, radian_angle) * pos;
  return {res.x, res.y, res.z};
}

// Each coord of |v| represents the scale on that dimenstion.
Mat4 Scale(const Vec3& v);

inline Mat4 Scale(float scale) {
  return Scale({scale, scale, scale});
}

// Usually |hint_up| points upward. Our Y vector represents up.
Mat4 LookAt(Vec3 pos, Vec3 target, Vec3 hint_up = {0, 1, 0});

Mat4 Frustrum(float left, float right, float bottom, float top, float near, float far);

Mat4 Ortho(float left, float right, float bottom, float top);
Mat4 Ortho(float left, float right, float bottom, float top, float near, float far);

// Returns |Frustrum| after calculating the values.
Mat4 Perspective(float fov, float aspect_ratio, float near, float far);

Vec3 PositionFromTransformMatrix(const Mat4&);
Vec3 RotationFromTransformMatrix(const Mat4&);
Vec3 ScaleFromTransformMatrix(const Mat4&);

// Calls the three extractions calls.
void DecomposeTransformMatrix(const Mat4&, Vec3* position, Vec3* rotation, Vec3* scale);

// =================================================================================================
// Frames (axis)
// =================================================================================================

struct AxisFrame {
  Vec3 forward;
  Vec3 up;
  Vec3 right;
};

// Gets the |foward|, |up| and |right| vectors from the given |direction| vector.
// |direction| doesn't need to be normalized. The returned vectors will be normalized.
AxisFrame GetAxisFrame(Vec3 direction);

// =================================================================================================
// Euler Angles
// =================================================================================================

// |pitch| = Radian angle of rotation by the x-axis.
// |yaw| = Randian angle of rotation by the y-axis.
Vec3 DirectionFromEuler(float pitch, float yaw);
inline Vec3 DirectionFromEulerDeg(float pitch_deg, float yaw_deg) {
  return DirectionFromEuler(ToRadians(pitch_deg), ToRadians(yaw_deg));
}

Vec2 EulerFromDirection(const Vec3& direction);
inline Vec2 EulerFromDirectionDeg(const Vec3& direction) {
  Vec2 euler= EulerFromDirection(direction);
  return {ToDegrees(euler.x), ToDegrees(euler.y)};
}

// =================================================================================================
// Quaternion
// =================================================================================================

union Quaternion {
  // Members.

  struct { Vec3 dir; float angle; };
  struct { float x, y, z, w; };
  Vec4 elements;

  // Constructor.

  // clang-format off
  Quaternion() = default;
  Quaternion(const Vec4& v) { elements = v; }
  Quaternion(const Quaternion& q) : elements(q.elements) {}
  Quaternion& operator=(const Quaternion& q) { elements = q.elements; return *this; }
  Quaternion(Quaternion&& q) : elements(q.elements) {}
  Quaternion& operator=(Quaternion&& q) { elements = q.elements; return *this; }
  // clang-format on

  // Operators.

  Quaternion operator+(const Quaternion& q) const { return elements + q.elements; }
  void operator+=(const Quaternion& q) { elements += q.elements; }

  Quaternion operator-(const Quaternion& q) const { return elements - q.elements; }
  void operator-=(const Quaternion& q) { elements -= q.elements; }

  // clang-format off
  Quaternion operator*(const Quaternion& q) const {
    Quaternion res;
    res.x = ( x * q.w) + (y * q.z) - (z * q.y) + (w * q.x);
    res.y = (-x * q.z) + (y * q.w) + (z * q.x) + (w * q.y);
    res.z = ( x * q.y) - (y * q.x) + (z * q.w) + (w * q.z);
    res.w = (-x * q.x) - (y * q.y) - (z * q.z) + (w * q.w);

    return res;
  };

  void operator*=(const Quaternion& q) {
    x = ( x * q.w) + (y * q.z) - (z * q.y) + (w * q.x);
    y = (-x * q.z) + (y * q.w) + (z * q.x) + (w * q.y);
    z = ( x * q.y) - (y * q.x) + (z * q.w) + (w * q.z);
    w = (-x * q.x) - (y * q.y) - (z * q.z) + (w * q.w);
  }
  // clang-format on

  Quaternion operator*(float s) const { return elements * s; };
  void operator*=(float s) { elements * s; }

  Quaternion operator/(float s) const { return elements / s; };
  void operator/=(float s) { elements / s; }
};

inline float Dot(const Quaternion& q1, const Quaternion& q2) {
  return Dot(q1.elements, q2.elements);
}

Quaternion Inverse(const Quaternion&);

Quaternion Normalize(const Quaternion& q);

// |t| is between [0, 1].
// The returned quaternion is normalized.
inline Quaternion NLerp(const Quaternion& q1, const Quaternion& q2, float t) {
  Quaternion res;
  res.x = Lerp(q1.x, q2.x, t);
  res.y = Lerp(q1.y, q2.y, t);
  res.z = Lerp(q1.z, q2.z, t);
  res.w = Lerp(q1.w, q2.w, t);

  return Normalize(res);
}

Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);

Mat4 ToMat4(const Quaternion&);

}  // rothko
