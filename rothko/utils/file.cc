// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/utils/file.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <cstring>

#include "rothko/utils/defer.h"
#include "rothko/utils/strings.h"

namespace rothko {

namespace {

constexpr char kCharsToTrim[] = "\t\r\n ";

}  // namespace

bool ReadWholeFile(const std::string& path, std::string* out, bool add_extra_zero) {
  FILE* file;
  size_t file_size;

  auto trimmed_path = Trim(path, kCharsToTrim);
  file = fopen(trimmed_path.c_str(), "rb");
  if (file == NULL) {
    printf("Could not open file %s: %s\n", trimmed_path.c_str(), strerror(errno));
    fflush(stdout);
    return false;
  }

  DEFER([file]() { fclose(file); });

  // Get the file size.
  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  out->clear();
  int pad = add_extra_zero ? 1 : 0;
  out->resize(file_size + pad);
  auto result = fread(out->data(), 1, file_size, file);
  if (result != file_size) {
    printf("[%s:%d]: Error reading file %s. Expected: %zu, Got: %zu\n", __FUNCTION__, __LINE__,
                                                                        path.c_str(),
                                                                        file_size,
                                                                        result);
    fflush(stdout);
    return false;
  }

  if (add_extra_zero)
    out->back() = '\0';

  return true;
}

bool ReadWholeFile(const std::string& path, std::vector<uint8_t>* out) {
  FILE* file;
  size_t file_size;

  auto trimmed_path = Trim(path, kCharsToTrim);
  file = fopen(trimmed_path.c_str(), "rb");
  if (file == NULL) {
    printf("Could not open file %s: %s\n", trimmed_path.c_str(), strerror(errno));
    fflush(stdout);
    return false;
  }

  DEFER([file]() { fclose(file); });

  // Get the file size.
  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  out->clear();
  out->resize(file_size);
  auto result = fread(out->data(), 1, file_size, file);
  if (result != file_size) {
    printf("[%s:%d]: Error reading file %s. Expected: %zu, Got: %zu\n", __FUNCTION__, __LINE__,
                                                                        path.c_str(),
                                                                        file_size,
                                                                        result);
    fflush(stdout);
    return false;
  }

  return true;
}

FileHandle::~FileHandle() {
  if (hndl.has_value())
    CloseFile(this);
}

FileHandle OpenFile(const std::string& path, bool append) {
  FileHandle handle;

  auto trimmed_path = Trim(path, kCharsToTrim);
  FILE* file = fopen(trimmed_path.c_str(), append ? "a" : "w+");
  if (file == NULL) {
    printf("Could not open file %s: %s\n", trimmed_path.c_str(), strerror(errno));
    fflush(stdout);
    return handle;
  }

  handle.hndl = (void*)file;
  return handle;
}

uint32_t WriteToFile(FileHandle* handle, void* data, size_t size) {
  assert(Valid(*handle));
  size_t res = fwrite(data, size, 1, (FILE*)handle->hndl.value);
  assert(res == 1);
  return size;
}

void Flush(FileHandle* handle) {
  fflush((FILE*)handle->hndl.value);
}

void CloseFile(FileHandle* handle) {
  assert(Valid(*  handle));
  int res = fclose((FILE*)handle->hndl.value);
  assert(res == 0);
  handle->hndl.clear();
}

}  // namespace rothko
