// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/platform/platform.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <mach-o/dyld.h>
#include <sys/errno.h>
#include <AppKit/AppKit.h>

namespace rothko {

// Initialize --------------------------------------------------------------------------------------

// Initialization in macOS is a no-op.

PlatformHandle::PlatformHandle() = default;
PlatformHandle::~PlatformHandle() = default;

std::unique_ptr<PlatformHandle> InitializePlatform() {
  return std::make_unique<PlatformHandle>();
}

// Paths -------------------------------------------------------------------------------------------

std::string GetCurrentExecutablePath() {
  char buf[256];
  uint32_t bufsize = sizeof(buf);
  int res = _NSGetExecutablePath(buf, &bufsize);
  if (res != 0) {
    fprintf(stderr, "Could not get path to current executable: %s\n", strerror(errno));
    fflush(stderr);
    return {};
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

std::string GetBasePath() {
  std::string exe_path = GetCurrentExecutablePath();
  size_t separator = exe_path.rfind('/');
  if (separator == std::string::npos)
    return exe_path;
  auto base_path = exe_path.substr(0, separator);
  return JoinPaths({std::move(base_path), ".."});
}

uint64_t GetNanoseconds() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec * 1000000000 + now.tv_nsec;
}

// Dialogs -----------------------------------------------------------------------------------------

std::string OpenFileDialog() {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];
  NSOpenPanel *dialog = [NSOpenPanel openPanel];
  [dialog setAllowsMultipleSelection:NO];

  std::string result;
  if ([dialog runModal] == NSModalResponseOK) {
    NSURL *url = [dialog URL];
    result = [[url path] UTF8String];
  }

  [pool release];
  [keyWindow makeKeyAndOrderFront:nil];

  return result;
}

}  // namespace rothko
