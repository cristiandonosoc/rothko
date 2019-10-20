// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/models/cube.h>
#include <rothko/scene/camera.h>
#include <rothko/scene/grid.h>
#include <rothko/scene/scene_graph.h>

#include "shaders.h"

using namespace rothko;

namespace {

template <typename UBO>
RenderMesh CreateRenderCommand(Mesh* mesh,
                               Shader* shader,
                               const UBO& ubo) {
  RenderMesh render_mesh = {};
  render_mesh.mesh = mesh;
  render_mesh.shader = shader;
  render_mesh.primitive_type = PrimitiveType::kTriangles;

  render_mesh.indices_count = mesh->indices.size();

  render_mesh.vert_ubo_data = (uint8_t*)&ubo.vert;
  render_mesh.frag_ubo_data = (uint8_t*)&ubo.frag;

  return render_mesh;
}

// Material table.

std::pair<std::string, simple_lighting::ObjectShaderUBO::Frag::Material> CreateMaterial(
    const std::string& name, Vec3 ambient, Vec3 diffuse, Vec3 specular, float shininess) {
  simple_lighting::ObjectShaderUBO::Frag::Material material = {};
  material.ambient = ambient;
  material.diffuse = diffuse;
  material.specular = specular;
  material.shininess = shininess;

  return {name, std::move(material)};
}

std::vector<std::pair<std::string, simple_lighting::ObjectShaderUBO::Frag::Material>> kMaterials = {

    CreateMaterial("emerald",
                   {0.0215f, 0.1745f, 0.0215f},
                   {0.07568f, 0.61424f, 0.07568f},
                   {0.633f, 0.727811f, 0.633f},
                   0.6f),
    CreateMaterial(
        "jade",
        {0.135f, 0.2225f, 0.1575f},
        {0.54f, 0.89f, 0.63f},
        {0.316228f, 0.316228f, 0.316228f}, 0.1f),
    CreateMaterial("obsidian",
                   {0.05375f, 0.05f, 0.06625f},
                   {0.18275f, 0.17f, 0.22525f},
                   {0.332741f, 0.328634f, 0.346435f},
                   0.3f),
    CreateMaterial("pearl",
                   {0.25f, 0.20725f, 0.20725f},
                   {1.00f, 0.829f, 0.829f},
                   {0.296648f, 0.296648f, 0.296648f},
                   0.088f),
    CreateMaterial("ruby",
                   {0.1745f, 0.01175f, 0.01175f},
                   {0.61424f, 0.04136f, 0.04136f},
                   {0.727811f, 0.626959f, 0.626959f},
                   0.6f),
    CreateMaterial("turquoise",
                   {0.10f, 0.18725f, 0.1745f},
                   {0.396f, 0.74151f, 0.69102f},
                   {0.297254f, 0.30829f, 0.306678f},
                   0.1f),
    CreateMaterial("brass",
                   {0.329412f, 0.223529f, 0.027451f},
                   {0.780392f, 0.568627f, 0.113725f},
                   {0.992157f, 0.941176f, 0.807843f},
                   0.21794872f),
    CreateMaterial("bronze",
                   {0.2125f, 0.1275f, 0.054f},
                   {0.714f, 0.4284f, 0.18144f},
                   {0.393548f, 0.271906f, 0.166721f},
                   0.2f),
};

};  // namespace

int main() {
  Game game = {};
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitGame(&game, &window_config, true))
    return 1;

  Shader object_shader = simple_lighting::CreateObjectShader(game.renderer.get());
  if (!Valid(object_shader))
    return 1;

  Shader light_shader = simple_lighting::CreateLightShader(game.renderer.get());
  if (!Valid(light_shader))
    return 1;

  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  Grid grid;
  if (!Init(game.renderer.get(), &grid, "main-grid"))
    return 1;

  Mesh cube_mesh = CreateCubeMesh(VertexType::k3dNormal, "cube");
  if (!RendererStageMesh(game.renderer.get(), &cube_mesh))
    return 1;

  Mesh light_cube_mesh = CreateCubeMesh(VertexType::k3d, "light-cube");
  if (!RendererStageMesh(game.renderer.get(), &light_cube_mesh))
    return 1;

  auto scene_graph = std::make_unique<SceneGraph>();

  SceneNode* base_light = AddNode(scene_graph.get());
  SceneNode* light_node = AddNode(scene_graph.get(), base_light);
  light_node->transform.position = {3, 1.4f, 1.6f};
  light_node->transform.scale *= 0.2f;

  simple_lighting::LightShaderUBO light_ubo = {};
  light_ubo.frag.light_color = ToVec3(Color::White());

  // Create the UBOs.
  std::vector<simple_lighting::ObjectShaderUBO> ubos;
  ubos.reserve(kMaterials.size());

  int i = 0;
  constexpr int kMaxRow = 4;
  for (auto& [name, mat] : kMaterials) {
    simple_lighting::ObjectShaderUBO ubo = {};
    float x = i % kMaxRow - 1;
    float y = i / kMaxRow;
    ubo.vert.model = Translate({x, y, 0});
    ubo.vert.model *= Scale(0.5f);

    ubo.frag.light.ambient = {0.2f, 0.2f, 0.2f};
    ubo.frag.light.diffuse = {0.5f, 0.5f, 0.5f};
    ubo.frag.light.specular = {1, 1, 1};

    mat.shininess *= 100;
    ubo.frag.material = mat;

    ubos.push_back(std::move(ubo));
    i++;
  }

  bool running = true;
  bool move_light = true;
  Vec3 light_pos = {};
  float time_delta = 0;
  while (running) {
    auto events = Update(&game);
    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    if (KeyUpThisFrame(&game.input, Key::kEscape)) {
      running = false;
      break;
    }

    if (KeyUpThisFrame(&game.input, Key::kSpace)) {
      move_light = !move_light;
    }

    DefaultUpdateOrbitCamera(game.input, &camera);
    Update(scene_graph.get());

    // Update the UBOs.
    if (move_light) {
      time_delta += game.time.frame_delta;
      light_pos = Vec3(Sin(time_delta) * 4, 0.4f, 1);
    }

    light_ubo.vert.model = Translate(light_pos);
    light_ubo.vert.model *= Scale(0.1f);

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Gray66()));
    commands.push_back(GetCommand(camera));

    // Draw the cubes.
    for (auto& ubo : ubos) {
      ubo.frag.light.pos = light_pos;
      commands.push_back(CreateRenderCommand(&cube_mesh, &object_shader, ubo));
    }
    commands.push_back(CreateRenderCommand(&light_cube_mesh, &light_shader, light_ubo));
    commands.push_back(grid.render_command);

    commands.push_back(PopCamera());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
