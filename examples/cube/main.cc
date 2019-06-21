// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/graphics/graphics.h>
#include <rothko/utils/logging.h>
#include <rothko/window/window.h>

#include <rothko/window/sdl/sdl_definitions.h>

using namespace rothko;

namespace {

bool Setup(Window*, Renderer*);

}  // namespace


int main() {
  Window window;
  Renderer renderer;
  if (!Setup(&window, &renderer))
    return 1;

  Input input = {};

  // We load a texture.
  Texture texture;
  if (!STBLoadTexture("out/wall.jpg", TextureType::kRGBA, &texture))
    return 1;

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

namespace {

bool Setup(Window* window, Renderer* renderer) {
  // Window.
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  if (!InitWindow(window, &window_config)) {
    LOG(ERROR, "Could not initialize window. Exiting.");
    return false;
  }

  // Renderer.
  InitRendererConfig renderer_config = {};
  renderer_config.type = RendererType::kOpenGL;
  renderer_config.window = window;
  if (!InitRenderer(renderer, &renderer_config)) {
    LOG(ERROR, "Could not initialize the renderer. Exiting.");
    return false;
  }

  return true;
}

}  // namespace
