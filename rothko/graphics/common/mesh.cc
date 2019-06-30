// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/mesh.h"

#include "rothko/graphics/common/renderer.h"
#include "rothko/utils/logging.h"

namespace rothko {

Mesh::~Mesh() {
  if (Staged(this))
    RendererUnstageMesh(this->renderer, this);
}

const char* ToString(VertexType type) {
  switch (type) {
    case VertexType::kDefault: return "Default";
    case VertexType::kColor: return "Color";
    case VertexType::kLast: return "Last";
  }

  NOT_REACHED();
  return "<unknown>";
}

}  // namespace rothko
