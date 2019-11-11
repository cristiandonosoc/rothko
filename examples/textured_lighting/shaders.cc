// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shaders.h"

namespace rothko {
namespace textured_lighting {

// Light Shader -----------------------------------------------------------------------------------

namespace {

constexpr char kLightVertShader[] = R"(

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

layout (std140) uniform VertUniforms {
  mat4 model;
  mat4 normal_matrix;
};

out vec3 pos;
out vec3 normal;
out vec2 uv;

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);

  // We want the frag position in world space, not view space. Only multiply by the model matrix.
  pos = vec3(model * vec4(in_pos, 1));
  uv = in_uv;

  // Normals have to take into account the model transformation.
  // We use a normal matrix because non-uniform scale will distort the normal direction.
  normal = mat3(normal_matrix) * in_normal;
}
)";

constexpr char kLightFragShader[] = R"(

layout (location = 0) out vec4 out_color;

struct Light {
  vec4 pos;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

uniform sampler2D tex0;   // diffuse map.
uniform sampler2D tex1;   // Specular map.

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

bool IsZero(vec3 v) {
  return v.r == 0.0f && v.g == 0.0f && v.b == 0.0f;
}

void main() {
  vec3 unit_normal = normalize(normal);

  // Depending on the type of light position, we know whether this is a directional light or not.
  vec3 light_dir;
  float attenuation = 1.0f;   // By default, directional light have no attenuation.
  if (light.pos.w == 0.0f) {
    // Directional light.
    light_dir = normalize(-vec3(light.pos));
  } else {
    light_dir = normalize(vec3(light.pos) - pos);
    float d = length(vec3(light.pos) - pos);
    attenuation = 1.0f / (light.constant + light.linear * d + light.quadratic * d * d);
  }

  // Ambient light.
  vec3 ambient_light = light.ambient * vec3(texture(tex0, uv));

  // Diffuse light.
  float diffuse = max(dot(unit_normal, light_dir), 0);
  vec3 diffuse_color = vec3(texture(tex0, uv));
  if (IsZero(diffuse_color))
    diffuse_color = vec3(0.44f, 0.55f, 0.22f);
  vec3 diffuse_light = light.diffuse * diffuse * diffuse_color;

  // Specular light.
  float specular_strength = 0.5f;
  vec3 view_dir = normalize(camera_pos - pos);
  vec3 reflect_dir = reflect(-light_dir, unit_normal);
  float specular = pow(max(dot(view_dir, reflect_dir), 0), material.shininess);
  vec3 specular_light = light.specular * specular * vec3(texture(tex1, uv));

  // Final lighting output.
  ambient_light *= attenuation;
  diffuse_light *= attenuation;
  specular_light *= attenuation;

  vec3 color = ambient_light + diffuse_light + specular_light;
  out_color = vec4(color, 1);
}

)";

}  // namespace

Shader CreateLightShader(Renderer* renderer) {
  Shader shader;

  shader.name = "point-dir-lighting";
  shader.vertex_type = VertexType::k3dNormalUV;
  shader.vert_ubo_name = "VertUniforms";
  shader.vert_ubo_size = sizeof(LightShaderUBO::Vert);

  shader.frag_ubo_name = "FragUniforms";
  shader.frag_ubo_size = sizeof(LightShaderUBO::Frag);

  shader.texture_count = 2;

  shader.vert_src = CreateVertexSource(kLightVertShader);
  shader.frag_src = CreateFragmentSource(kLightFragShader);

  if (!RendererStageShader(renderer, &shader))
    return {};
  return shader;
}

// Spot Light Shader -------------------------------------------------------------------------------

namespace {

constexpr char kSpotLightVertShader[] = R"(

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

layout (std140) uniform VertUniforms {
  mat4 model;
  mat4 normal_matrix;
};

out vec3 pos;
out vec3 normal;
out vec2 uv;

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);

  // We want the frag position in world space, not view space. Only multiply by the model matrix.
  pos = vec3(model * vec4(in_pos, 1));
  uv = in_uv;

  // Normals have to take into account the model transformation.
  // We use a normal matrix because non-uniform scale will distort the normal direction.
  normal = mat3(normal_matrix) * in_normal;
}
)";

constexpr char kSpotLightFragShader[] = R"(

layout (location = 0) out vec4 out_color;

struct Light {
  vec3 pos;
  vec3 direction;
  float cutoff_cos;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform sampler2D tex0;   // diffuse map.
uniform sampler2D tex1;   // Specular map.

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

bool IsZero(vec3 v) {
  return v.r == 0.0f && v.g == 0.0f && v.b == 0.0f;
}

void main() {
  vec3 light_dir = normalize(vec3(light.pos) - pos);

  // Angle between |light_dir| and the spot light center direction.
  float theta = dot(light_dir, normalize(-light.direction));
  if (theta < light.cutoff_cos) {
    // Use ambient light so that scene isn't completely dark.
    vec3 color = light.ambient * vec3(texture(tex0, uv));
    out_color = vec4(color, 1);
    return;
  }

  light_dir = theta * light_dir;
  vec3 unit_normal = normalize(normal);

  // Ambient light.
  vec3 ambient_light = light.ambient * vec3(texture(tex0, uv));

  // Diffuse light.
  float diffuse = max(dot(unit_normal, light_dir), 0);
  vec3 diffuse_color = vec3(texture(tex0, uv));
  if (IsZero(diffuse_color))
    diffuse_color = vec3(0.44f, 0.55f, 0.22f);
  vec3 diffuse_light = light.diffuse * diffuse * diffuse_color;

  // Specular light.
  float specular_strength = 0.5f;
  vec3 view_dir = normalize(camera_pos - pos);
  vec3 reflect_dir = reflect(-light_dir, unit_normal);
  float specular = pow(max(dot(view_dir, reflect_dir), 0), material.shininess);
  vec3 specular_light = light.specular * specular * vec3(texture(tex1, uv));

  vec3 color = ambient_light + diffuse_light + specular_light;
  out_color = vec4(color, 1);
}

)";

}  // namespace

