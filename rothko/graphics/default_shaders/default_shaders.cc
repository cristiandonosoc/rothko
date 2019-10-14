// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/default_shaders/default_shaders.h"

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

Shader Vertex3dUVColorShader() {
  Shader shader;
  shader.name = "3dNormalUV-default";
  shader.vertex_type = VertexType::k3dUVColor;
  shader.vert_ubo_name = "Uniforms";
  shader.vert_ubo_size = sizeof(Mat4);
  shader.texture_count = 2;

  shader.vert_src = CreateVertexSource(kVertex3dUVColorVertexShader);
  shader.frag_src = CreateFragmentSource(kVertex3dUVColorFragmentShader);

  return shader;
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
uniform sampler2D tex1;

void main() {
  /* out_color = mix(texture(tex0, f_uv), texture(tex1, f_uv), 0.5f) * f_color; */
  out_color = f_color;
}
)";

Shader Vertex3dNormalTangentUVShader() {
  Shader shader;
  shader.name = "3dNormalUV-default";
  shader.vertex_type = VertexType::k3dNormalTangentUV;
  shader.vert_ubo_name = "Uniforms";
  shader.vert_ubo_size = sizeof(Mat4);
  shader.texture_count = 2;

  shader.vert_src = CreateVertexSource(kVertex3dNormalTangentUVVertexShader);
  shader.frag_src = CreateFragmentSource(kVertex3dNormalTangentUVFragmentShader);

  return shader;
}

}  // namespace

// GetDefaultShader --------------------------------------------------------------------------------

Shader CreateDefaultShader(VertexType vertex_type) {
  switch (vertex_type) {
    case VertexType::k2dUVColor: return {};
    case VertexType::k3dColor: return {};
    case VertexType::k3dNormalUV: return {};
    case VertexType::k3dUV: return {};
    case VertexType::k3dUVColor: return Vertex3dUVColorShader();
    case VertexType::k3dNormalTangentUV: return Vertex3dNormalTangentUVShader();
    case VertexType::kLast: return {};
  }

  NOT_REACHED();
  return {};
}

}  // namespace rothko
