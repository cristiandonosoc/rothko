// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shader.h"

#include <rothko/graphics/renderer.h>
#include <rothko/graphics/shader.h>

using namespace rothko;

namespace {

constexpr char kVertexShader[] = R"(
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec2 f_uv;
out vec4 f_color;

layout (std140) uniform Uniforms {
  mat4 model;
};

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);
  f_uv = in_uv;
  f_color = in_color;
}
)";

constexpr char kFragmentShader[] = R"(
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
  ShaderConfig config = {};
  config.name = "cube-shader";
  config.vertex_type = VertexType::k3dUVColor;
  config.vert_ubo_name = "Uniforms";
  config.vert_ubo_size = sizeof(UBO);
  config.texture_count = 2;

  auto vert_src = CreateVertexSource(kVertexShader);
  auto frag_src = CreateFragmentSource(kFragmentShader);

  return RendererStageShader(renderer, config, vert_src, frag_src);
}
