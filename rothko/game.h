// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/graphics.h"
#include "rothko/input/input.h"
#include "rothko/logging/logging.h"
#include "rothko/platform/platform.h"
#include "rothko/window/window.h"

namespace rothko {

// Game is a shorthand way to initialize common things every game needs: window, renderer, input
// and whatnot.

struct Game {
  std::unique_ptr<PlatformHandle> platform_handle;
  std::unique_ptr<LoggerHandle> log_handle;
  std::unique_ptr<Renderer> renderer;
  Window window;
  Input input;
  Time time;
};

// WindowType and RendererType must be compatible (ie. both OpenGL, Vulkan, etc.).
bool InitGame(Game*, InitWindowConfig*, bool log_to_stdout);

WindowEvent StartFrame(Game*);

// Returns false if the game loop should exit.
bool DefaultGameFrame(Game*, WindowEvent* out_event);

}  // namespace
