// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/widgets/lights.h"

#include "rothko/models/cube.h"

namespace rothko {

// Shader Creation ---------------------------------------------------------------------------------

namespace {

constexpr char kPointLightVertShader[] = R"(

layout (location = 0) in vec3 in_pos;

layout (std140) uniform VertUniforms {
  vec3 offset_pos;
};

void main() {
  vec3 pos = in_pos * 0.2f + offset_pos;
  gl_Position = camera_proj * camera_view * vec4(pos, 1.0);
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

}  // namespace

Shader CreatePointLightShader(Renderer* renderer) {
  Shader shader;
  shader.name = "lighting";
  shader.vertex_type = VertexType::k3d;
  shader.vert_ubo_name = "VertUniforms";
  shader.vert_ubo_size = sizeof(Vec3);

  shader.frag_ubo_name = "FragUniforms";
  shader.frag_ubo_size = sizeof(Vec3);

  shader.vert_src = CreateVertexSource(kPointLightVertShader);
  shader.frag_src = CreateFragmentSource(kPointLightFragShader);

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

// LightWidgetManager ------------------------------------------------------------------------------

void Init(LightWidgetManager* light_widgets, const std::string& name, Shader* point_light_shader,
                                             Mesh* point_light_mesh) {
  light_widgets->name = name;
  light_widgets->point_light_shader = point_light_shader;
  light_widgets->point_light_mesh = point_light_mesh;

  Reset(light_widgets);
}

void Reset(LightWidgetManager* light_widgets) {
  light_widgets->point_lights.clear();
  light_widgets->directional_lights.clear();
}

void PushPointLight(LightWidgetManager* light_widgets, LightWidgetManager::PointLight point_light) {
  light_widgets->point_lights.push_back(std::move(point_light));
}

std::vector<RenderMesh> GetRenderCommands(const LightWidgetManager& light_widgets) {
  std::vector<RenderMesh> render_commands;
  render_commands.reserve(light_widgets.point_lights.size() +
                          light_widgets.directional_lights.size());

  // Point lights.
  for (auto& point_light : light_widgets.point_lights) {
    RenderMesh render_mesh = {};
    render_mesh.mesh = light_widgets.point_light_mesh;
    render_mesh.shader = light_widgets.point_light_shader;
    render_mesh.primitive_type = PrimitiveType::kTriangles;
    render_mesh.indices_count = light_widgets.point_light_mesh->indices.size();
    render_mesh.vert_ubo_data =
        (uint8_t*)&point_light + offsetof(LightWidgetManager::PointLight, position);
    render_mesh.frag_ubo_data =
        (uint8_t*)&point_light + offsetof(LightWidgetManager::PointLight, color);

    render_commands.push_back(std::move(render_mesh));
  }

  // TODO(Cristian): Add directional lights.

  return render_commands;
}

}  // namespace rothko
