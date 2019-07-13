// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/logging/logging.h"

#include <stdarg.h>

#include <atomic>
#include <thread>

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

// DoLogging ---------------------------------------------------------------------------------------

namespace {

std::atomic<bool> gLoggerExists    = false;
uint64_t gReaderIndex              = 0;
std::atomic<bool> gRunning         = false;
std::atomic<uint64_t> gWriterIndex = 0;

struct LogMessage {
  uint32_t log_category;
  Location location;
  std::string msg;
};

LogMessage gLogMessages[1024] = {};

void OutputLogMessage(const LogMessage& message) {
  return;

  // TODO(Cristian): Add time.
  fprintf(stderr, "[%s][%s:%d][%s] %s\n", LogCategoryToString(message.log_category),
                                          message.location.file,
                                          message.location.line,
                                          message.location.function,
                                          message.msg.c_str());
  fflush(stderr);
}

void LoggingLoop() {
  while (gRunning) {
    // Output all messages currently not written in this logger iteration.
    uint64_t writer_index = gWriterIndex;
    while (gReaderIndex < writer_index) {
      uint64_t reader_index = gReaderIndex % ARRAY_SIZE(gLogMessages);

      OutputLogMessage(gLogMessages[reader_index]);

      gReaderIndex++;
    }

    // We sleep until the next logging iteration.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
};

}  // namespace

void DoLogging(int32_t category, Location location, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  auto msg = StringPrintfV(fmt, va);
  va_end(va);

  uint64_t writer_index = (gWriterIndex++) % ARRAY_SIZE(gLogMessages);
  auto& message = gLogMessages[writer_index];

  message.log_category = category;
  message.location = std::move(location);
  message.msg = std::move(msg);

  // TODO(Cristian): Add time.
  fprintf(stderr, "[%s][%s:%d][%s] %s\n", LogCategoryToString(message.log_category),
                                          message.location.file,
                                          message.location.line,
                                          message.location.function,
                                          message.msg.c_str());
  fflush(stderr);



}

// Logger ------------------------------------------------------------------------------------------

namespace {

std::thread gLoggingThread;

}  // namespace

Logger Logger::CreateLogger() {
  if (gLoggerExists) {
    fprintf(stderr, "Logger already created! Only one logger can exists!\n");
    fflush(stderr);
    exit(1);
  }
  gLoggerExists = true;
  gRunning = true;

  Logger logger;

  // Create the thread.
  gLoggingThread = std::thread(LoggingLoop);

  logger.valid_ = true;

  return logger;
}

Logger::Logger() = default;

Logger::~Logger() {
  if (!valid_)
    return;
  valid_ = false;

  // Stop the other thread.
  gRunning = false;
  gLoggingThread.join();
}

// Move.
Logger::Logger(Logger&& other) {
  valid_ = other.valid_;
  other.valid_ = false;
}

Logger& Logger::operator=(Logger&& other) {
  valid_ = other.valid_;
  other.valid_ = false;

  return *this;
}

}  // namespace rothko
