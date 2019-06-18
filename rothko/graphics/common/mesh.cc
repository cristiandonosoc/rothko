// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/mesh.h"

#include "rothko/graphics/common/renderer.h"

namespace rothko {

Mesh::~Mesh() {
  if (Staged(this))
    RendererUnstageMesh(this->renderer, this);
}

}  // namespace rothko

