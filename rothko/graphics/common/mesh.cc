// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/mesh.h"

#include "rothko/graphics/common/renderer.h"
#include "rothko/logging/logging.h"

namespace rothko {

Mesh::~Mesh() {
  if (Staged(this))
    RendererUnstageMesh(this->renderer, this);
}

const char* ToString(VertexType type) {
  switch (type) {
    case VertexType::kDefault: return "Default";
    case VertexType::kColor: return "Color";
    case VertexType::kImgui: return "Imgui";
    case VertexType::kLast: return "Last";
  }

  NOT_REACHED();
  return "<unknown>";
}

void PushIndices(Mesh* mesh, Mesh::IndexType* data, uint32_t count) {
  mesh->indices.reserve(mesh->indices_count + count);

  Mesh::IndexType* begin = data;
  Mesh::IndexType* end = data + count;

  mesh->indices.insert(mesh->indices.end(), (uint8_t*)begin, (uint8_t*)end);
  mesh->indices_count += count;
}

void PushIndices(Mesh* mesh, Mesh::IndexType* data, uint32_t count, uint32_t offset) {
  mesh->indices.reserve(mesh->indices_count + count);

  Mesh::IndexType* ptr = data;
  Mesh::IndexType* end = data + count;

  while (ptr != end) {
    Mesh::IndexType val = *ptr++ + offset;

    // We add each byte of the index.
    uint8_t* tmp = (uint8_t*)&val;
    for (size_t i = 0; i < sizeof(Mesh::IndexType) / sizeof(uint8_t); i++) {
      mesh->indices.emplace_back(*tmp++);
    }
  }

  mesh->indices_count += count;
}

}  // namespace rothko
