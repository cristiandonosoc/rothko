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

const char* ToString(LogCategory category) {
  switch (category) {
    case LogCategory::kApp: return "App";
    case LogCategory::kFatal: return "Fatal";
    case LogCategory::kImgui: return "Imgui";
    case LogCategory::kGraphics: return "Graphics";
    case LogCategory::kOpenGL: return "OpenGL";
    case LogCategory::kLast: return "Last";
  }

  NOT_REACHED();
  return "<unknown>";
}

const char* ToString(LogSeverity severity) {
  switch (severity) {
    case LogSeverity::kInfo: return "Info";
    case LogSeverity::kWarning: return "Warning";
    case LogSeverity::kError: return "Error";
    case LogSeverity::kAssert: return "Assert";
  }

  NOT_REACHED();
  return "<unknown>";
}

// LogContainer ------------------------------------------------------------------------------------
//
// Actual struct that holds the logs for the system.
// The system is active while there is an active LoggerHandle.

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

const LogContainer& GetLogs() {
  if (!gLogs)
    SEGFAULT();

  return *gLogs;
}

// DoLogging ---------------------------------------------------------------------------------------

namespace {

void NanoToLogTime(LogTime* time, uint64_t nanos) {

  /* uint64_t micro = nanos / 1000; */
  /* time.microseconds = nanos - micro; */
  /* uint64_t milli = micro / 1000; */
  /* time.milliseconds = micro - milli; */
  /* uint64_t secs = milli / 1000; */

  time->nanos = nanos;
  uint64_t micros = nanos / 1000;
  time->micros = micros % 1000000;
  uint64_t secs = micros / 1000000;
  time->seconds = secs % 60;
  uint64_t min = secs / 60;
  time->minutes = min % 60;
  time->hours = min / 60;
}

void OutputLogMessage(bool to_stdout, const LogEntry& message) {
  // TODO(Cristian): Log to file.
  if (to_stdout) {
    // TODO(Cristian): Add time.
    printf("[%s][%s:%d][%s] %s\n",
           ToString(message.category),
           message.location.file,
           message.location.line,
           message.location.function,
           message.msg.c_str());
    fflush(stdout);
  }
}

}  // namespace

void DoLogging(
    LogCategory category, LogSeverity severity, Location location, const char* fmt, ...) {
  if (!gLoggingActive)
    return;

  va_list va;
  va_start(va, fmt);
  auto msg = StringPrintfV(fmt, va);
  va_end(va);

  // Assert goes to the console.
  if (severity == LogSeverity::kAssert) {
    LogEntry entry = {};
    NanoToLogTime(&entry.log_time, GetNanoseconds());
    entry.category = category;
    entry.severity = severity;
    entry.location = location;
    entry.msg = std::move(msg);

    OutputLogMessage(true, entry);

    return;
  }

  // Insert the logs into the log system.
  int write_index = gLogs->write_index++ % LogContainer::kMaxEntries;

  auto& entry = gLogs->entries[write_index];
  NanoToLogTime(&entry.log_time, GetNanoseconds());
  entry.category = category;
  entry.severity = severity;
  entry.location = std::move(location);
  entry.msg = std::move(msg);
}

}  // namespace rothko
