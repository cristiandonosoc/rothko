// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <filesystem>

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

// List Directory ----------------------------------------------------------------------------------

namespace {

bool IsDir(const std::filesystem::path& path, bool* error = nullptr) {
  std::error_code err;
  bool is_dir = std::filesystem::is_directory(path, err);
  if (err) {
    fprintf(stderr, "Could not open %s: %s.\n", path.string().c_str(), err.message().c_str());
    if (error)
      *error = true;
    return false;
  }

  if (error)
    *error = false;
  return is_dir;
}

bool ShouldFilterExtension(const std::filesystem::path& path, const std::string& extension) {
  if (extension.empty())
    return false;

  auto ext = path.extension().string();
  if (ext.empty())
    return true;

  if (ext[0] == '.')
    ext = ext.substr(1);
  return ext != extension;
}

}  // namespace

bool ListDirectory(const std::string& p,
                   std::vector<DirectoryEntry>* out,
                   const std::string& extension) {
  std::filesystem::path path(p);
  if (!IsDir(path))
    return false;

  out->clear();
  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    auto entry_path = entry.path();

    bool has_error = false;
    bool is_dir = IsDir(entry_path, &has_error);
    if (has_error)
      continue;

    if (ShouldFilterExtension(entry_path, extension))
      continue;

    DirectoryEntry dir_entry = {};
    dir_entry.is_dir = is_dir;
    dir_entry.path = entry_path.string();

    out->push_back(std::move(dir_entry));
  }

  return true;
}

std::string GetBasePath(const std::string& p) {
  std::filesystem::path path(p);
  bool error = false;
  if (IsDir(path, &error))
    return p;

  if (error)
    return {};

  return path.parent_path().string();
}

// Time --------------------------------------------------------------------------------------------

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
