// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "rothko/utils/location.h"
#include "rothko/utils/macros.h"

// Log Categories --------------------------------------------------------------
//
// Categories to better determine what a particular log is talking about.

constexpr int32_t kLogCategory_DEBUG = 0;
constexpr int32_t kLogCategory_INFO = 2;
constexpr int32_t kLogCategory_WARNING = 3;
constexpr int32_t kLogCategory_ERROR = 4;
constexpr int32_t kLogCategory_ASSERT = 5;
constexpr int32_t kLogCategory_NO_FRAME = 6;

namespace rothko {

const char* LogCategoryToString(int32_t category);

void DoLogging(int32_t category, Location, const char *fmt, ...)
    PRINTF_FORMAT(3, 4);

#if DEBUG_MODE

#define LOG(level, ...)                                                        \
  ::rothko::DoLogging(kLogCategory_##level, FROM_HERE VA_ARGS(__VA_ARGS__));

#define ASSERT(condition)                                                      \
  if (!(condition)) {                                                          \
    LOG(ASSERT, "Condition failed: %s", #condition);                           \
    SEGFAULT();                                                                \
  }

#define NOT_REACHED(...)                                                       \
  LOG(ASSERT, "Invalid path");                                                 \
  SEGFAULT()

#else

#define LOG(level, ...) do {} while (false);
#define ASSERT(condition) do {} while (false);
#define NOT_REACHED(...) do {} while (false);

#endif


}  // namespace rothko
