// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/window/common/window.h"
#include "rothko/window/common/window_backend.h"

#include <unordered_map>

#include "rothko/utils/logging.h"

namespace rothko {

Window::~Window() {
  if (Valid(this))
    ShutdownWindow(this);
}

// Backend Suscription ---------------------------------------------------------

namespace {

using FactoryMap =
    std::unordered_map<WindowBackendType, WindowBackendFactoryFunction>;

FactoryMap* GetFactoryMap() {
  static FactoryMap factory_map;
  return &factory_map;
}

std::unique_ptr<WindowBackend>
CreateWindowBackend(WindowBackendType type) {
  FactoryMap* factory_map = GetFactoryMap();
  auto it = factory_map->find(type);
  ASSERT(it != factory_map->end());

  WindowBackendFactoryFunction factory = it->second;
  return factory();
}

}  // namespace

void SuscribeWindowBackendFactoryFunction(
    WindowBackendType type, WindowBackendFactoryFunction factory) {
  FactoryMap* factory_map = GetFactoryMap();
  ASSERT(factory_map->find(type) == factory_map->end());
  factory_map->insert({type, factory});
}

// InitWindow ------------------------------------------------------------------

namespace {

void Reset(Window* window) {
  window->backend_type = WindowBackendType::kLast;
  window->backend.reset();
}

}  // namespace

bool InitWindow(Window* window, WindowBackendType type,
                InitWindowConfig* config) {
  ASSERT(type != WindowBackendType::kLast);

  window->backend = CreateWindowBackend(type);
  bool success = window->backend->Init(window, config);
  if (!success)
    Reset(window);
  return success;
}

// Shutdown --------------------------------------------------------------------

void ShutdownWindow(Window* window) {
  ASSERT(Valid(window));
  window->backend->Shutdown();
  Reset(window);
}

// UpdateWindow ----------------------------------------------------------------

std::vector<WindowEvent> NewFrame(Window* window, Input* input) {
  ASSERT(Valid(window));
  return window->backend->NewFrame(window, input);
}

// WindowSwapBuffers -----------------------------------------------------------

void WindowSwapBuffers(Window* window) {
  ASSERT(Valid(window));
  return window->backend->SwapBuffers();
}

// Vulkan Extensions -----------------------------------------------------------

std::vector<const char*>
WindowGetVulkanInstanceExtensions(Window* window) {
  ASSERT(Valid(window));
  return window->backend->GetVulkanInstanceExtensions();
}

// |vk_instance| & |surface_khr| must be casted to the right type in the
// implementation. This is so that we don't need to forward declare vulkan
// typedefs.
bool WindowCreateVulkanSurface(Window* window, void* vk_instance,
                                      void* surface_khr) {
  ASSERT(Valid(window));
  return window->backend->CreateVulkanSurface(vk_instance, surface_khr);
}

// Misc ------------------------------------------------------------------------

const char* ToString(WindowBackendType type) {
  switch (type) {
    case WindowBackendType::kSDLOpenGL: return "SDLOpenGL";
    case WindowBackendType::kSDLVulkan: return "SDLVulkan";
    case WindowBackendType::kLast: return "Last";
  }

  NOT_REACHED("Unknown backend type: %u", (uint32_t)type);
  return "<unknown>";
}

}  // namespace rothko
