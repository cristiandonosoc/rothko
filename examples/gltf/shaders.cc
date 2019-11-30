// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shaders.h"

#include <rothko/graphics/graphics.h>

namespace rothko {
namespace gltf {

namespace {

constexpr char kModelVertShader[] = R"(
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec4 in_tangent;
layout (location = 3) in vec2 in_uv;

out vec2 f_uv;
out vec4 f_color;

layout (std140) uniform Model {
  mat4 transform;
} model;

layout (std140) uniform Node {
  mat4 transform;
} node;

void main() {
  gl_Position = camera_proj * camera_view * model.transform * node.transform * vec4(in_pos, 1.0);

  f_uv = in_uv;
  f_color = vec4(in_normal, 1);
}
)";

constexpr char kModelFragShader[] = R"(
in vec2 f_uv;
in vec4 f_color;

layout (location = 0) out vec4 out_color;

uniform sampler2D tex0;
/* uniform sampler2D tex1; */

void main() {
  out_color = texture(tex0, f_uv);
  /* out_color = mix(texture(tex0, f_uv), texture(tex1, f_uv), 0.5f) * f_color; */
  /* out_color = f_color; */
}
)";

}  // namespace

std::unique_ptr<Shader> CreateModelShader(Renderer* renderer) {
  ShaderConfig config = {};
  config.name = "model";
  config.vertex_type = VertexType::k3dNormalTangentUV;
  config.ubos[0].name = "Model";
  config.ubos[0].size = sizeof(ModelUBO::Model);
  config.ubos[1].name = "Node";
  config.ubos[1].size = sizeof(ModelUBO::Node);
  /* config.frag_ubo_name = "FragUniforms"; */
  /* config.frag_ubo_size = sizeof(FullLightUBO::Frag); */
  config.texture_count = 2;

  auto vert_src = CreateVertexSource(kModelVertShader);
  auto frag_src = CreateFragmentSource(kModelFragShader);

  return RendererStageShader(renderer, config, vert_src, frag_src);
}

}  // namespace gltf
}  // namespace rothko

