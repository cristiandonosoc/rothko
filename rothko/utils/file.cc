// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/utils/file.h"

#include <stdio.h>

#include "rothko/utils/logging.h"

namespace rothko {

bool ReadWholeFile(const std::string& path,
                   std::string* out,
                   bool add_extra_zero) {
  FILE* file;
  size_t file_size;

  file = fopen(path.data(), "rb");
  if (file == NULL) {
    LOG(ERROR, "Could not open file: %s", path.c_str());
    return false;
  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  out->clear();
  int pad = add_extra_zero ? 1 : 0;
  out->resize(file_size + pad);
  auto result = fread(out->data(), 1, file_size, file);
  if (result != file_size) {
    LOG(ERROR, "Could not read file: %s", path.c_str());
    return false;
  }

  fclose(file);
  if (add_extra_zero)
    out->back() = '\0';

  return true;
}

FileHandle::~FileHandle() {
  if (hndl.has_value())
    CloseFile(this);
}

FileHandle OpenFile(const std::string_view& path, bool append) {
  FileHandle handle;
  FILE* file = fopen(path.data(), append ? "a" : "w+");
  if (file == NULL)
    return handle;

  handle.hndl = (void*)file;
  return handle;
}

void WriteToFile(FileHandle* handle, void* data, size_t size) {
  ASSERT(Valid(handle));

  size_t res = fwrite(data, sizeof(char), size, (FILE*)handle->hndl.value);
  ASSERT(res == size);
}

void Flush(FileHandle* handle) {
  fflush((FILE*)handle->hndl.value);
}

void CloseFile(FileHandle* handle) {
  ASSERT(Valid(handle));
  int res = fclose((FILE*)handle->hndl.value);
  ASSERT(res == 0);
  handle->hndl.clear();
}

}  // namespace rothko
