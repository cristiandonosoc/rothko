
// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <vector>

#include "rothko/math/math.h"
#include "rothko/utils/clear_on_move.h"
#include "rothko/logging/logging.h"
#include "rothko/utils/macros.h"

namespace rothko {

struct Renderer;

enum class VertexType : uint32_t {
  kDefault,       // VertexDefault.
  k2dUVColor,     // Vertex2dUvColor.
  k3dUVColor,     // Vertex3dUVColor.
  kLast,
};
const char* ToString(VertexType);
uint32_t ToSize(VertexType);

// Mesh --------------------------------------------------------------------------------------------

struct Mesh {
  using IndexType = uint32_t;

  RAII_CONSTRUCTORS(Mesh);

  std::string name;

  Renderer* renderer = nullptr;
  ClearOnMove<uint32_t> uuid = 0;
  VertexType vertex_type = VertexType::kLast;

  std::vector<uint8_t> vertices;
  std::vector<uint8_t> indices;

  uint32_t vertices_count = 0;
  uint32_t indices_count = 0;
};

inline bool Staged(Mesh* m) { return m->uuid.has_value(); }

inline void Reset(Mesh* mesh) {
  mesh->vertices.clear();
  mesh->vertices_count = 0;
  mesh->indices.clear();
  mesh->indices_count = 0;
}

template <typename VertexType>
void PushVertices(Mesh* mesh, VertexType* data, uint32_t count) {
  ASSERT_MSG(mesh->vertex_type == VertexType::kVertexType,
             "Expected \"%s\", Got \"%s\"",
             ToString(mesh->vertex_type),
             ToString(VertexType::kVertexType));

  // Have to be able to hold all the vertices.
  mesh->vertices.reserve((mesh->vertices_count + count) * ToSize(mesh->vertex_type));

  VertexType* begin = data;
  VertexType* end = data + count;

  mesh->vertices.insert(mesh->vertices.end(), (uint8_t*)begin, (uint8_t*)end);
  mesh->vertices_count += count;
}

// Pushes an array of indices into the mesh.
// The |offset| is a value that will be added to each element.
void PushIndices(Mesh* mesh, Mesh::IndexType* data, uint32_t count, uint32_t offset = 0);

// Vertex Definitions ------------------------------------------------------------------------------

// NOTE: pragma pack(push, <MODE>) pushes into the compiler state the way the compiler should pad
//       in the fields of a struct. Normally the compiler will attempt to pad fields in order to
//       align memory to a particular boundary (normally 4 or 8 bytes).
//
//       pack(push, 1) tells the compiler to not pad at all and leave the memory layout of the
//       struct as it's defined. This normally is more inefficient or error-prone, but for terms of
//       OpenGL, it is something we want in order to tightly pack the buffer sent to the GPU.
#pragma pack(push, 1)

struct VertexDefault {
  static constexpr VertexType kVertexType = VertexType::kDefault;

  Vec3 pos;
  Vec3 normal;
  Vec2 uv;
};
static_assert(sizeof(VertexDefault) == 32);

struct Vertex2dUVColor{
  static constexpr VertexType kVertexType = VertexType::k2dUVColor;

  Vec2 pos;
  Vec2 uv;
  uint32_t color;
};
static_assert(sizeof(Vertex2dUVColor) == 20);

struct Vertex3dUVColor {
  static constexpr VertexType kVertexType = VertexType::k3dUVColor;

  Vec3 pos;
  Vec2 uv;
  uint32_t color;
};
static_assert(sizeof(Vertex3dUVColor) == 24);

#pragma pack(pop)

}  // namespace rothko
