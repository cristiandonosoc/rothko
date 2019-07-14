// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/window/common/window_backend.h"

#include "rothko/logging/logging.h"

namespace rothko {

std::vector<const char*>
WindowBackend::GetVulkanInstanceExtensions() {
  NOT_REACHED_MSG("This function must be subclassed or not called at all!");
  return {};
}

bool WindowBackend::CreateVulkanSurface(void*, void*) {
  NOT_REACHED_MSG("This function must be subclassed or not called at all!");
  return false;
}

}  // namespace rothko
