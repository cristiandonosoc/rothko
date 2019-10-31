// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/widgets/lights.h"

#include "rothko/models/cube.h"
#include "rothko/scene/transform.h"

namespace rothko {

// Shader Creation ---------------------------------------------------------------------------------

// ***** Point Light ******

namespace {

constexpr char kPointLightVertShader[] = R"(
layout (location = 0) in vec3 in_pos;

layout (std140) uniform VertUniforms {
  mat4 model;
};

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);
}
)";

constexpr char kPointLightFragShader[] = R"(
layout (location = 0) out vec4 out_color;

layout (std140) uniform FragUniforms {
  vec3 color;
};

void main() {
  out_color = vec4(color, 1);
}
)";

// ***** Directional Light *****

constexpr char kDirectionalLightVertShader[] = R"(
layout (location = 0) in vec3 in_pos;

layout (std140) uniform VertUniforms {
  mat4 model;
};

void main() {
  gl_Position = camera_proj * camera_view * model * vec4(in_pos, 1.0);
}
)";

constexpr char kDirectionalLightFragShader[] = R"(
layout (location = 0) out vec4 out_color;

layout (std140) uniform FragUniforms {
  vec3 color;
};

void main() {
  out_color = vec4(color, 1);
}
)";

}  // namespace

Shader CreatePointLightShader(Renderer* renderer) {
  Shader shader;
  shader.name = "point-light-shader";
  shader.vertex_type = VertexType::k3d;
  shader.vert_ubo_name = "VertUniforms";
  shader.vert_ubo_size = sizeof(Mat4);

  shader.frag_ubo_name = "FragUniforms";
  shader.frag_ubo_size = sizeof(Vec3);

  shader.vert_src = CreateVertexSource(kPointLightVertShader);
  shader.frag_src = CreateFragmentSource(kPointLightFragShader);

  if (!RendererStageShader(renderer, &shader))
    return {};
  return shader;
}

Shader CreateDirectionalLightShader(Renderer* renderer) {
  Shader shader;
  shader.name = "directional-light-shader";
  shader.vertex_type = VertexType::k3d;
  shader.vert_ubo_name = "VertUniforms";
  shader.vert_ubo_size = sizeof(Mat4);

  shader.frag_ubo_name = "FragUniforms";
  shader.frag_ubo_size = sizeof(Vec3);

  shader.vert_src = CreateVertexSource(kDirectionalLightVertShader);
  shader.frag_src = CreateFragmentSource(kDirectionalLightFragShader);

  if (!RendererStageShader(renderer, &shader))
    return {};
  return shader;
}

// Mesh Creation -----------------------------------------------------------------------------------

Mesh CreatePointLightMesh(Renderer* renderer) {
  Mesh point_light_mesh = CreateCubeMesh(VertexType::k3d, "light-cube");
  if (!RendererStageMesh(renderer, &point_light_mesh))
    return {};
  return point_light_mesh;
}

Mesh CreateDirectionalLightMesh(Renderer* renderer) {
  Mesh mesh = {};
  mesh.name = "directional-light-mesh";
  mesh.vertex_type = VertexType::k3d;

  // Create circle.
  constexpr int kCirclePoints = 16;
  constexpr float kSizeFactor = 0.6f;
  constexpr float kRayLength = 2.0f;
  constexpr float kAngle = kRadians360 / (float)kCirclePoints;

  std::vector<Vertex3d> vertices;
  std::vector<Mesh::IndexType> indices;
  for (int i = 0; i < kCirclePoints; i++) {
    vertices.push_back(Vertex3d{{0, kSizeFactor * Cos(i * kAngle), kSizeFactor * Sin(i * kAngle)}});
    indices.push_back(i);
  }
  indices.push_back(0);   // Close the loop.

  // Create lines.
  for (int i = 0; i < kCirclePoints; i++) {
    vertices.push_back(
        Vertex3d{{kRayLength, kSizeFactor * Cos(i * kAngle), kSizeFactor * Sin(i * kAngle)}});
    indices.push_back(line_strip::kPrimitiveReset);
    indices.push_back(i);
    indices.push_back(i + kCirclePoints);
  }

  PushVertices(&mesh, vertices.data(), vertices.size());
  PushIndices(&mesh, indices.data(), indices.size());

  if (!RendererStageMesh(renderer, &mesh))
    return {};
  return mesh;
}

// LightWidgetManager ------------------------------------------------------------------------------

void Init(LightWidgetManager* light_widgets, const std::string& name,
                                             Shader* point_light_shader,
                                             Mesh* point_light_mesh,
                                             Shader* directional_light_shader,
                                             Mesh* directional_light_mesh) {
  light_widgets->name = name;
  light_widgets->point_light_shader = point_light_shader;
  light_widgets->point_light_mesh = point_light_mesh;
  light_widgets->directional_light_shader = directional_light_shader;
  light_widgets->directional_light_mesh = directional_light_mesh;

  Reset(light_widgets);
}

void Reset(LightWidgetManager* light_widgets) {
  light_widgets->point_lights.clear();
  light_widgets->directional_lights.clear();
}

// Push Lights -------------------------------------------------------------------------------------

void PushPointLight(LightWidgetManager* light_widgets, Transform* transform, Vec3 color) {
  LightWidgetManager::Light light = {};
  light.transform = transform;
  light.color = color;

  light_widgets->point_lights.push_back(std::move(light));
}

void PushDirectionalLight(LightWidgetManager* light_widgets, Transform* transform, Vec3 color) {
  LightWidgetManager::Light light = {};
  light.transform = transform;
  light.color = color;

  light_widgets->directional_lights.push_back(std::move(light));
}

// GetRenderCommands -------------------------------------------------------------------------------

std::vector<RenderMesh> GetRenderCommands(const LightWidgetManager& light_widgets) {
  std::vector<RenderMesh> render_commands;
  render_commands.reserve(light_widgets.point_lights.size() +
                          light_widgets.directional_lights.size());

  // Point lights.
  for (auto& light : light_widgets.point_lights) {
    RenderMesh render_mesh = {};
    render_mesh.mesh = light_widgets.point_light_mesh;
    render_mesh.shader = light_widgets.point_light_shader;
    render_mesh.primitive_type = PrimitiveType::kTriangles;
    render_mesh.indices_count = light_widgets.point_light_mesh->indices.size();
    render_mesh.vert_ubo_data = (uint8_t*)&light.transform->world_matrix;
    render_mesh.frag_ubo_data = (uint8_t*)&light.color;

    render_commands.push_back(std::move(render_mesh));
  }

  // Directional lights.
  for (auto& light : light_widgets.directional_lights) {
    RenderMesh render_mesh = {};
    render_mesh.mesh = light_widgets.directional_light_mesh;
    render_mesh.shader = light_widgets.directional_light_shader;
    render_mesh.primitive_type = PrimitiveType::kLineStrip;
    render_mesh.indices_count = light_widgets.directional_light_mesh->indices.size();
    render_mesh.vert_ubo_data = (uint8_t*)&light.transform->world_matrix;
    render_mesh.frag_ubo_data = (uint8_t*)&light.color;

    render_commands.push_back(std::move(render_mesh));
  }

  return render_commands;
}

}  // namespace rothko
