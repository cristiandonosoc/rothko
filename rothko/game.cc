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

  // Initial config of the renderer.
  PushConfig initial_config = {};
  initial_config.viewport_pos = {};
  initial_config.viewport_size = game->window.screen_size;
  RendererExecuteCommands(game->renderer.get(), {initial_config});

  return true;
}

WindowEvent StartFrame(Game* game) {
  WindowEvent event = StartFrame(&game->window, &game->input);
  Update(&game->time);
  RendererStartFrame(game->renderer.get());
  return event;
}

bool DefaultGameFrame(Game* game, WindowEvent* out_event) {
  *out_event = StartFrame(game);
  if (*out_event == WindowEvent::kQuit)
    return false;

  if (KeyUpThisFrame(game->input, Key::kEscape)) {
    *out_event = WindowEvent::kQuit;
    return false;
  }

  return true;
}

}  // namespace rothko
