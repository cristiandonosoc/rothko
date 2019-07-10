// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/graphics/graphics.h>
#include <rothko/logging/logging.h>
#include <rothko/window/window.h>

#include <thread>

using namespace rothko;

int main() {
  // Window.
  Window window = {};
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  if (!InitWindow(&window, &window_config)) {
    LOG(ERROR, "Could not initialize window. Exiting.");
    return 1;
  }

  // Renderer.
  Renderer renderer = {};
  InitRendererConfig renderer_config = {};
  renderer_config.type = RendererType::kOpenGL;
  renderer_config.window = &window;
  if (!InitRenderer(&renderer, &renderer_config)) {
    LOG(ERROR, "Could not initialize the renderer. Exiting.");
    return 1;
  }

  Input input = {};

  // Sample game loop.
  bool running = true;
  while (running) {
    auto events = NewFrame(&window, &input);
    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    StartFrame(&renderer);

    EndFrame(&renderer);

    /* std::this_thread::sleep_for(std::chrono::milliseconds(16)); */
  }
}
