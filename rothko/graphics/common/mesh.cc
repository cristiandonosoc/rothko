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

uint32_t ToSize(VertexType type) {
  switch (type) {
    case VertexType::kDefault: return sizeof(VertexDefault);
    case VertexType::kColor: return sizeof(VertexColor);
    case VertexType::kImgui: return sizeof(VertexImgui);
    case VertexType::kLast: break;
  }

  NOT_REACHED();
  return 0;
}


void PushIndices(Mesh* mesh, Mesh::IndexType* data, uint32_t count, uint32_t offset) {
  mesh->indices.reserve((mesh->indices_count + count) * sizeof(Mesh::IndexType));

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

  mesh->indices_count += count;
}

}  // namespace rothko
