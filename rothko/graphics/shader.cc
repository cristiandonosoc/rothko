// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/shader.h"

#include <map>
#include <optional>

#include "rothko/graphics/renderer.h"
#include "rothko/logging/logging.h"
#include "rothko/utils/strings.h"
#include "rothko/utils/file.h"

namespace rothko {

Shader::~Shader() {
  if (Staged(*this))
    RendererUnstageShader(this->renderer, this);
}

bool LoadShaderSources(const std::string& vert_path,
                       const std::string& frag_path,
                       Shader* out) {
  if (!ReadWholeFile(vert_path, &out->vert_src) ||
      !ReadWholeFile(frag_path, &out->frag_src)) {
    return false;
  }
  return true;
}

void RemoveSources(Shader* shader) {
  shader->vert_src.clear();
  shader->frag_src.clear();
}

}  // namespace rothko
