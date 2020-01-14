// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <vector>

#include "rothko/graphics/vertices.h"
#include "rothko/math/math.h"
#include "rothko/utils/clear_on_move.h"
#include "rothko/logging/logging.h"
#include "rothko/utils/macros.h"

namespace rothko {

struct Renderer;

// Mesh --------------------------------------------------------------------------------------------

struct Mesh {
  using IndexType = uint32_t;

  RAII_CONSTRUCTORS(Mesh);

  std::string name;
  uint32_t id = 0;
  uint32_t staged = 0;

  Renderer* renderer = nullptr;
  VertexType vertex_type = VertexType::kLast;

  std::vector<uint8_t> vertices;
  uint32_t vertex_count = 0;

  std::vector<IndexType> indices;
};

bool StageWithCapacity(Renderer*, Mesh*, VertexType, uint32_t vertex_count, uint32_t index_count);
inline bool Staged(const Mesh& mesh) { return mesh.staged != 0; }

inline void Reset(Mesh* mesh) {
  mesh->vertices.clear();
  mesh->vertex_count = 0;

  mesh->indices.clear();
}

template <typename VertexType>
void PushVertices(Mesh* mesh, VertexType* data, uint32_t count) {
  ASSERT_MSG(mesh->vertex_type == VertexType::kVertexType,
             "Expected \"%s\", Got \"%s\"",
             ToString(mesh->vertex_type),
             ToString(VertexType::kVertexType));

  // Have to be able to hold all the vertices.
  mesh->vertices.reserve((mesh->vertex_count + count) * ToSize(mesh->vertex_type));

  VertexType* begin = data;
  VertexType* end = data + count;

  mesh->vertices.insert(mesh->vertices.end(), (uint8_t*)begin, (uint8_t*)end);
  mesh->vertex_count += count;
}

// Pushes an array of indices into the mesh.
// The |offset| is a value that will be added to each element.
inline void
PushIndices(Mesh* mesh, Mesh::IndexType* data, uint32_t count, Mesh::IndexType offset = 0) {
  mesh->indices.reserve(mesh->indices.size() + count);

  for (uint32_t i = 0; i < count; i++) {
    Mesh::IndexType index = *data++ + offset;
    mesh->indices.push_back(index);
  }
}

}  // namespace rothko
