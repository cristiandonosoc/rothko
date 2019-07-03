// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <vector>

#include "rothko/math/math.h"
#include "rothko/utils/clear_on_move.h"
#include "rothko/utils/logging.h"
#include "rothko/utils/macros.h"

namespace rothko {

struct Renderer;

enum class VertexType : uint32_t {
  kDefault,   // VertexDefault.
  kColor,     // VertexColor.
  kLast,
};
const char* ToString(VertexType);

// Mesh ------------------------------------------------------------------------

struct Mesh {
  RAII_CONSTRUCTORS(Mesh);

  using IndexType = uint32_t;

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

template <typename VertexType>
void PushVertices(Mesh* mesh, VertexType* data, uint32_t count) {
  ASSERT(mesh->vertex_type == VertexType::kVertexType);

  // Have to be able to hold all the vertices.
  mesh->vertices.reserve(mesh->vertices_count + count);

  VertexType* begin = data;
  VertexType* end = data + count;

  mesh->vertices.insert(mesh->vertices.end(), (uint8_t*)begin, (uint8_t*)end);
  mesh->vertices_count += count;
}

inline void PushIndices(Mesh* mesh, Mesh::IndexType* data, uint32_t count) {
  mesh->indices.reserve(mesh->indices_count + count);

  Mesh::IndexType* begin = data;
  Mesh::IndexType* end = data + count;

  mesh->indices.insert(mesh->indices.end(), (uint8_t*)begin, (uint8_t*)end);
  mesh->indices_count += count;
}

// Vertex Definitions ----------------------------------------------------------

struct VertexDefault {
  static constexpr VertexType kVertexType = VertexType::kDefault;

  Vec3 pos;
  Vec3 normal;
  Vec2 uv;
};

struct VertexColor {
  static constexpr VertexType kVertexType = VertexType::kColor;

  Vec3 pos;
  uint32_t color;
};

}  // namespace rothko
