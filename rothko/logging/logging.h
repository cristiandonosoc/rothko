// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <atomic>
#include <memory>
#include <string>

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

const char* LogCategoryToString(int32_t category);

// Shutdowns the logger system at destruction. There should only be one active at a time.
struct LoggerHandle {
  LoggerHandle();
  ~LoggerHandle();

  DELETE_COPY_AND_ASSIGN(LoggerHandle);
  DELETE_MOVE_AND_ASSIGN(LoggerHandle);
};

// Keeps alive the logging system. Treat as singleton.
std::unique_ptr<LoggerHandle> InitLoggingSystem();

void DoLogging(int32_t category, Location, const char *fmt, ...)
    PRINTF_FORMAT(3, 4);

#if DEBUG_MODE

#define LOG(level, ...)                                                                           \
  {                                                                                               \
    constexpr Location location{                                                                  \
        StrAfterToken(__FILE__, FILEPATH_SEPARATOR), __LINE__, StrAfterToken(__FUNCTION__, ':')}; \
    ::rothko::DoLogging(::kLogCategory_##level, location VA_ARGS(__VA_ARGS__));                   \
  }

#define ASSERT(condition)                              \
  do {                                                 \
    if (!(condition)) {                                \
      LOG(ASSERT, "Condition failed: %s", #condition); \
      SEGFAULT();                                      \
    }                                                  \
  } while (false)

#define ASSERT_MSG(condition, ...)                     \
  do {                                                 \
    if (!(condition)) {                                \
      LOG(ASSERT, "Condition failed: %s", #condition); \
      LOG(ASSERT, __VA_ARGS__);                        \
      SEGFAULT();                                      \
    }                                                  \
  } while (false)

#define NOT_REACHED()            \
  do {                           \
    LOG(ASSERT, "Invalid path"); \
    SEGFAULT();                  \
  } while (false)

#define NOT_REACHED_MSG(...)     \
  do {                           \
    LOG(ASSERT, "Invalid path"); \
    LOG(ASSERT, __VA_ARGS__)     \
    SEGFAULT();                  \
  } while (false)

#define NOT_IMPLEMENTED()           \
  do {                              \
    LOG(ASSERT, "Not implemented"); \
    SEGFAULT();                     \
  } while (false)

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
