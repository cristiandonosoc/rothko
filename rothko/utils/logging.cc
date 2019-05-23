// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/utils/logging.h"

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

}  // namespace rothko
