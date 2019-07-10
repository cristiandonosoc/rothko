// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/ui/imgui/imgui_shaders.h"

#include "rothko/graphics/common/shader.h"
#include "rothko/logging/logging.h"

namespace rothko {
namespace imgui {

// =================================================================================================
// OpenGL
// =================================================================================================

#ifdef ROTHKO_OPENGL_ENABLED

const char kOpenGLVertex[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes --------------------------------------------------------------------------------------

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec4 color;
out vec2 uv;

// Uniforms ----------------------------------------------------------------------------------------

layout (std140) uniform Camera {
  mat4 proj;
  mat4 view;
};

void main() {
  gl_Position = proj * view * vec4(in_pos.xy, 0, 1.0f);
  color = in_color;
  uv = in_uv;
}
)";

const char kOpenGLFragment[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes --------------------------------------------------------------------------------------

in vec4 color;
in vec2 uv;

layout (location = 0) out vec4 out_color;

// Uniforms ----------------------------------------------------------------------------------------

uniform sampler2D tex_sampler;

// Code --------------------------------------------------------------------------------------------

void main() {
  out_color = color * texture(tex_sampler, uv);
}

)";

Shader GetOpenGLImguiShader() {
  Shader shader;
  shader.name = "Imgui Shader";
  shader.vert_ubo.name = "Camera";
  shader.vert_ubo.size = 128;
  shader.texture_count = 0;

  shader.vert_src = kOpenGLVertex;
  shader.frag_src = kOpenGLFragment;

  return shader;
}

#else

Shader GetOpenGLImguiShader() {
  NOT_REACHED_MSG("OpenGL support not enabled.");
  return {};
}

#endif

}  // namespace imgui
}  // namespace rothko
