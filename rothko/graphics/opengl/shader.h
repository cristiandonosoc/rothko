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

struct UBOBinding {
  int binding_index = -1;
  uint32_t buffer_handle = 0;
};

struct ShaderHandles {
  uint32_t program;
  std::vector<UBOBinding> vert_ubos;
  std::vector<UBOBinding> frag_ubos;
};

bool OpenGLStageShader(OpenGLRendererBackend*, Shader*);
void OpenGLUnstageShader(OpenGLRendererBackend*, Shader*);

}  // namespace opengl
}  // namespace rothko
