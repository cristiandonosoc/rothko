// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/widgets/lights.h"

#include "rothko/models/cube.h"
#include "rothko/scene/transform.h"
#include "rothko/utils/strings.h"

namespace rothko {

// Shader Creation ---------------------------------------------------------------------------------

// ***** Point Light ******

namespace {

constexpr char kLightWidgetVertShader[] = R"(
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

constexpr char kLightWidgetFragShader[] = R"(
layout (location = 0) out vec4 out_color;

layout (std140) uniform FragUniforms {
  vec3 color;
};

void main() {
  out_color = vec4(color, 1);
}
)";

constexpr char kLightWidgetShaderName[] = "point-light-shader";


const Shader* GetLightWidgetShader(Renderer* renderer) {
  {
    const Shader* shader = RendererGetShader(renderer, kLightWidgetShaderName);
    if (shader)
      return shader;
  }

  ShaderConfig config = {};
  config.name = "directional-light-shader";
  config.vertex_type = VertexType::k3d;
  config.ubos[0].name = "VertUniforms";
  config.ubos[0].size = sizeof(Mat4);
  config.ubos[1].name = "FragUniforms";
  config.ubos[1].size = sizeof(Vec3);

  auto vert_src = CreateVertexSource(kLightWidgetVertShader);
  auto frag_src = CreateFragmentSource(kLightWidgetFragShader);

  static auto shader = RendererStageShader(renderer, config, vert_src, frag_src);
  return shader.get();
}

}  // namespace

// Mesh Creation -----------------------------------------------------------------------------------

namespace {

Mesh CreatePointLightMesh(Renderer* renderer, const std::string& name) {
  auto mesh_name = StringPrintf("%s-light-widget-cube-mesh", name.c_str());
  Mesh point_light_mesh = CreateCubeMesh(VertexType::k3d, std::move(mesh_name));
  if (!RendererStageMesh(renderer, &point_light_mesh))
    return {};
  return point_light_mesh;
}

Mesh CreateDirectionalLightMesh(Renderer* renderer, const std::string& name) {
  Mesh mesh = {};
  mesh.name = StringPrintf("%s-directional-light-mesh", name.c_str());
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

}  // namespace

// LightWidgetManager ------------------------------------------------------------------------------

bool Init(LightWidgetManager* lights, Renderer* renderer, const std::string& name) {
  if (!Init(&lights->lines, renderer, StringPrintf("%s-light-widget-lines", name.c_str())))
    return false;

  const Shader* widget_shader = GetLightWidgetShader(renderer);

  lights->name = name;
  lights->point_light_shader = widget_shader;
  lights->directional_light_shader = widget_shader;
  lights->point_light_mesh = CreatePointLightMesh(renderer, name);
  lights->directional_light_mesh = CreateDirectionalLightMesh(renderer, name);

  return true;
}

bool Init(LightWidgetManager* lights, Renderer* renderer, const std::string& name,
          const Shader* point_light_shader,
          const Shader* directional_light_shader,
          const Shader* lines_shader) {

  if (!Init(&lights->lines, renderer, lines_shader, "light-widget-lines"))
    return false;

  lights->name = name;
  lights->point_light_shader = point_light_shader;
  lights->point_light_mesh = CreatePointLightMesh(renderer, name);
  lights->directional_light_shader = directional_light_shader;
  lights->directional_light_mesh = CreateDirectionalLightMesh(renderer, name);

  Reset(lights);

  return true;
}

void Reset(LightWidgetManager* lights) {
  lights->point_lights.clear();
  lights->directional_lights.clear();

  Reset(&lights->lines);
}

void Stage(LightWidgetManager* lights, Renderer* renderer) {
  Stage(&lights->lines, renderer);
}

// Push Lights -------------------------------------------------------------------------------------

void PushPointLight(LightWidgetManager* lights, Transform* transform, Vec3 color) {
  PointLight light = {};
  light.transform = transform;
  light.color = color;

  lights->point_lights.push_back(std::move(light));
}

void PushDirectionalLight(LightWidgetManager* lights, Transform* transform, Vec3 color) {
  DirectionalLight light = {};
  light.transform = transform;
  light.color = color;

  lights->directional_lights.push_back(std::move(light));
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
}

// GetRenderCommands -------------------------------------------------------------------------------

std::vector<RenderCommand> GetRenderCommands(const LightWidgetManager& lights) {
  std::vector<RenderCommand> render_commands;
  render_commands.reserve(lights.point_lights.size() +
                          lights.directional_lights.size());

  // Point lights.
  for (auto& light : lights.point_lights) {
    RenderMesh render_mesh = {};
    render_mesh.mesh = &lights.point_light_mesh;
    render_mesh.shader = lights.point_light_shader;
    render_mesh.primitive_type = PrimitiveType::kTriangles;
    render_mesh.indices_count = lights.point_light_mesh.indices.size();
    render_mesh.ubo_data[0] = (uint8_t*)&light.transform->world_matrix;
    render_mesh.ubo_data[1] = (uint8_t*)&light.color;

    render_commands.push_back(std::move(render_mesh));
  }

  // Directional lights.
  for (auto& light : lights.directional_lights) {
    RenderMesh render_mesh = {};
    render_mesh.mesh = &lights.directional_light_mesh;
    render_mesh.shader = lights.directional_light_shader;
    render_mesh.primitive_type = PrimitiveType::kLineStrip;
    render_mesh.indices_count = lights.directional_light_mesh.indices.size();
    render_mesh.ubo_data[0] = (uint8_t*)&light.transform->world_matrix;
    render_mesh.ubo_data[1] = (uint8_t*)&light.color;

    render_commands.push_back(std::move(render_mesh));
  }

  // Spot lights.
  render_commands.push_back(GetRenderCommand(lights.lines));

  return render_commands;
}

}  // namespace rothko
