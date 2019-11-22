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

float kScale = 0.2f;

void main() {
  vec3 pos = in_pos * kScale;
  gl_Position = camera_proj * camera_view * model * vec4(pos, 1.0);
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

float kScale = 0.4f;

void main() {
  vec3 pos = in_pos * kScale;
  gl_Position = camera_proj * camera_view * model * vec4(pos, 1.0);
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

std::unique_ptr<Shader> CreatePointLightShader(Renderer* renderer) {
  ShaderConfig config = {};
  config.name = "point-light-shader";
  config.vertex_type = VertexType::k3d;
  config.vert_ubo_name = "VertUniforms";
  config.vert_ubo_size = sizeof(Mat4);
  config.frag_ubo_name = "FragUniforms";
  config.frag_ubo_size = sizeof(Vec3);

  auto vert_src = CreateVertexSource(kPointLightVertShader);
  auto frag_src = CreateFragmentSource(kPointLightFragShader);

  return RendererStageShader(renderer, config, vert_src, frag_src);
}

std::unique_ptr<Shader> CreateDirectionalLightShader(Renderer* renderer) {
  ShaderConfig config = {};
  config.name = "directional-light-shader";
  config.vertex_type = VertexType::k3d;
  config.vert_ubo_name = "VertUniforms";
  config.vert_ubo_size = sizeof(Mat4);
  config.frag_ubo_name = "FragUniforms";
  config.frag_ubo_size = sizeof(Vec3);

  auto vert_src = CreateVertexSource(kDirectionalLightVertShader);
  auto frag_src = CreateFragmentSource(kDirectionalLightFragShader);

  return RendererStageShader(renderer, config, vert_src, frag_src);
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

bool Init(LightWidgetManager* light_widgets, Renderer* renderer, const std::string& name,
          const Shader* point_light_shader,
          const Mesh* point_light_mesh,
          const Shader* directional_light_shader,
          const Mesh* directional_light_mesh,
          const Shader* lines_shader) {

  if (!Init(&light_widgets->lines, renderer, lines_shader, "light-widget-lines"))
    return false;

  light_widgets->name = name;
  light_widgets->point_light_shader = point_light_shader;
  light_widgets->point_light_mesh = point_light_mesh;
  light_widgets->directional_light_shader = directional_light_shader;
  light_widgets->directional_light_mesh = directional_light_mesh;

  Reset(light_widgets);

  return true;
}

void Reset(LightWidgetManager* light_widgets) {
  light_widgets->point_lights.clear();
  light_widgets->directional_lights.clear();

  Reset(&light_widgets->lines);
}

void Stage(LightWidgetManager* light_widgets, Renderer* renderer) {
  Stage(&light_widgets->lines, renderer);
}

// Push Lights -------------------------------------------------------------------------------------

void PushPointLight(LightWidgetManager* light_widgets, Transform* transform, Vec3 color) {
  PointLight light = {};
  light.transform = transform;
  light.color = color;

  light_widgets->point_lights.push_back(std::move(light));
}

void PushDirectionalLight(LightWidgetManager* light_widgets, Transform* transform, Vec3 color) {
  DirectionalLight light = {};
  light.transform = transform;
  light.color = color;

  light_widgets->directional_lights.push_back(std::move(light));
}

void PushSpotLight(LightWidgetManager* lights, const SpotLight& light) {
  Vec3 pos = GetWorldPosition(*light.transform);
  Vec3 dir = GetWorldDirection(*light.transform);

  // Add the lines.
  auto [forward, up, right] = GetAxisFrame(dir);
  float s = Sin(light.angle);

  Vec3 end = pos + dir;
  PushLine(&lights->lines, pos, end - up * s, light.color);
  PushLine(&lights->lines, pos, end + up * s, light.color);
  PushLine(&lights->lines, pos, end - right * s, light.color);
  PushLine(&lights->lines, pos, end + right * s, light.color);

  // Add the end ring.
  PushRing(&lights->lines, end, dir, s, light.color);


  /* // Add the lines. */
  /* constexpr int kRingCount = 5; */
  /* constexpr float kDiff = 1.0f / (kRingCount - 1); */

  /* for (int i = 1; i < kRingCount; i++) { */
  /*   float diff = i * kDiff; */
  /*   float r = s * diff; */

  /*   PushRing(&lights->lines, */
  /*            light.position + light.direction * diff, */
  /*            light.direction, */
  /*            r, */
  /*            light.color); */
  /* } */
}

// GetRenderCommands -------------------------------------------------------------------------------

std::vector<RenderCommand> GetRenderCommands(const LightWidgetManager& light_widgets) {
  std::vector<RenderCommand> render_commands;
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

  // Spot lights.
  render_commands.push_back(GetRenderCommand(light_widgets.lines));

  return render_commands;
}

}  // namespace rothko
