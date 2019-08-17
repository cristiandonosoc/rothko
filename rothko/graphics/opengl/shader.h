// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

#include "rothko/graphics/shader.h"

namespace rothko {

struct Shader;

namespace opengl {

struct OpenGLRendererBackend;

struct ShaderHandles {
  static constexpr int kMaxTextures = 4;

  // The Uniform Buffer Object binding.
  struct UBO {
    int binding_index = -1;
    uint32_t buffer_handle = 0;
  };

  uint32_t program = 0;

  // We expect shaders to have a |proj| and |view| mat4 uniforms.
  int camera_pos_location = -1;     // Optional.
  int camera_proj_location = -1;
  int camera_view_location = -1;

  UBO vert_ubo = {};
  UBO frag_ubo = {};

  int texture_handles[kMaxTextures] = {};
};

bool OpenGLStageShader(OpenGLRendererBackend*, Shader*);
void OpenGLUnstageShader(OpenGLRendererBackend*, Shader*);

}  // namespace opengl
}  // namespace rothko
