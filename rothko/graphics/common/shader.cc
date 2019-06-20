// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/shader.h"

#include <map>
#include <optional>

#include "rothko/graphics/common/renderer.h"
#include "rothko/utils/logging.h"
#include "rothko/utils/strings.h"

namespace rothko {

Shader::~Shader() {
  if (Staged(*this))
    RendererUnstageShader(this->renderer, this);
}

}  // namespace rothko
