// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/widgets/grid.h"

#include "rothko/utils/strings.h"

namespace rothko {

// Create Grid -------------------------------------------------------------------------------------

namespace {

constexpr char kGridShaderName[] = "default-grid-shader";

}  // namespace

bool Init(Grid* grid, Renderer* renderer) {
  // See if we can get the shader.
  const Shader* shader = RendererGetShader(renderer, kGridShaderName);
  if (!shader) {
    // Otherwise we keep a global shader.
    static auto default_shader = CreateGridShader(renderer, kGridShaderName);
    if (!default_shader)
      return {};

    shader = default_shader.get();
  }

  return Init(grid, renderer, shader);
}

// Init --------------------------------------------------------------------------------------------

namespace {

bool InitMesh(Renderer* renderer, Grid* grid) {
  Mesh* mesh = &grid->mesh;
  mesh->name = "grid-widget-mesh";

  mesh->vertex_type = VertexType::k3dUV;

  constexpr float size = 10000.0f;
  Vertex3dUV vertices[] = {
      {{-size, 0, -size}, {0, 0}},
      {{ size, 0, -size}, {0, 1}},
      {{ size, 0,  size}, {1, 1}},
      {{-size, 0,  size}, {1, 0}},
  };

  Mesh::IndexType indices[] = {
    0, 1, 2, 2, 3, 0,
  };

  PushVertices(mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(mesh, indices, ARRAY_SIZE(indices));

  return RendererStageMesh(renderer, mesh);
}

}  // namespace

bool Init(Grid* grid, Renderer* renderer, const Shader* shader) {
  if (!InitMesh(renderer, grid))
    return false;

  grid->render_command = {};
  grid->render_command.primitive_type = PrimitiveType::kTriangles;
  grid->render_command.mesh = &grid->mesh;
  grid->render_command.shader = shader;
  grid->render_command.flags = kBlendEnabled | kDepthTest;
  grid->render_command.indices_count = grid->mesh.indices.size();

  return true;
}

// Shader ------------------------------------------------------------------------------------------

namespace {

constexpr char kGridVertexShader[] = R"(
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;

out vec2 f_uv;

out vec3 f_pos;
out vec3 f_transformed_pos;

void main() {
  gl_Position = camera_proj * camera_view * vec4(in_pos, 1.0);
  f_uv = in_uv;

  // NOTE(Cristian): we offset the point because the calculation for the line alpha weight does a
  //                 range shift that also translates. I still haven't found how not to do it
  //                 without this hack :(
  f_pos = in_pos + vec3(0.5f, 0, 0.5f);
  f_transformed_pos = gl_Position.xyz;
}
)";

constexpr char kGridFragmentShader[] = R"(
in vec2 f_uv;
in vec3 f_pos;
in vec3 f_transformed_pos;

layout (location = 0) out vec4 out_color;

const float fog_near = 20.0f;
const float fog_far = 50.0f;

void main() {
  vec2 wrapped = abs(fract(f_pos.xz) - vec2(0.5f, 0.5f));
  vec2 speed = fwidth(f_pos.xz);

  vec2 range = wrapped / speed;

  float line_width = 0.05f;
  float weight = clamp(min(range.x, range.y) - line_width, 0.0f, 1.0f);

  /* float camera_dist = distance(camera_pos, f_transformed_pos); */
  float camera_dist = distance(camera_pos, f_pos);

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

std::unique_ptr<Shader> CreateGridShader(Renderer* renderer, const std::string& name) {
  ShaderConfig config = {};
  config.name = name;
  config.vertex_type = VertexType::k3dUV;

  auto vert_src = CreateVertexSource(kGridVertexShader);
  auto frag_src = CreateFragmentSource(kGridFragmentShader);

  return RendererStageShader(renderer, config, vert_src, frag_src);
}

}  // namespace rothko
