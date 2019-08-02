// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>
#include <string>

#include "rothko/utils/macros.h"

namespace rothko {

// The platform is meant to give an interface to OS specific functionality in an uniform way.
// Each platform should compile its own implementation of this interface.
// The implementations are in rothko/platform/<PLATFORM>_platform.cc.
// (eg. rothko/platform/windows_platform.cc).

struct PlatformHandle {
  PlatformHandle();
  ~PlatformHandle();
  DELETE_MOVE_AND_ASSIGN(PlatformHandle);
  DELETE_COPY_AND_ASSIGN(PlatformHandle);
};

std::unique_ptr<PlatformHandle> InitializePlatform();

std::string GetBasePath();
std::string GetCurrentExecutablePath();
std::string GetCurrentExecutableDirectory();

}  // namespace rothko
