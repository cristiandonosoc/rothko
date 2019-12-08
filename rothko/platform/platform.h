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

std::string GetCurrentExecutablePath();
std::string GetCurrentExecutableDirectory();

// Returns a concatenate of path parts joined together.
std::string JoinPaths(const std::vector<std::string>& paths);

// Strip everything after and including the last filepath separator.
std::string GetBasePath(const std::string&);

// Strip everything up to and including the last filepath separator.
std::string GetBasename(const std::string& path);

bool IsDirectory(const std::string& path);

struct DirectoryEntry {
  bool is_dir = false;
  std::string path;
};
bool ListDirectory(const std::string& path, std::vector<DirectoryEntry>* out,
                   const std::string& extension = {});
inline bool ListDirectory(const DirectoryEntry& entry, std::vector<DirectoryEntry>* out,
                          const std::string& extension = {}) {
  if (!entry.is_dir)
    return false;
  return ListDirectory(entry.path, out, extension);
}

// Timing ------------------------------------------------------------------------------------------

// Amount of nanoseconds since the program started.
// Implementation is in each platform implementation (<PLATFORM>_platform.cc).
// See rothko/platform/platform.h for more details.
uint64_t GetNanoseconds();

// Everything is counted in nanoseconds.
constexpr uint64_t kMicroSecond = 1000u;
constexpr uint64_t kMilliSecond = 1000u * kMicroSecond;
constexpr uint64_t kSecond = 1000u * kMilliSecond;
constexpr uint64_t kMinute = 60u * kSecond;
constexpr uint64_t kHour = 60u * kMinute;
constexpr uint64_t kDay = 24u * kHour;
constexpr uint64_t kWeek = 7 * kDay;

// Struct that tracks how much time has passed since
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
