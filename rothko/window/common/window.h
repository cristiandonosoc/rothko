// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>
#include <vector>

#include "rothko/utils/macros.h"

namespace rothko {

// Window
// =============================================================================
//
// This is the abstraction Rothko provides for handling different window
// managers. From the point of view of the caller, there is no knowledge of what
// is being used to actually create/handle the windows.
//
// This abstraction is done by the usage of WindowBackends, which define a
// common interface of what a particular window manager has to provide. Each
// implementation backend (SDL, GLFW, Windows, etc.) must implement that
// interface and add itself to the WindowBackendType below.
//
// The way the code gets an instace of a particular backend is by using factory
// functions. Rothko will maintain a map of functions it can use to create an
// instance of a particular backend. Each backend must suscribe their factory
// function in order to work. See the definitions of the functions below.
//
// See also rothko/window/common/window_backend.h for more details.

struct Input;
struct WindowBackend;

enum class WindowBackendType {
  kSDLOpenGL,
  // kSDLVulkan,  TODO(Cristian): Implement back!
  kLast,
};
const char* ToString(WindowBackendType);

// Backend Suscription ---------------------------------------------------------

// Each backend, upon application startup, must suscribe a function that will
// be called to create a that particular WindowBackend.
using WindowBackendFactoryFunction = std::unique_ptr<WindowBackend> (*)();
void SuscribeWindowBackendFactoryFunction(WindowBackendType,
                                          WindowBackendFactoryFunction);

struct Window {
  RAII_CONSTRUCTORS(Window);

  int width = 0;
  int height = 0;

  static constexpr size_t kMaxUtf8Chars = 255;
  char utf8_chars_inputted[kMaxUtf8Chars + 1];  // For the extra zero.
  int utf8_index = 0;

  WindowBackendType backend_type = WindowBackendType::kLast;
  std::unique_ptr<WindowBackend> backend;
};

// API -------------------------------------------------------------------------

inline bool Valid(Window* wm) { return !!wm->backend; }

struct InitWindowConfig {
  bool borderless = false;
  bool fullscreen = false;
  bool hidden = false;
  bool resizable = false;

  // Mutual exclusive, maximized wins.
  bool minimized = false;
  bool maximized = false;
};
bool InitWindow(Window*, WindowBackendType, InitWindowConfig*);

// Will be called on destructor if window manager is valid.
void ShutdownWindow(Window*);

// Gets the window events and calculate inputs.
// TODO(Cristian): Once the container situation has been figured out, return
//                 these as a container instead of an out array.
enum class WindowEvent : uint32_t {
  kQuit,
  kWindowResize,
  kLast,
};
std::vector<WindowEvent> NewFrame(Window*, Input*);

// Some window managers have an explicit call for swapping buffers, notably
// OpenGL. Since each window manager is different, we let the backend take
// care of that functionality.
//
// In renderers that don't need this, this will be a no-op.
//
// NOTE: If v-sync is enabled, this will block on it.
void WindowSwapBuffers(Window*);

// *** VULKAN SPECIFIC ***
//
// Call it only on Window that have a backend that support these vulkan
// functions. See window/common/window_manager_backend.h for more details.

// Gets the extension that the window manager needs to work with vulkan.
std::vector<const char*> WindowGetVulkanInstanceExtensions(Window*);

// |vk_instance| & |surface_khr| must be casted to the right type in the
// implementation. This is so that we don't need to forward declare vulkan
// typedefs.
bool WindowCreateVulkanSurface(Window*, void* vk_instance, void* surface_khr);

}  // namespace rothko
