// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shader.h"

#include <rothko/graphics/renderer.h>

using namespace rothko;

namespace {

constexpr char kVertexShader[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec2 f_uv;
out vec4 f_color;

// Camera uniforms.
uniform mat4 proj;
uniform mat4 view;

layout (std140) uniform Uniforms {
  mat4 model;
};

void main() {
  gl_Position = proj * view * model * vec4(in_pos, 1.0);
  f_uv = in_uv;
  f_color = in_color;
}
)";

constexpr char kFragmentShader[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

in vec2 f_uv;
in vec4 f_color;

layout (location = 0) out vec4 out_color;

uniform sampler2D tex0;
uniform sampler2D tex1;

void main() {
  out_color = mix(texture(tex0, f_uv), texture(tex1, f_uv), 0.5f) * f_color;
}
)";

}  // namespace


std::unique_ptr<Shader> CreateShader(Renderer* renderer) {
  auto shader = std::make_unique<Shader>();
  shader->name = "cube-shader";
  shader->vert_ubo_name = "Uniforms";
  shader->vert_ubo_size = sizeof(UBO);
  shader->texture_count = 2;

  shader->vert_src = kVertexShader;
  shader->frag_src = kFragmentShader;

  if (!RendererStageShader(renderer, shader.get()))
    return nullptr;
  return shader;
}

// Grid Shader -------------------------------------------------------------------------------------

namespace {

constexpr char kGridVertexShader[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec2 f_uv;
out vec4 f_color;

out vec3 f_pos;

// Camera uniforms.
uniform mat4 proj;
uniform mat4 view;

void main() {
  gl_Position = proj * view * vec4(in_pos, 1.0);
  f_uv = in_uv;
  f_color = in_color;
  f_pos = gl_Position.xyz;
}
)";

constexpr char kGridFragmentShader[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

in vec2 f_uv;
in vec4 f_color;
in vec3 f_pos;

layout (location = 0) out vec4 out_color;


const float one_width = 0.01f;

void main() {
  if ((mod(f_pos.x, 1.0f) < one_width) || (mod(f_pos.z, 1.0f) < one_width)) {
    out_color = vec4(0, 0, 0, 1);
  } else {
    discard;
  }

  /* out_color = vec4(mod(f_pos.x, 1.0f), 0, 0, 1.0f); */
  /* out_color = vec4(f_pos.x - 15.0f, 0, 0, 1.0f); */
}
)";

}  // namespace

std::unique_ptr<Shader> CreateGridShader(Renderer* renderer) {
  auto shader = std::make_unique<Shader>();
  shader->name = "grid-shader";

  shader->vert_src = kGridVertexShader;
  shader->frag_src = kGridFragmentShader;

  if (!RendererStageShader(renderer, shader.get()))
    return nullptr;
  return shader;
}
