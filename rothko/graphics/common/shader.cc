// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/shader.h"

#include "rothko/graphics/common/renderer.h"

namespace rothko {

Shader::~Shader() {
  if (Staged(this))
    RendererUnstageShader(this->renderer, this);
}

}  // namespace rothko
