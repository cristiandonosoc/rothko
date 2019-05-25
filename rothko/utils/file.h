// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "rothko/utils/logging.h"
#include "rothko/utils/clear_on_move.h"

namespace rothko {

// Reads a complete file as binary data into |out|.
// If |add_extra_zero| is true, it will append a '\0' character to the end of
// the data.
bool ReadWholeFile(const std::string& path,
                   std::string* out,
                   bool add_extra_zero = true);

struct FileHandle {
  RAII_CONSTRUCTORS(FileHandle);
  ClearOnMove<void*> hndl;
};

inline bool Valid(FileHandle* file) { return file->hndl.has_value(); }

FileHandle OpenFile(const std::string_view& path, bool append = false);
void WriteToFile(FileHandle*, void* data, size_t size);
void Flush(FileHandle*);
void CloseFile(FileHandle*);


}  // namespace rothko
