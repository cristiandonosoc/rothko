// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/widgets/widgets.h>
#include <rothko/scene/camera.h>
#include <rothko/ui/imgui.h>

#include "tetris.h"

using namespace rothko;
using namespace tetris;

int main() {
  Game game = {};
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitGame(&game, &window_config, true))
    return 1;

  Grid grid;
  if (!Init(&grid, game.renderer.get()))
    return 1;

  constexpr float kDistance = 22;
  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  Vec3 target_pos = {kTetrisSizeX / 2, kTetrisSizeY / 2 + 2, 0};
  OrbitCamera camera = OrbitCamera::FromLookAt(target_pos + Vec3{0, 0, kDistance},
                                               target_pos,
                                               ToRadians(60.0f),
                                               aspect_ratio);

  auto tetris = InitTetris(game.renderer.get());
  if (!tetris)
    return 1;

  imgui::ImguiContext imgui;
  if (!Init(game.renderer.get(), &imgui))
    return 1;

  bool show_logs = false;

  WindowEvent frame_event = WindowEvent::kNone;
  while (DefaultGameFrame(&game, &frame_event)) {
    DefaultUpdateOrbitCamera(game.input, &camera);
    BeginFrame(&imgui, &game.window, &game.time, &game.input);

    if (KeyUpThisFrame(game.input, Key::kBackquote))
      show_logs = !show_logs;

    auto tetris_render = Update(tetris.get(), &game);

    if (show_logs)
      rothko::imgui::CreateLogWindow();

    PerFrameVector<RenderCommand> commands;

    PushCommand(&commands, ClearFrame::FromColor(Color::Graycc()));
    PushCommand(&commands,GetPushCamera(camera));
    PushCommand(&commands, grid.render_command);
    PushCommand(&commands, std::move(tetris_render));
    PushCommands(&commands, EndFrame(&imgui));

    commands.push_back(PopCamera());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));
    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
