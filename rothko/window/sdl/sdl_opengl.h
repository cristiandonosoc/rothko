// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/utils/clear_on_move.h"
#include "rothko/utils/macros.h"
#include "rothko/window/sdl/sdl_definitions.h"
#include "rothko/window/common/window_backend.h"

namespace rothko {
namespace sdl {

struct SDLOpenGLWindow : public WindowBackend {
  RAII_CONSTRUCTORS(SDLOpenGLWindow);

  ClearOnMove<SDL_Window*> sdl_window = nullptr;
  ClearOnMove<SDL_GLContext> gl_context = NULL;

  Window* window = nullptr;   // Not owning. Must outlive.

  // Array of events that can happen within a frame.
  int event_index = 0;
  WindowEvent events[4];

  SDL_Cursor* cursors[(int)MouseCursor::kLast] = {};

  // Virtual Interface ---------------------------------------------------------


  bool Init(Window*, InitWindowConfig*) override;
  void Shutdown() override;
  std::vector<WindowEvent> NewFrame(Window*, Input*) override;
  void SwapBuffers() override;

  void ShowCursor(bool) override;
  void SetMouseCursor(MouseCursor) override;
};

inline bool Valid(SDLOpenGLWindow* sdl) {
  return sdl->sdl_window.has_value() && sdl->gl_context.has_value();
}

}  // namespace sdl
}  // namespace rothko
