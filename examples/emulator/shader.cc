// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shader.h"

#include <rothko/graphics/graphics.h>

namespace rothko {
namespace emulator {

namespace {

constexpr char kNormalVertex[] = R"(
#version 330
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec2 uv;
out vec4 color;

uniform mat4 proj;
uniform mat4 view;


void main() {
  gl_Position = proj * view * vec4(in_pos.xy, 0, 1.0f);
  uv = in_uv;
  color = in_color;
}
)";

constexpr char kNormalFrag[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

in vec4 color;
in vec2 uv;

uniform sampler2D tex_sampler;

layout (location = 0) out vec4 out_color;

void main() {
  out_color = color * texture(tex_sampler, uv);
}
)";

}  // namespace

std::unique_ptr<Shader> CreateNormalShader(Renderer* renderer) {
  ShaderConfig config = {};
  config.name = "normal-shader";
  config.vertex_type = VertexType::k3dUVColor;
  config.texture_count = 1;

  auto vert_src = kNormalVertex;
  auto frag_src = kNormalFrag;

  return RendererStageShader(renderer, config, vert_src, frag_src);
}

}  // namespace emulator
}  // namespace rothko
