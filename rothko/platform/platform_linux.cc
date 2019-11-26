// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "rothko/platform/platform.h"

namespace rothko {

// Initialize --------------------------------------------------------------------------------------

// Initialization in linux is a no-op.

PlatformHandle::PlatformHandle() = default;
PlatformHandle::~PlatformHandle() = default;

std::unique_ptr<PlatformHandle> InitializePlatform() {
  return std::make_unique<PlatformHandle>();
}

// Paths -------------------------------------------------------------------------------------------

std::string GetCurrentExecutablePath() {
  char buf[1024];
  int res = readlink("/proc/self/exe", buf, sizeof(buf));
  if (res < 0) {
    fprintf(stderr, "Could not get path to current executable: %s\n",
            strerror(errno));
    fflush(stderr);
    return std::string();
  }

  return buf;
}

std::string GetCurrentExecutableDirectory() {
  std::string exe_path = GetCurrentExecutablePath();
  size_t separator = exe_path.rfind('/');
  if (separator == std::string::npos)
    return exe_path;
  return exe_path.substr(0, separator);
}

uint64_t GetNanoseconds() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec * 1000000000 + now.tv_nsec;
}

// Dialogs -----------------------------------------------------------------------------------------

std::string OpenFileDialog() {
  char filename[1024];
  FILE* f = popen("zenity --file-selection", "r");
  fgets(filename, 1024, f);
  if (pclose(f) != 0)
    return {};

  // Zenity adds a spurious '\n' at the end that we need to remove.
  std::string path = filename;
  printf("PATH: %s\n", path.c_str());
  if (path.back() == '\n')
    return path.substr(0, path.size() - 1);
  return filename;
}

}  // namespace rothko
