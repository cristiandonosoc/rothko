// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "rothko/math/math.h"

namespace rothko {

enum class VertexType : uint32_t {
  k2dUVColor,
  k3dColor,
  k3dNormalUV,
  k3dUV,
  k3dUVColor,
  kLast,
};
const char* ToString(VertexType);
uint32_t ToSize(VertexType);

// Vertex Definitions ------------------------------------------------------------------------------

// NOTE: pragma pack(push, <MODE>) pushes into the compiler state the way the compiler should pad
//       in the fields of a struct. Normally the compiler will attempt to pad fields in order to
//       align memory to a particular boundary (normally 4 or 8 bytes).
//
//       pack(push, 1) tells the compiler to not pad at all and leave the memory layout of the
//       struct as it's defined. This normally is more inefficient or error-prone, but for terms of
//       OpenGL, it is something we want in order to tightly pack the buffer sent to the GPU.
#pragma pack(push, 1)

struct Vertex3DNormalUV {
  static constexpr VertexType kVertexType = VertexType::k3dNormalUV;

  Vec3 pos;
  Vec3 normal;
  Vec2 uv;
};
static_assert(sizeof(Vertex3DNormalUV) == 32);

struct Vertex2dUVColor{
  static constexpr VertexType kVertexType = VertexType::k2dUVColor;

  Vec2 pos;
  Vec2 uv;
  uint32_t color;
};
static_assert(sizeof(Vertex2dUVColor) == 20);

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

#pragma pack(pop)




}  // namespace rothko
