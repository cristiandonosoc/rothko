// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "rothko/math/math.h"

namespace rothko {

// Detection occurs according to bits.
constexpr uint32_t kVertCompPos2d   = (1 << 1);             // Vec2
constexpr uint32_t kVertCompPos3d   = (1 << 2);             // Vec3

constexpr uint32_t kVertCompNormal  = (1 << 4);             // Vec3
constexpr uint32_t kVertCompTangent = (1 << 5);             // Vec4

constexpr uint32_t kVertCompUV0_byte   = (1 << 8);          // uint8_t[2] / uint16_t
constexpr uint32_t kVertCompUV0_short  = (1 << 9);          // uint16_t[2] / uint32_t
constexpr uint32_t kVertCompUV0_float  = (1 << 10);         // Vec2

constexpr uint32_t kVertCompUV1_byte   = (1 << 11);         // uint8_t[2] / uint16_t
constexpr uint32_t kVertCompUV1_short  = (1 << 12);         // uint16_t[2] / uint32_t
constexpr uint32_t kVertCompUV1_float  = (1 << 13);         // Vec2

constexpr uint32_t kVertCompColorRGB_byte   = (1 << 14);    // uint8_t[3]
constexpr uint32_t kVertCompColorRGB_short  = (1 << 15);    // uint16_t[3]
constexpr uint32_t kVertCompColorRGB_float  = (1 << 16);    // Vec3

constexpr uint32_t kVertCompColorRGBA_byte   = (1 << 17);   // uint8_t[4] / uint32_t
constexpr uint32_t kVertCompColorRGBA_short  = (1 << 18);   // uint16_t[4] / uint64_t
constexpr uint32_t kVertCompColorRGBA_float  = (1 << 19);   // Vec4

constexpr uint32_t kVertCompJoints_byte = (1 << 20);        // uint8_t[4] / uint32_t
constexpr uint32_t kVertCompJoints_short = (1 << 21);       // uint16_t[4] / uint64_t

constexpr uint32_t kVertCompWeights_byte   = (1 << 22);     // uint8_t[4] / uint32_t
constexpr uint32_t kVertCompWeights_short  = (1 << 23);     // uint16_t[4] / uint64_t
constexpr uint32_t kVertCompWeights_float  = (1 << 24);     // Vec4

const char* VertCompToString(uint32_t);

enum class VertexType : uint32_t {
  k2dUVColor = kVertCompPos2d | kVertCompUV0_float | kVertCompColorRGBA_byte,
  k3dColor = kVertCompPos3d | kVertCompColorRGBA_byte,
  k3dNormalUV = kVertCompPos3d | kVertCompNormal | kVertCompUV0_float,
  k3dUV = kVertCompPos3d | kVertCompUV0_float,
  k3dUVColor = kVertCompPos3d | kVertCompUV0_float | kVertCompColorRGBA_byte,
  k3dNormalTangentUV = kVertCompPos3d | kVertCompNormal | kVertCompTangent | kVertCompUV0_float,
  kLast = (uint32_t)-1,
};
VertexType ToVertexType(uint32_t);
const char* ToString(VertexType);
uint32_t ToSize(VertexType);

// Vertex Definitions ==============================================================================

// NOTE: pragma pack(push, <MODE>) pushes into the compiler state the way the compiler should pad
//       in the fields of a struct. Normally the compiler will attempt to pad fields in order to
//       align memory to a particular boundary (normally 4 or 8 bytes).
//
//       pack(push, 1) tells the compiler to not pad at all and leave the memory layout of the
//       struct as it's defined. This normally is more inefficient or error-prone, but for terms of
//       OpenGL, it is something we want in order to tightly pack the buffer sent to the GPU.
#pragma pack(push, 1)

struct Vertex2dUVColor{
  static constexpr VertexType kVertexType = VertexType::k2dUVColor;

  Vec2 pos;
  Vec2 uv;
  uint32_t color;
};
static_assert(sizeof(Vertex2dUVColor) == 20);

struct Vertex3dNormalUV {
  static constexpr VertexType kVertexType = VertexType::k3dNormalUV;

  Vec3 pos;
  Vec3 normal;
  Vec2 uv;
};
static_assert(sizeof(Vertex3dNormalUV) == 32);

struct Vertex3dColor {
  static constexpr VertexType kVertexType = VertexType::k3dColor;

  Vec3 pos;
  uint32_t color;
};
static_assert(sizeof(Vertex3dColor) == 16);

struct Vertex3dUV {
  static constexpr VertexType kVertexType = VertexType::k3dUV;

  Vec3 pos;
  Vec2 uv;
};
static_assert(sizeof(Vertex3dUV) == 20);

struct Vertex3dUVColor {
  static constexpr VertexType kVertexType = VertexType::k3dUVColor;

  Vec3 pos;
  Vec2 uv;
  uint32_t color;
};
static_assert(sizeof(Vertex3dUVColor) == 24);

struct Vertex3dNormalTangentUV {
  static constexpr VertexType kVertexType = VertexType::k3dNormalTangentUV;

  Vec3 pos;
  Vec3 normal;
  Vec4 tangent;
  Vec2 uv;
};
static_assert(sizeof(Vertex3dNormalTangentUV) == 12 * sizeof(float));

#pragma pack(pop)




}  // namespace rothko
