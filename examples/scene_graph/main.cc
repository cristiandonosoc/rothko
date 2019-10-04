// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/graphics/default_shaders/default_shaders.h>
#include <rothko/models/cube.h>
#include <rothko/scene/camera.h>
#include <rothko/scene/grid.h>

using namespace rothko;

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

  Mat4 model_matrix = Mat4::Identity();


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
    model_matrix = Rotate({1, 2, 3}, angle);

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Blue()));
    commands.push_back(GetCommand(camera));
    /* commands.push_back(grid.render_command); */

    RenderMesh render_cube = {};
    render_cube.mesh = &cube;
    render_cube.shader = &default_shader;
    render_cube.primitive_type = PrimitiveType::kTriangles;
    render_cube.cull_faces = false;
    render_cube.indices_size = cube.index_count;
    render_cube.vert_ubo_data = (uint8_t*)&model_matrix;
    /* render_cube.textures.push_back(tex1); */
    /* render_cube.textures.push_back(tex0); */
    commands.push_back(render_cube);

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
