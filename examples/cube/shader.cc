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
  auto shader = std::make_unique<Shader>();
  shader->name = "cube-shader";
  shader->vert_ubo_name = "Uniforms";
  shader->vert_ubo_size = sizeof(UBO);
  shader->texture_count = 2;

  shader->vert_src = CreateVertexSource(kVertexShader);
  shader->frag_src = CreateFragmentSource(kFragmentShader);

  if (!RendererStageShader(renderer, shader.get()))
    return nullptr;
  return shader;
}

// Grid Shader -------------------------------------------------------------------------------------

namespace {

constexpr char kGridVertexShader[] = R"(
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec2 f_uv;
out vec4 f_color;

out vec3 f_pos;
out vec3 f_transformed_pos;

void main() {
  gl_Position = camera_proj * camera_view * vec4(in_pos, 1.0);
  f_uv = in_uv;
  f_color = in_color;

  // NOTE(Cristian): we offset the point because the calculation for the line alpha weight does a
  //                 range shift that also translates. I still haven't found how not to do it
  //                 without this hack :(
  f_pos = in_pos + vec3(0.5f, 0, 0.5f);
  f_transformed_pos = gl_Position.xyz;
}
)";

constexpr char kGridFragmentShader[] = R"(
in vec2 f_uv;
in vec4 f_color;
in vec3 f_pos;
in vec3 f_transformed_pos;

layout (location = 0) out vec4 out_color;

const float fog_near = 10.0f;
const float fog_far = 50.0f;

void main() {
  vec2 wrapped = abs(fract(f_pos.xz) - vec2(0.5f, 0.5f));
  vec2 speed = fwidth(f_pos.xz);

  vec2 range = wrapped / speed;

  float line_width = 0.05f;
  float weight = clamp(min(range.x, range.y) - line_width, 0.0f, 1.0f);

  float camera_dist = distance(camera_pos, f_transformed_pos);

  float fog = 1 - ((camera_dist - fog_near) / (fog_far - fog_near));
  /* out_color = vec4(0, 0, 0, fog); */

  float grid_weight = 1 - weight;

  vec2 real_pos = f_pos.xz - vec2(0.5f, 0.5f);
  if (abs(real_pos.x) < 0.1f && abs(real_pos.y) < 0.1f) {
    out_color = vec4(1, 0, 0, 1);
  } else {
    out_color = vec4(0, 0, 0, grid_weight * fog);
  }


}
)";

}  // namespace

std::unique_ptr<Shader> CreateGridShader(Renderer* renderer) {
  auto shader = std::make_unique<Shader>();
  shader->name = "grid-shader";

  shader->vert_src = CreateVertexSource(kGridVertexShader);
  shader->frag_src = CreateFragmentSource(kGridFragmentShader);

  if (!RendererStageShader(renderer, shader.get()))
    return nullptr;
  return shader;
}
