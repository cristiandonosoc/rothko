// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/logging/logging.h"

#include <stdarg.h>

#include <atomic>
#include <thread>

#include "rothko/platform/timing.h"
#include "rothko/utils/macros.h"
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

  return "<unknown>";
}

// LogContainer ------------------------------------------------------------------------------------
//
// Actual struct that holds the logs for the system.
// The system is active while there is an active LoggerHandle.

namespace {

struct LogEntry {
  uint64_t nanoseconds = 0;
  uint32_t log_category = UINT32_MAX;
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

}  // namespace

// Logging Loop ------------------------------------------------------------------------------------
//
// Loop that runs on another thread that outputs the logs into stdout.
// TODO(Cristian): Output to a file instead.

namespace {

std::unique_ptr<LogContainer> gLogs = nullptr;
std::atomic<bool> gLoggingActive = false;

// If |to_stdout| is true, it will also log into stdout.
void OutputLogMessage(bool to_stdout, const LogEntry& entry);  // Defined further down.

// Must only be changed by the logging thread.
uint64_t gReaderIndex = 0;

void LoggingLoop() {
  while (true) {
    // Output all messages currently not written in this logger iteration.
    uint64_t writer_index = gLogs->write_index;
    while (gReaderIndex < writer_index) {
      uint64_t reader_index = gReaderIndex % LogContainer::kMaxEntries;

      OutputLogMessage(false, gLogs->entries[reader_index]);
      gReaderIndex++;
    }

    if (!gLoggingActive)
      break;

    // We sleep until the next logging iteration.
    // Should be faster than a frame so that's hard to overflow the buffer.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
};

std::thread gLoggingThread;

}  // namespace

// Logger Handle -----------------------------------------------------------------------------------

std::unique_ptr<LoggerHandle> InitLoggingSystem() {
  if (gLogs) {
    printf("%s:%d -> LOGGING SHOULD NOT BE ACTIVE ON INIT!\n", __FILE__, __LINE__);
    fflush(stdout);
    SEGFAULT();
  }

  // Start the logging loop thread.
  gLoggingActive = true;
  gLogs.reset(new LogContainer());
  gLoggingThread = std::thread(LoggingLoop);


  return std::make_unique<LoggerHandle>();
}

LoggerHandle::LoggerHandle() = default;

LoggerHandle::~LoggerHandle() {
  if (!gLogs) {
    printf("%s:%d -> LOGGING SHOULD BE ACTIVE ON SHUTDOWN!\n", __FILE__, __LINE__);
    fflush(stdout);
    SEGFAULT();
  }

  // Wait for the loop to end.
  gLoggingActive = false;
  gLoggingThread.join();
  gLogs.reset();
}

// DoLogging ---------------------------------------------------------------------------------------

namespace {

void OutputLogMessage(bool to_stdout, const LogEntry& message) {
  // TODO(Cristian): Log to file.
  if (to_stdout) {
    // TODO(Cristian): Add time.
    printf("[%s][%s:%d][%s] %s\n",
           LogCategoryToString(message.log_category),
           message.location.file,
           message.location.line,
           message.location.function,
           message.msg.c_str());
    fflush(stdout);
  }
}

}  // namespace

void DoLogging(int32_t category, Location location, const char* fmt, ...) {
  if (!gLoggingActive)
    return;

  va_list va;
  va_start(va, fmt);
  auto msg = StringPrintfV(fmt, va);
  va_end(va);

  // Assert goes to the console.
  if (category == kLogCategory_ASSERT) {
    LogEntry entry = {};
    entry.nanoseconds = GetNanoseconds();
    entry.log_category = category;
    entry.location = location;
    entry.msg = std::move(msg);

    OutputLogMessage(true, entry);

    return;
  }

  // Insert the logs into the log system.
  int write_index = gLogs->write_index++ % LogContainer::kMaxEntries;

  auto& entry = gLogs->entries[write_index];
  entry.nanoseconds = GetNanoseconds();
  entry.log_category = category;
  entry.location = std::move(location);
  entry.msg = std::move(msg);
}

}  // namespace rothko
