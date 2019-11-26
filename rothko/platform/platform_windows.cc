// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>
#include <objbase.h>
#include <shobjidl.h>
#include <stdlib.h>
#include <windows.h>

#include "rothko/platform/platform.h"
#include "rothko/utils/defer.h"

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
