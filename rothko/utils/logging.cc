// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/utils/logging.h"

#include "rothko/utils/strings.h"

namespace rothko {

const char* LogCategoryToString(int32_t category) {
  switch (category) {
    case kLogCategory_DEBUG: return "DEBUG";
    case kLogCategory_INFO: return "INFO";
    case kLogCategory_WARNING: return "WARNING";
    case kLogCategory_ERROR: return "ERROR";
    case kLogCategory_ASSERT: return "ASSERT";
    case kLogCategory_NO_FRAME: return "NO_FRAME";
  }

  NOT_REACHED();
  return "<unknown>";
}

// DoLogging -------------------------------------------------------------------

void DoLogging(int32_t category, Location location, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  auto msg = StringPrintfV(fmt, va);
  va_end(va);

  // TODO(Cristian): Add time.
  fprintf(stderr, "[%s][%s:%d] %s\n", LogCategoryToString(category),
          location.file, location.line, msg.c_str());
  fflush(stderr);
}

}  // namespace rothko
