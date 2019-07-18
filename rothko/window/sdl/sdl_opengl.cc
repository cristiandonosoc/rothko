// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/window/sdl/sdl_opengl.h"

#include <memory>

#include "rothko/input/input.h"
#include "rothko/logging/logging.h"
#include "rothko/window/common/window.h"
#include "rothko/window/sdl/sdl_input.h"

namespace rothko {
namespace sdl {

// Backend Suscription -----------------------------------------------------------------------------

namespace {

std::unique_ptr<WindowBackend> CreateWindow() {
  return std::make_unique<SDLOpenGLWindow>();
}

struct BackendSuscriptor {
  BackendSuscriptor() {
    SuscribeWindowBackendFactoryFunction(WindowType::kSDLOpenGL, CreateWindow);
  }
};

// Trigger the suscription.
BackendSuscriptor backend_suscriptor;

} // namespace

// Shutdown ----------------------------------------------------------------------------------------

namespace {

void SDLOpenGLShutdown(SDLOpenGLWindow* sdl) {
  if (sdl->gl_context.has_value()) {
    SDL_GL_DeleteContext(sdl->gl_context.value);
    sdl->gl_context.clear();
  }

  if (sdl->sdl_window.has_value()) {
    SDL_DestroyWindow(sdl->sdl_window.value);
    sdl->sdl_window.clear();
  }

  sdl->window = nullptr;


  if (sdl->cursors[(int)MouseCursor::kArrow])
    SDL_SetCursor(sdl->cursors[(int)MouseCursor::kArrow]);

  for (int i = 0; i < ARRAY_SIZE(sdl->cursors); i++) {
    SDL_Cursor* cursor = sdl->cursors[i];
    sdl->cursors[i] = NULL;
    if (cursor == NULL)
      continue;
    SDL_FreeCursor(cursor);
  }

  SDL_ShowCursor(SDL_TRUE);
}

}  // namespace

void SDLOpenGLWindow::Shutdown() {
  SDLOpenGLShutdown(this);
}

// Init --------------------------------------------------------------------------------------------

namespace {

bool SDLOpenGLInit(SDLOpenGLWindow* sdl, Window* window, InitWindowConfig* config) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    LOG(ERROR, "Error loading SDL: %s", SDL_GetError());
    return false;
  }

#if DEBUG_MODE
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  // Setup SDL flags.
  uint32_t window_flags = SDL_WINDOW_OPENGL;
  if (config->borderless)
    window_flags |= SDL_WINDOW_BORDERLESS;
  if (config->fullscreen)
    window_flags |= SDL_WINDOW_FULLSCREEN;
  if (config->hidden)
    window_flags |= SDL_WINDOW_HIDDEN;
  if (config->resizable)
    window_flags |= SDL_WINDOW_RESIZABLE;
  if (config->minimized)
    window_flags |= SDL_WINDOW_MINIMIZED;
  if (config->maximized) {
    window_flags &= ~SDL_WINDOW_MINIMIZED;  // Remove minimized.
    window_flags |= SDL_WINDOW_MAXIMIZED;
  }

  sdl->sdl_window = SDL_CreateWindow("rothko", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     config->screen_size.x, config->screen_size.y,
                                     window_flags);
  if (!sdl->sdl_window.has_value()) {
    LOG(ERROR, "Error creating window: %s", SDL_GetError());
    SDLOpenGLShutdown(sdl);
    return false;
  }

  // Setup the OpenGL Context.
  sdl->gl_context = SDL_GL_CreateContext(sdl->sdl_window.value);
  if (!sdl->gl_context.has_value()) {
    LOG(ERROR, "Error creating OpenGL context: %s", SDL_GetError());
    SDLOpenGLShutdown(sdl);
    return false;
  }

  /* SDL_GL_SetSwapInterval(1);  // Enable v-sync. */
  SDL_GetWindowSize(sdl->sdl_window.value, &window->width, &window->height);
  LOG(DEBUG, "Window size: %d, %d", window->width, window->height);

