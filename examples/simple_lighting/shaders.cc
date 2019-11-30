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

struct Light {
  vec3 pos;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

layout (std140) uniform FragUniforms {
  Light light;
  Material material;
};

in vec3 pos;
in vec3 normal;

void main() {
  vec3 unit_normal = normalize(normal);

  // Ambient light.
  vec3 ambient_light = light.ambient * material.ambient * 0.2f;

  // Diffuse light.
  vec3 light_dir = normalize(light.pos - pos);
  float diffuse = max(dot(unit_normal, light_dir), 0);
  vec3 diffuse_light = light.diffuse * diffuse * material.diffuse;

  // Specular light.
  float specular_strength = 0.5f;
  vec3 view_dir = normalize(camera_pos - pos);
  vec3 reflect_dir = reflect(-light_dir, unit_normal);
  float specular = pow(max(dot(view_dir, reflect_dir), 0), material.shininess);
  /* vec3 specular_light = specular * specular_strength * light_color; */
  vec3 specular_light = light.specular * specular * material.specular;

  // Final lighting output.
  vec3 color = ambient_light + diffuse_light + specular_light;
  out_color = vec4(color, 1);
}

)";

}  // namespace

std::unique_ptr<Shader> CreateObjectShader(Renderer* renderer) {
  ShaderConfig config = {};
  config.name = "lighting";
  config.vertex_type = VertexType::k3dNormal;
  config.ubos[0].name = "VertUniforms";
  config.ubos[0].size = sizeof(ObjectShaderUBO::Vert);
  config.ubos[1].name = "FragUniforms";
  config.ubos[1].size = sizeof(ObjectShaderUBO::Frag);

  auto vert_src = CreateVertexSource(kObjectVertShader);
  auto frag_src = CreateFragmentSource(kObjectFragShader);

  return RendererStageShader(renderer, config, vert_src, frag_src);
}

}  // namespace simple_lighting
}  // namespace rothko
