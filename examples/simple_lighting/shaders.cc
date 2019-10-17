// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shaders.h"

namespace rothko {
namespace simple_lighting {

// Object Shader -----------------------------------------------------------------------------------

namespace {

constexpr char kObjectVertShader[] = R"(

layout (location = 0) in vec3 in_pos;

layout (std140) uniform VertUniforms {
  mat4 model;
};

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);
}
)";

constexpr char kObjectFragShader[] = R"(

layout (location = 0) out vec4 out_color;

layout (std140) uniform FragUniforms {
  vec3 object_color;
  vec3 light_color;
};

void main() {
  float ambient_strength = 0.2f;
  vec3 ambient_light = ambient_strength * light_color;

  vec3 color = ambient_light * object_color;
  out_color = vec4(color, 1);
}

)";

}  // namespace

Shader CreateObjectShader(Renderer* renderer) {
  Shader shader;

  shader.name = "lighting";
  shader.vertex_type = VertexType::k3d;
  shader.vert_ubo_name = "VertUniforms";
  shader.vert_ubo_size = sizeof(ObjectShaderUBO::Vert);

  shader.frag_ubo_name = "FragUniforms";
  shader.frag_ubo_size = sizeof(ObjectShaderUBO::Frag);

  shader.vert_src = CreateVertexSource(kObjectVertShader);
  shader.frag_src = CreateFragmentSource(kObjectFragShader);

  if (!RendererStageShader(renderer, &shader))
    return {};
  return shader;
}

// Light Shader ------------------------------------------------------------------------------------

namespace {

constexpr char kLightVertShader[] = R"(

layout (location = 0) in vec3 in_pos;

layout (std140) uniform VertUniforms {
  mat4 model;
};

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);
}

)";

constexpr char kLightFragShader[] = R"(

layout (location = 0) out vec4 out_color;

layout (std140) uniform FragUniforms {
  vec3 light_color;
};

void main() {
  out_color = vec4(light_color, 1);
}

)";


}  // namespace


Shader CreateLightShader(Renderer* renderer) {
  Shader shader;
  shader.name = "lighting";
  shader.vertex_type = VertexType::k3d;
  shader.vert_ubo_name = "VertUniforms";
  shader.vert_ubo_size = sizeof(LightShaderUBO::Vert);

  shader.frag_ubo_name = "FragUniforms";
  shader.frag_ubo_size = sizeof(LightShaderUBO::Frag);

  shader.vert_src = CreateVertexSource(kLightVertShader);
  shader.frag_src = CreateFragmentSource(kLightFragShader);

  if (!RendererStageShader(renderer, &shader))
    return {};
  return shader;
}

}  // namespace simple_lighting
}  // namespace rothko


