// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/default_shaders/default_shaders.h"

#include "rothko/graphics/renderer.h"
#include "rothko/logging/logging.h"

namespace rothko {

// Vertex3dUVColor ---------------------------------------------------------------------------------

namespace {

constexpr char kVertex3dUVColorVertexShader[] = R"(
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

constexpr char kVertex3dUVColorFragmentShader[] = R"(
in vec2 f_uv;
in vec4 f_color;

layout (location = 0) out vec4 out_color;

uniform sampler2D tex0;
uniform sampler2D tex1;

void main() {
  /* out_color = mix(texture(tex0, f_uv), texture(tex1, f_uv), 0.5f) * f_color; */
  out_color = f_color;
}
)";

std::unique_ptr<Shader> Vertex3dUVColorShader(Renderer* renderer) {
  ShaderConfig config;
  config.name = "3dNormalUV-default";
  config.vertex_type = VertexType::k3dUVColor;
  config.ubos[0].name = "Uniforms";
  config.ubos[0].size = sizeof(Mat4);
  config.texture_count = 2;

  auto vert_src = CreateVertexSource(kVertex3dUVColorVertexShader);
  auto frag_src = CreateFragmentSource(kVertex3dUVColorFragmentShader);

  return RendererStageShader(renderer, config, vert_src, frag_src);
}

}  // namespace

// Vertex3dNormalTangentUV -------------------------------------------------------------------------

namespace {

constexpr char kVertex3dNormalTangentUVVertexShader[] = R"(
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec4 in_tangent;
layout (location = 3) in vec2 in_uv;

out vec2 f_uv;
out vec4 f_color;

layout (std140) uniform Uniforms {
  mat4 model;
};

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);

  f_uv = in_uv;
  f_color = vec4(in_normal, 1);
}
)";

constexpr char kVertex3dNormalTangentUVFragmentShader[] = R"(
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

std::unique_ptr<Shader> Vertex3dNormalTangentUVShader(Renderer* renderer) {
  ShaderConfig config;
  config.name = "3dNormalUV-default";
  config.vertex_type = VertexType::k3dNormalTangentUV;
  config.ubos[0].name = "Uniforms";
  config.ubos[0].size = sizeof(Mat4);
  config.texture_count = 2;

  auto vert_src = CreateVertexSource(kVertex3dNormalTangentUVVertexShader);
  auto frag_src = CreateFragmentSource(kVertex3dNormalTangentUVFragmentShader);

  return RendererStageShader(renderer, config, vert_src, frag_src);
}

}  // namespace

// GetDefaultShader --------------------------------------------------------------------------------

std::unique_ptr<Shader> CreateDefaultShader(Renderer* renderer, VertexType vertex_type) {
  switch (vertex_type) {
    case VertexType::k2dUVColor: return {};
    case VertexType::k3d: return {};
    case VertexType::k3dColor: return {};
    case VertexType::k3dNormal: return {};
    case VertexType::k3dNormalUV: return {};
    case VertexType::k3dUV: return {};
    case VertexType::k3dUVColor: return Vertex3dUVColorShader(renderer);
    case VertexType::k3dNormalTangentUV: return Vertex3dNormalTangentUVShader(renderer);
    case VertexType::kLast: return {};
  }

  NOT_REACHED();
  return {};
}

}  // namespace rothko