  // Mouse cursors.
  sdl->cursors[(int)MouseCursor::kArrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  sdl->cursors[(int)MouseCursor::kIbeam] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
  sdl->cursors[(int)MouseCursor::kWait] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
  sdl->cursors[(int)MouseCursor::kCrosshair] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
  sdl->cursors[(int)MouseCursor::kWaitArrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
  sdl->cursors[(int)MouseCursor::kSizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
  sdl->cursors[(int)MouseCursor::kSizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
  sdl->cursors[(int)MouseCursor::kSizeWE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
  sdl->cursors[(int)MouseCursor::kSizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
  sdl->cursors[(int)MouseCursor::kSizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
  sdl->cursors[(int)MouseCursor::kNo] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
  sdl->cursors[(int)MouseCursor::kHand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

  sdl->window = window;
  return true;
}

}  // namespace

bool SDLOpenGLWindow::Init(Window* w, InitWindowConfig* config) {
  return SDLOpenGLInit(this, w, config);
}

// UpdateWindow ------------------------------------------------------------------------------------

namespace {

void PushEvent(SDLOpenGLWindow* sdl, WindowEvent event) {
  ASSERT(sdl->event_index < ARRAY_SIZE(sdl->events));
  sdl->events[sdl->event_index++] = event;
}

void PushUtf8Char(SDLOpenGLWindow* sdl, char c) {
  Window* window = sdl->window;
  ASSERT(window->utf8_index < ARRAY_SIZE(window->utf8_chars_inputted));
  window->utf8_chars_inputted[window->utf8_index++] = c;
}

void ResetUtf8(Window* window) {
  for (int i = 0; i < ARRAY_SIZE(window->utf8_chars_inputted); i++) {
    window->utf8_chars_inputted[i] = 0;
  }
  window->utf8_index = 0;
}

void HandleWindowEvent(const SDL_WindowEvent& window_event,
                       SDLOpenGLWindow* sdl,
                       Window* window) {
  // Fow now we're interested in window changed.
  if (window_event.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
    PushEvent(sdl, WindowEvent::kWindowResize);
    window->width = window_event.data1;
    window->height = window_event.data2;
  }
}

std::vector<WindowEvent>
SDLOpenGLNewFrame(SDLOpenGLWindow *sdl, Window *window, Input *input) {
  ASSERT(Valid(sdl));

  // Restart the state.
  sdl->event_index = 0;
  ResetUtf8(sdl->window);

  NewFrame(input);  // We do the frame flip.

  // Handle events.
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT: PushEvent(sdl, WindowEvent::kQuit); break;
      case SDL_KEYUP: HandleKeyUpEvent(event.key, input); break;
      case SDL_MOUSEWHEEL: HandleMouseWheelEvent(event.wheel, input); break;
      case SDL_WINDOWEVENT: HandleWindowEvent(event.window, sdl, window); break;
      case SDL_TEXTINPUT: {
        // event.text.text is a char[32].
        for (char c : event.text.text) {
          PushUtf8Char(sdl, c);
          if (c == 0)
            break;
        }
      }
      default: break;
    }
  }

  HandleKeysDown(input);
  HandleMouse(input);

  // Chain the events into a linked list.
  if (sdl->event_index == 0)
    return {};

  std::vector<WindowEvent> event_list;
  event_list.reserve(sdl->event_index);
  for (int i = 0; i < sdl->event_index; i++) {
    event_list.push_back(sdl->events[i]);
  }
  return event_list;
}

}  // namespace

std::vector<WindowEvent>
SDLOpenGLWindow::NewFrame(Window* w, Input* input) {
  return SDLOpenGLNewFrame(this, w, input);
}

// SwapBuffers -------------------------------------------------------------------------------------

void SDLOpenGLWindow::SwapBuffers() {
  SDL_GL_SwapWindow(this->sdl_window.value);
}

// Misc --------------------------------------------------------------------------------------------

SDLOpenGLWindow::~SDLOpenGLWindow() {
  if (Valid(this))
    SDLOpenGLShutdown(this);
}

void SDLOpenGLWindow::SetMouseCursor(MouseCursor cursor) {
  int index = (int)cursor;
  ASSERT(index < (int)MouseCursor::kLast);
  SDL_Cursor* sdl_cursor = this->cursors[index];
  ASSERT(sdl_cursor);
  SDL_SetCursor(sdl_cursor);
}

void SDLOpenGLWindow::ShowCursor(bool show) {
  SDL_ShowCursor(show ? SDL_TRUE : SDL_FALSE);
}

}  // namespace sdl
}  // namespace rothko
