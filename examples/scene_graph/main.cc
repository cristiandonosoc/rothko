// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/graphics/default_shaders/default_shaders.h>
#include <rothko/models/cube.h>
#include <rothko/scene/camera.h>
#include <rothko/scene/grid.h>
#include <rothko/scene/scene_graph.h>

using namespace rothko;

namespace {

RenderMesh GetCubeRenderCommand(Mesh* mesh, Shader* shader, Transform* transform) {
  RenderMesh render_mesh = {};

  render_mesh.mesh = mesh;
  render_mesh.shader = shader;
  render_mesh.primitive_type = PrimitiveType::kTriangles;
  render_mesh.cull_faces = false;
  render_mesh.indices_size = mesh->index_count;
  render_mesh.vert_ubo_data = (uint8_t*)&transform->world_matrix;
  /* render_mesh.textures.push_back(tex1); */
  /* render_mesh.textures.push_back(tex0); */

  return render_mesh;
}

}  // namespace

int main() {
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  Game game;
  if (!InitGame(&game, &window_config, false))
    return 1;

  Grid grid;
  if (!Init(game.renderer.get(), &grid, "main-grid"))
    return 1;

  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  Shader default_shader = CreateDefaultShader(VertexType::k3dNormalUV);
  if (!Valid(default_shader) || !RendererStageShader(game.renderer.get(), &default_shader))
    return 1;

  Mesh cube = CreateCubeMesh("cube");
  if (!RendererStageMesh(game.renderer.get(), &cube))
    return 1;

  auto scene_graph = std::make_unique<SceneGraph>();
  Transform* transform = AddTransform(scene_graph.get());
  /* transform->position = {0, 3, 0}; */
  /* transform->scale = {0.5f, 0.5f, 0.5f}; */

  Transform* child = AddTransform(scene_graph.get(), transform);
  child->scale = {0.3f, 0.3f, 0.3f};

  Transform* grand_child = AddTransform(scene_graph.get(), child);
  grand_child->scale = {0.7f, 0.3f, 1.3f};

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

    /* float angle = game.time.seconds * ToRadians(20.0f); */
    /* transform->rotation.y = angle; */

    float child_angle = game.time.seconds * ToRadians(33.0f);
    child->position = {2 * Cos(child_angle), 0, 2 * Sin(child_angle)};

    /* grand_child->position = {0, 0.5f * Cos(2 * child_angle), 0.5f * Sin(2 * child_angle)}; */
    grand_child->position = {0, 1, 0};
    Update(transform);
    Update(child, transform);
    Update(grand_child, child);

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Graycc()));
    commands.push_back(GetCommand(camera));

    commands.push_back(GetCubeRenderCommand(&cube, &default_shader, transform));
    commands.push_back(GetCubeRenderCommand(&cube, &default_shader, child));
    commands.push_back(GetCubeRenderCommand(&cube, &default_shader, grand_child));
    commands.push_back(grid.render_command);

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
