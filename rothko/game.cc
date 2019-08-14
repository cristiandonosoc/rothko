// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/game.h"

namespace rothko {

bool InitGame(Game* game, InitWindowConfig* window_config, bool log_to_stdout) {
  game->platform_handle = InitializePlatform();

  game->log_handle = InitLoggingSystem(log_to_stdout);

  if (!InitWindow(&game->window, window_config)) {
    ERROR(App, "Could not initialize window.");
    return false;
  }

  game->renderer = InitRenderer();
  if (!game->renderer) {
    ERROR(App, "Could not initialize the renderer.");
    return false;
  }

  return true;
}

PerFrameVector<WindowEvent> Update(Game* game) {
  auto events = NewFrame(&game->window, &game->input);
  Update(&game->time);
  RendererStartFrame(game->renderer.get());
  return events;
}

}  // namespace rothko
