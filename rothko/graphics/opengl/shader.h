// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

#include "rothko/graphics/common/shader.h"

namespace rothko {

struct Shader;

namespace opengl {

struct OpenGLRendererBackend;

struct ShaderHandles {
  // The Uniform Buffer Object binding.
  struct UBO {
    int binding_index = -1;
    uint32_t buffer_handle = 0;
  };

  uint32_t program = 0;
  UBO vert_ubo = {};
  UBO frag_ubo = {};
};

bool OpenGLStageShader(OpenGLRendererBackend*, Shader*);
void OpenGLUnstageShader(OpenGLRendererBackend*, Shader*);

}  // namespace opengl
}  // namespace rothko
