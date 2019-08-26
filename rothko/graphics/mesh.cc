// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/mesh.h"

#include "rothko/graphics/renderer.h"
#include "rothko/logging/logging.h"

namespace rothko {

Mesh::~Mesh() {
  if (Staged(*this))
    RendererUnstageMesh(this->renderer, this);
}

bool StageWithCapacity(Renderer* renderer, Mesh* mesh, VertexType vertex_type,
                       uint32_t vertex_count, uint32_t index_count) {
  ASSERT(!Staged(*mesh));
  ASSERT(vertex_count > 0);
  ASSERT(index_count > 0);

  Reset(mesh);
  mesh->vertex_type = vertex_type;
  mesh->vertices = std::vector<uint8_t>(ToSize(vertex_type) * vertex_count);
  mesh->indices = std::vector<uint8_t>(sizeof(Mesh::IndexType) * index_count);


  bool staged = RendererStageMesh(renderer, mesh);
  if (!staged)
    return false;

  // We just want the capacity, we don't need the data anymore.
  mesh->vertices.clear();
  mesh->indices.clear();

  return true;
}

void PushIndices(Mesh* mesh, Mesh::IndexType* data, uint32_t count, uint32_t offset) {
  mesh->indices.reserve((mesh->index_count + count) * sizeof(Mesh::IndexType));

  Mesh::IndexType* ptr = data;
  Mesh::IndexType* end = data + count;

  while (ptr != end) {
    Mesh::IndexType val = *ptr + offset;

    // We add each byte of the index.
    uint8_t* tmp = (uint8_t*)&val;
    uint8_t* tmp_end = (uint8_t*)(&val + 1);
    mesh->indices.insert(mesh->indices.end(), tmp, tmp_end);

    ptr++;
  }

  mesh->index_count += count;
}

}  // namespace rothko
