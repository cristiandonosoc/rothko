// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>
#include <objbase.h>
#include <shobjidl.h>
#include <stdlib.h>
#include <windows.h>

#include "rothko/platform/platform.h"
#include "rothko/utils/defer.h"

#include <filesystem>

namespace rothko {

// Initialize --------------------------------------------------------------------------------------

namespace {

bool gPlatformInitialized = false;

}  // namespace

PlatformHandle::PlatformHandle() = default;
PlatformHandle::~PlatformHandle() {
  assert(gPlatformInitialized);

  CoUninitialize();
  gPlatformInitialized = false;
}

std::unique_ptr<PlatformHandle> InitializePlatform() {
  assert(!gPlatformInitialized);
  gPlatformInitialized = true;

  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  assert(SUCCEEDED(hr));

  return std::unique_ptr<PlatformHandle>(new PlatformHandle());
}

// Paths -------------------------------------------------------------------------------------------

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

}  // namespace

std::string GetCurrentExecutablePath() {
  // Get the executable handle.
  HMODULE exe_module = GetModuleHandleA(NULL);
  char path[MAX_PATH];
  GetModuleFileName(exe_module, (LPSTR)path, MAX_PATH);

  return path;
}

std::string GetCurrentExecutableDirectory() {
  std::string exe_path = GetCurrentExecutablePath();

  size_t separator = exe_path.rfind('\\');
  if (separator == std::string::npos)
    return exe_path;
  return exe_path.substr(0, separator);
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

// List Directory ----------------------------------------------------------------------------------

namespace {

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

bool IsDirectory(const std::string& path) { return IsDir(path); }

bool ListDirectory(const std::string& p, std::vector<DirectoryEntry>* out,
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

// GetNanoseconds ----------------------------------------------------------------------------------

namespace {

uint64_t GetHighPerformaceCounter() {
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return (uint64_t)counter.QuadPart;
}

uint64_t GetHighPerformaceFrequency() {
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  return (uint64_t)frequency.QuadPart;
}

thread_local uint64_t last_counter = GetHighPerformaceCounter();

}  // namespace

uint64_t GetNanoseconds() {
  uint64_t now = GetHighPerformaceCounter();
  uint64_t diff = now - last_counter;

  // ddiff is in seconds.
  double ddiff = (double)diff / (double)GetHighPerformaceFrequency();
  ddiff *= 1000000000;
  return (uint64_t)ddiff;
}

// Dialogs -----------------------------------------------------------------------------------------

std::string OpenFileDialog() {
  IFileOpenDialog* file_open_dialog;

  // Create the FileOpenDialog object.
  HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
                                NULL,
                                CLSCTX_ALL,
                                IID_IFileOpenDialog,
                                reinterpret_cast<void**>(&file_open_dialog));
  if (!SUCCEEDED(hr))
    return {};

  // Always free the dialog.
  DEFER([file_open_dialog]() { file_open_dialog->Release(); });

  // Show the Open dialog box.
  hr = file_open_dialog->Show(NULL);
  if (!SUCCEEDED(hr))
    return {};

  // Get the file name from the dialog box.
  IShellItem* shell_item;
  hr = file_open_dialog->GetResult(&shell_item);
  if (!SUCCEEDED(hr))
    return {};

  // Always free the shell item.
  DEFER([shell_item]() { shell_item->Release(); });

  // Holder of the selected path.
  wchar_t* filepath = NULL;
  DEFER([filepath]() {
    CoTaskMemFree(filepath);  // NULL does a NO-OP, so it's OK to not check.
  });

  hr = shell_item->GetDisplayName(SIGDN_FILESYSPATH, &filepath);
  if (SUCCEEDED(hr)) {
    // We transform from the horrible PWSTR type.
    char path[512] = {};
    size_t res = wcstombs(path, filepath, sizeof(path));
    assert(res > 0);

    return std::string(path);
  }

  return {};
}

}  // namespace rothko
