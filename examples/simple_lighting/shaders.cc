// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shaders.h"

namespace rothko {
namespace simple_lighting {

// Object Shader -----------------------------------------------------------------------------------

namespace {

constexpr char kObjectVertShader[] = R"(

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;

layout (std140) uniform VertUniforms {
  mat4 model;
};

out vec3 pos;
out vec3 normal;

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);

  // We want the frag position in world space, not view space. Only multiply by the model matrix.
  pos = vec3(model * vec4(in_pos, 1));
  normal = in_normal;
}
)";

constexpr char kObjectFragShader[] = R"(

layout (location = 0) out vec4 out_color;

layout (std140) uniform FragUniforms {
  vec3 object_color;
  vec3 light_color;
  vec3 light_pos;
};

in vec3 pos;
in vec3 normal;

void main() {
  // Ambient light.
  float ambient_strength = 0.2f;
  vec3 ambient_light = ambient_strength * light_color;

  // Diffuse light.
  vec3 unit_normal = normalize(normal);
  vec3 light_dir = normalize(light_pos - pos);
  float diffuse = max(dot(unit_normal, light_dir), 0);
  vec3 diffuse_light = diffuse * light_color;

  // Final lighting output.
  vec3 color = (ambient_light + diffuse_light) * object_color;
  out_color = vec4(color, 1);
}

)";

}  // namespace

Shader CreateObjectShader(Renderer* renderer) {
  Shader shader;

  shader.name = "lighting";
  shader.vertex_type = VertexType::k3dNormal;
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


