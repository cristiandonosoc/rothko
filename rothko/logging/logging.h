// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <atomic>
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

struct LogEntry {
  uint64_t nanoseconds = 0;
  uint32_t log_category = UINT32_MAX;
  std::string location;
  std::string msg;
};

struct LogContainer {
  static constexpr int kMaxEntries = 4096;
  static void Init();

  static LogContainer* Get();

  std::atomic<uint64_t> write_index = 0;

  LogEntry entries[kMaxEntries] = {};

  DELETE_COPY_AND_ASSIGN(LogContainer);
  DELETE_MOVE_AND_ASSIGN(LogContainer);

 private:
  LogContainer();
  ~LogContainer();
};

void DoLogging(int32_t category, Location, const char *fmt, ...)
    PRINTF_FORMAT(3, 4);

#if DEBUG_MODE

#define LOG(level, ...) ::rothko::DoLogging(::kLogCategory_##level, FROM_HERE VA_ARGS(__VA_ARGS__));

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
