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
  mesh->vertices.resize(ToSize(vertex_type) * vertex_count);

  mesh->indices.resize(index_count);

  bool staged = RendererStageMesh(renderer, mesh);
  if (!staged)
    return false;

  // We just want the capacity, we don't need the data anymore.
  mesh->vertices.clear();
  mesh->indices.clear();

  return true;
}

}  // namespace rothko
