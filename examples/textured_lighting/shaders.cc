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
layout (location = 2) in vec2 in_uv;

layout (std140) uniform VertUniforms {
  mat4 model;
};

out vec3 pos;
out vec3 normal;
out vec2 uv;

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);

  // We want the frag position in world space, not view space. Only multiply by the model matrix.
  pos = vec3(model * vec4(in_pos, 1));
  normal = in_normal;
  uv = in_uv;
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


uniform sampler2D tex0;           // diffuse map.
uniform sampler2D tex1;

struct Material {
  vec3 specular;
  float shininess;
};

layout (std140) uniform FragUniforms {
  Light light;
  Material material;
};

in vec3 pos;
in vec3 normal;
in vec2 uv;

void main() {
  vec3 unit_normal = normalize(normal);

  // Ambient light.
  /* vec3 ambient_light = light.ambient * material.ambient; */
  vec3 ambient_light = light.ambient * vec3(texture(tex0, uv));

  // Diffuse light.
  vec3 light_dir = normalize(light.pos - pos);
  float diffuse = max(dot(unit_normal, light_dir), 0);
  /* vec3 diffuse_light = light.diffuse * diffuse * diffuse; */
  vec3 diffuse_light = light.diffuse * diffuse * vec3(texture(tex0, uv));

  // Specular light.
  float specular_strength = 0.5f;
  vec3 view_dir = normalize(camera_pos - pos);
  vec3 reflect_dir = reflect(-light_dir, unit_normal);
  float specular = pow(max(dot(view_dir, reflect_dir), 0), material.shininess);
  /* vec3 specular_light = light.specular * specular * material.specular; */
  vec3 specular_light = light.specular * specular * vec3(texture(tex1, uv));

  // Final lighting output.
  vec3 color = ambient_light + diffuse_light + specular_light;
  out_color = vec4(color, 1);
}

)";

}  // namespace

Shader CreateObjectShader(Renderer* renderer) {
  Shader shader;

  shader.name = "lighting";
  shader.vertex_type = VertexType::k3dNormalUV;
  shader.vert_ubo_name = "VertUniforms";
  shader.vert_ubo_size = sizeof(ObjectShaderUBO::Vert);

  shader.frag_ubo_name = "FragUniforms";
  shader.frag_ubo_size = sizeof(ObjectShaderUBO::Frag);

  shader.texture_count = 2;

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


