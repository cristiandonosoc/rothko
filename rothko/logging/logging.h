// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <atomic>
#include <memory>
#include <string>

#include "rothko/utils/location.h"
#include "rothko/utils/macros.h"

namespace rothko {

// Log Categories ----------------------------------------------------------------------------------
//
// Categories to better determine what a particular log is talking about.

enum class LogCategory : uint32_t {
  kApp,
  kFatal,
  kImgui,
  kGraphics,    // Common graphics functionality.
  kOpenGL,      // OpenGL renderer sub-system.
  kLast,
};
const char* ToString(LogCategory);

enum class LogSeverity : uint32_t {
  kInfo,
  kWarning,
  kError,
  kAssert,
};
const char* ToString(LogSeverity);

// Shutdowns the logger system at destruction. There should only be one active at a time.
struct LoggerHandle {
  LoggerHandle();
  ~LoggerHandle();

  DELETE_COPY_AND_ASSIGN(LoggerHandle);
  DELETE_MOVE_AND_ASSIGN(LoggerHandle);
};

// Keeps alive the logging system. Treat as singleton.
std::unique_ptr<LoggerHandle> InitLoggingSystem(bool log_to_stdout);

struct LogTime {
  int hours;
  int minutes;
  int seconds;
  /* int milliseconds; */
  int micros;

  uint64_t nanos;
};

struct LogEntry {
  LogTime log_time;
  LogCategory category;
  LogSeverity severity;
  Location location = {};
  std::string msg;
};

struct LogContainer {
  static constexpr int kMaxEntries = 4096;

  LogContainer() = default;
  ~LogContainer() = default;
  DELETE_COPY_AND_ASSIGN(LogContainer);
  DELETE_MOVE_AND_ASSIGN(LogContainer);

  std::atomic<uint64_t> write_index = 0;
  LogEntry entries[kMaxEntries] = {};
};
const LogContainer& GetLogs();

void DoLogging(LogCategory category, LogSeverity severity, Location, const char* fmt, ...)
    PRINTF_FORMAT(4, 5);

#undef ERROR

#if DEBUG_MODE

#define LOG(category, ...) INTERNAL_LOG(category, Info, __VA_ARGS__)
#define WARNING(category, ...) INTERNAL_LOG(category, Warning, __VA_ARGS__)
#define ERROR(category, ...) INTERNAL_LOG(category, Error, __VA_ARGS__)

#define ASSERT(condition)                                              \
  do {                                                                 \
    if (!(condition)) {                                                \
      INTERNAL_LOG(Fatal, Assert, "Condition failed: %s", #condition); \
      SEGFAULT();                                                      \
    }                                                                  \
  } while (false)

#define ASSERT_MSG(condition, ...)                                     \
  do {                                                                 \
    if (!(condition)) {                                                \
      INTERNAL_LOG(Fatal, Assert, "Condition failed: %s", #condition); \
      INTERNAL_LOG(Fatal, Assert, __VA_ARGS__);                        \
      SEGFAULT();                                                      \
    }                                                                  \
  } while (false)

#define NOT_REACHED()                            \
  do {                                           \
    INTERNAL_LOG(Fatal, Assert, "Invalid path"); \
    SEGFAULT();                                  \
  } while (false)

#define NOT_REACHED_MSG(...)                     \
  do {                                           \
    INTERNAL_LOG(Fatal, Assert, "Invalid path"); \
    INTERNAL_LOG(Fatal, Assert, __VA_ARGS__)     \
    SEGFAULT();                                  \
  } while (false)

#define NOT_IMPLEMENTED()                           \
  do {                                              \
    INTERNAL_LOG(Fatal, Assert, "Not implemented"); \
    SEGFAULT();                                     \
  } while (false)

// You shouldn't be calling this directly.
#define INTERNAL_LOG(category, severity, ...)                                                     \
  {                                                                                               \
    constexpr Location location{                                                                  \
        StrAfterToken(__FILE__, FILEPATH_SEPARATOR), __LINE__, StrAfterToken(__FUNCTION__, ':')}; \
    ::rothko::DoLogging(::rothko::LogCategory::k##category,                                       \
                        ::rothko::LogSeverity::k##severity,                                       \
                        location VA_ARGS(__VA_ARGS__));                                           \
  }

#else

#define LOG(category, ...) do {} while (false);
#define WARNING(category, ...) do {} while (false);
#define ERROR(category, ...) do {} while (false);

#define ASSERT(condition) do {} while (false);
#define ASSERT_MSG(condition, ...) do {} while (false);

#define NOT_REACHED() do {} while (false);
#define NOT_REACHED_MSG(...) do {} while (false);

#define NOT_IMPLEMENTED() do {} while (false);

#endif

}  // namespace rothko
