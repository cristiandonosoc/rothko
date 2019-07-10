// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "rothko/utils/location.h"
#include "rothko/utils/macros.h"

// Log Categories ----------------------------------------------------------------------------------
//
// Categories to better determine what a particular log is talking about.

constexpr uint32_t kLogCategory_DEBUG = 0;
constexpr uint32_t kLogCategory_INFO = 2;
constexpr uint32_t kLogCategory_WARNING = 3;
constexpr uint32_t kLogCategory_ERROR = 4;
constexpr uint32_t kLogCategory_ASSERT = 5;
constexpr uint32_t kLogCategory_NO_FRAME = 6;

namespace rothko {

struct Logger {
  // Can only be called once.
  static Logger CreateLogger();

  ~Logger();

  DELETE_COPY_AND_ASSIGN(Logger);
  DECLARE_MOVE_AND_ASSIGN(Logger);

  private:
   Logger();

   bool valid_ = false;
};

const char* LogCategoryToString(int32_t category);

void DoLogging(int32_t category, Location, const char *fmt, ...)
    PRINTF_FORMAT(3, 4);

#if DEBUG_MODE

#define LOG(level, ...) ::rothko::DoLogging(::kLogCategory_##level, FROM_HERE VA_ARGS(__VA_ARGS__));

#define ASSERT(condition)                            \
  if (!(condition)) {                                \
    LOG(ASSERT, "Condition failed: %s", #condition); \
    SEGFAULT();                                      \
  }

#define ASSERT_MSG(condition, ...)                   \
  if (!(condition)) {                                \
    LOG(ASSERT, "Condition failed: %s", #condition); \
    LOG(ASSERT, __VA_ARGS__);                        \
    SEGFAULT();                                      \
  }

#define NOT_REACHED()          \
  LOG(ASSERT, "Invalid path"); \
  SEGFAULT()

#define NOT_REACHED_MSG(...)   \
  LOG(ASSERT, "Invalid path"); \
  LOG(ASSERT, __VA_ARGS__)     \
  SEGFAULT()

#define NOT_IMPLEMENTED()         \
  LOG(ASSERT, "Not implemented"); \
  SEGFAULT()

#else

#define LOG(level, ...) \
  do {                  \
  } while (false);
#define ASSERT(condition) \
  do {                    \
  } while (false);
#define NOT_REACHED(...) \
  do {                   \
  } while (false);
#define NOT_REACHED_MSG(...) \
  do {                       \
  } while (false);

#endif

}  // namespace rothko