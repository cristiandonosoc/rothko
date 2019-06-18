// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <vector>

#include "rothko/math/vec.h"
#include "rothko/utils/clear_on_move.h"
#include "rothko/utils/macros.h"

namespace rothko {

struct Renderer;

// Vertices types --------------------------------------------------------------

enum class VertexType : uint32_t {
  kDefault,   // VertexDefault.
  kLast,
};

struct VertexDefault {
  static constexpr VertexType kVertexType = VertexType::kDefault;

  Vec3 pos;
  Vec3 normal;
  Vec2 uv;
};

// Mesh ------------------------------------------------------------------------

struct Mesh {
  RAII_CONSTRUCTORS(Mesh);

  Renderer* renderer = nullptr;
  ClearOnMove<uint32_t> uuid = 0;
  VertexType vertex_type = VertexType::kLast;

  std::vector<uint8_t> vertices;
  std::vector<uint8_t> indices;

  uint32_t vertices_count;
  uint32_t indices_count;
};

inline bool Staged(Mesh* m) { return m->uuid.has_value(); }

template <typename VertexType>
void PushVertices(Mesh* mesh, VertexType* data, uint32_t count) {
  ASSERT(mesh->vertex_type == VertexType::kVertexType);

  // Have to be able to hold all the vertices.
  mesh->vertices.reserve(mesh->vertices_count + count);
  for (uint32_t i = 0; i < count; i++) {
    mesh->vertices.emplace_back(*data++);
  }
  mesh->vertices_count += count;
}

}  // namespace rothko