Shader CreateSpotLightShader(Renderer* renderer) {
  Shader shader;

  shader.name = "spot-lighting";
  shader.vertex_type = VertexType::k3dNormalUV;
  shader.vert_ubo_name = "VertUniforms";
  shader.vert_ubo_size = sizeof(SpotLightShaderUBO::Vert);

  shader.frag_ubo_name = "FragUniforms";
  shader.frag_ubo_size = sizeof(SpotLightShaderUBO::Frag);

  shader.texture_count = 2;

  shader.vert_src = CreateVertexSource(kSpotLightVertShader);
  shader.frag_src = CreateFragmentSource(kSpotLightFragShader);

  if (!RendererStageShader(renderer, &shader))
    return {};
  return shader;
}
// Full Light Shader -------------------------------------------------------------------------------

namespace {

constexpr char kFullLightVertShader[] = R"(

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

layout (std140) uniform VertUniforms {
  mat4 model;
  mat4 normal_matrix;
};

out vec3 f_pos;
out vec3 f_normal;
out vec2 f_uv;

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);

  // We want the frag position in world space, not view space. Only multiply by the model matrix.
  f_pos = vec3(model * vec4(in_pos, 1));
  f_uv = in_uv;

  // Normals have to take into account the model transformation.
  // We use a normal matrix because non-uniform scale will distort the normal direction.
  f_normal = mat3(normal_matrix) * in_normal;
}

)";

constexpr char kFullLightFragShader[] = R"(

layout (location = 0) out vec4 out_color;

in vec3 f_pos;
in vec3 f_normal;
in vec2 f_uv;

// ***** Structs *****

struct LightProperties {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct DirectionalLight {
  vec3 direction;

  LightProperties properties;
};

struct PointLight {
  vec3 position;

  LightProperties properties;

  float constant;
  float linear;
  float quadratic;
};

struct Material {
  vec3 specular;
  float shininess;
};

// ***** Uniforms *****

#define NUM_POINT_LIGHTS 4

layout (std140) uniform FragUniforms {
  Material material;

  DirectionalLight dir_light;
  PointLight point_lights[NUM_POINT_LIGHTS];
} uniforms;

uniform sampler2D f_tex0;   // diffuse map.
uniform sampler2D f_tex1;   // Specular map.

// ***** Functions *****

vec3 CalculateLightColor(Material material, LightProperties light, vec3 light_dir, vec3 normal,
                         vec3 view_dir) {
  // Ambient light.
  vec3 ambient_light = light.ambient * vec3(texture(f_tex0, f_uv));

  // Diffuse light.
  float diffuse = max(dot(normal, light_dir), 0);
  vec3 diffuse_color = vec3(texture(f_tex0, f_uv));
  vec3 diffuse_light = light.diffuse * diffuse * diffuse_color;

  // Specular light.
  float specular_strength = 0.5f;
  vec3 reflect_dir = reflect(-light_dir, normal);
  float specular = pow(max(dot(view_dir, reflect_dir), 0), material.shininess);
  vec3 specular_light = light.specular * specular * vec3(texture(f_tex1, f_uv));

  // Final lighting output.
  return ambient_light + diffuse_light + specular_light;
}

vec3 CalculateDirectionalLight(Material material, DirectionalLight light, vec3 normal,
                               vec3 view_dir) {
  return CalculateLightColor(material, light.properties, light.direction, normal, view_dir);
}

vec3 CalculatePointLight(Material material, PointLight light, vec3 normal, vec3 view_dir) {
  vec3 light_dir = normalize(vec3(light.position) - f_pos);

  float d = length(vec3(light.position) - f_pos);
  float attenuation = 1.0f / (light.constant + light.linear * d + light.quadratic * d * d);

  vec3 color = CalculateLightColor(material, light.properties, light_dir, normal, view_dir);

  color *= attenuation;

  return color;
}

// ***** Main ******

void main() {
  vec3 output = vec3(0);

  vec3 view_dir = normalize(camera_pos - f_pos);

  output += CalculateDirectionalLight(uniforms.material, uniforms.dir_light, f_normal, view_dir);

  for (int i = 0; i < NUM_POINT_LIGHTS; i++) {
    output += CalculatePointLight(uniforms.material, uniforms.point_lights[i], f_normal, view_dir);
  }

  // output += CalculateSpotLight();

  out_color = vec4(output, 1);
}

)";

}  // namespace

Shader CreateFullLightShader(Renderer* renderer) {
  Shader shader;

  shader.name = "full-lighting";
  shader.vertex_type = VertexType::k3dNormalUV;
  shader.vert_ubo_name = "VertUniforms";
  shader.vert_ubo_size = sizeof(SpotLightShaderUBO::Vert);

  shader.frag_ubo_name = "FragUniforms";
  shader.frag_ubo_size = sizeof(SpotLightShaderUBO::Frag);

  shader.texture_count = 2;

  shader.vert_src = CreateVertexSource(kFullLightVertShader);
  shader.frag_src = CreateFragmentSource(kFullLightFragShader);

  if (!RendererStageShader(renderer, &shader))
    return {};
  return shader;
}

}  // namespace textured_lighting
}  // namespace rothko
