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
  light_node->transform.position = {3, 2, 0};
  light_node->transform.scale *= 0.2f;

  simple_lighting::ObjectShaderUBO object_ubo = {};
  object_ubo.frag.object_color = ToVec3(Color::Yellow());
  object_ubo.frag.light_color = ToVec3(Color::Red());

  simple_lighting::LightShaderUBO light_ubo = {};
  light_ubo.frag.light_color = ToVec3(Color::White());

  bool running = true;
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

    DefaultUpdateOrbitCamera(game.input, &camera);

    float angle = game.time.seconds * ToRadians(20.0f);
    base_light->transform.rotation.y = angle;
    Update(scene_graph.get());

    // Update the UBOs.

    /* Vec4 new_pos = light_node->transform.world_matrix * light_node->transform.position; */
    Vec4 new_pos = light_node->transform.world_matrix * Vec3{};
    object_ubo.frag.light_pos = ToVec3(new_pos);
    light_ubo.vert.model = light_node->transform.world_matrix;

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Gray66()));
    commands.push_back(GetCommand(camera));

    // Draw the cubes.
    commands.push_back(CreateRenderCommand(&cube_mesh, &object_shader, object_ubo));
    commands.push_back(CreateRenderCommand(&light_cube_mesh, &light_shader, light_ubo));

    commands.push_back(grid.render_command);

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
