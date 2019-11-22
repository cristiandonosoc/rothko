// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/scene/camera.h>
#include <rothko/widgets/grid.h>
#include <rothko/game.h>

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

  auto grid_shader = CreateGridShader(game.renderer.get(), "grid-shader");
  if (!grid_shader)
    return 1;

  Grid grid;
  if (!Init(&grid, game.renderer.get(), grid_shader.get()))
    return 1;

  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  bool running = true;
  while (running) {
    auto events = Update(&game);
    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    if (KeyUpThisFrame(game.input, Key::kEscape)) {
      running = false;
      break;
    }

    constexpr float kMouseSensibility = 0.007f;
    static float kMaxPitch = ToRadians(89.0f);
    if (!IsZero(game.input.mouse_offset)) {
      camera.angles.x -= game.input.mouse_offset.y * kMouseSensibility;
      if (camera.angles.x > kMaxPitch) {
        camera.angles.x = kMaxPitch;
      } else if (camera.angles.x < -kMaxPitch) {
        camera.angles.x = -kMaxPitch;
      }

      camera.angles.y += game.input.mouse_offset.x * kMouseSensibility;
      if (camera.angles.y > kRadians360) {
        camera.angles.y -= kRadians360;
      } else if (camera.angles.y < 0) {
        camera.angles.y += kRadians360;
      }
    }

    // Zoom.
    if (game.input.mouse.wheel.y != 0) {
      // We actually want to advance a percentage of the distance.
      camera.distance -= game.input.mouse.wheel.y * camera.distance * camera.zoom_speed;
      if (camera.distance < 0.5f)
        camera.distance = 0.5f;
    }

    Update(&camera);


    PerFrameVector<RenderCommand> commands;

    // Clear command.
    ClearFrame clear_frame;
    clear_frame = {};
    clear_frame.color = ToUint32(Color::Blue());
    commands.push_back(std::move(clear_frame));

    // Camera.
    commands.push_back(GetPushCamera(camera));

    commands.push_back(grid.render_command);

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
