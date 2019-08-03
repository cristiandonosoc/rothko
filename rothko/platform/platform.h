// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "rothko/utils/macros.h"

namespace rothko {

// Platform Initialization -------------------------------------------------------------------------

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

// Paths -------------------------------------------------------------------------------------------

std::string GetBasePath();
std::string GetCurrentExecutablePath();
std::string GetCurrentExecutableDirectory();

// Returns a concatenate of path parts joined together.
std::string JoinPaths(const std::vector<std::string>& paths);

// Strip everything up to and including the last filepath separator.
std::string GetBasename(const std::string& path);

// Timing ------------------------------------------------------------------------------------------

// Amount of nanoseconds since the program started.
// Implementation is in each platform implementation (<PLATFORM>_platform.cc).
// See rothko/platform/platform.h for more details.
uint64_t GetNanoseconds();

struct Time {
  // Represents the time since |Init| was called.
  uint64_t total_time = 0;    // In nanoseconds.
  float seconds = 0;

  float frame_delta = 0;

  float frame_delta_average = 0;
  float frame_rate = 0;

  // Amount of frames to keep track of in order to get an average frame time.
  static constexpr int kFrameTimesCounts = 128;

  // TODO(Cristian): When we're interested, start tracking these times.
  float frame_deltas[kFrameTimesCounts] = {};
  int frame_deltas_index = 0;
};

Time InitTime();
void Update(Time*);

// System Dialogs ----------------------------------------------------------------------------------

// Returns empty string on cancel.
std::string OpenFileDialog();

}  // namespace rothko
