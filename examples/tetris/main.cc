// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/widgets/widgets.h>
#include <rothko/scene/camera.h>

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

  constexpr float kDistance = 14;
  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({kTetrisSizeX / 2, kTetrisSizeY / 2, kDistance},
                                               {kTetrisSizeX / 2, kTetrisSizeY / 2 + 2, 0},
                                               ToRadians(60.0f),
                                               aspect_ratio);

  auto tetris = InitTetris(game.renderer.get());
  if (!tetris)
    return 1;

  WindowEvent frame_event = WindowEvent::kNone;
  while (DefaultGameFrame(&game, &frame_event)) {
    DefaultUpdateOrbitCamera(game.input, &camera);

    auto tetris_render = Update(tetris.get(), game.renderer.get(), game.input);

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Graycc()));

    commands.push_back(GetPushCamera(camera));

    commands.push_back(grid.render_command);
    commands.push_back(std::move(tetris_render));

    commands.push_back(PopCamera());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));
    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
