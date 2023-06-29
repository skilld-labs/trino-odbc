/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modifications Copyright Amazon.com, Inc. or its affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _TIMESTREAM_ODBC_LOG
#define _TIMESTREAM_ODBC_LOG

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "ignite/common/common.h"
#include "ignite/common/include/common/concurrent.h"
#include "timestream/odbc/log_level.h"

using ignite::odbc::common::concurrent::CriticalSection;

#define DEFAULT_LOG_PATH timestream::odbc::Logger::GetDefaultLogPath()

#define WRITE_LOG_MSG(param, logLevel) \
  WRITE_MSG_TO_STREAM(param, logLevel, (std::ostream*)nullptr)

#define WRITE_MSG_TO_STREAM(param, logLevel, logStream)                       \
  {                                                                           \
    std::shared_ptr< timestream::odbc::Logger > p =                           \
        timestream::odbc::Logger::GetLoggerInstance();                        \
    if (p->GetLogLevel() >= logLevel && (p->IsEnabled() || p->EnableLog())) { \
      std::ostream* prevStream = p.get()->GetLogStream();                     \
      if (logStream != nullptr) {                                             \
        /* Override the stream temporarily */                                 \
        p.get()->SetLogStream(logStream);                                     \
      }                                                                       \
      std::unique_ptr< timestream::odbc::LogStream > lstream(                 \
          new timestream::odbc::LogStream(p.get()));                          \
      std::string msg_prefix;                                                 \
      switch (logLevel) {                                                     \
        case timestream::odbc::LogLevel::Type::DEBUG_LEVEL:                   \
          msg_prefix = "DEBUG MSG: ";                                         \
          break;                                                              \
        case timestream::odbc::LogLevel::Type::INFO_LEVEL:                    \
          msg_prefix = "INFO MSG: ";                                          \
          break;                                                              \
        case timestream::odbc::LogLevel::Type::WARNING_LEVEL:                 \
          msg_prefix = "WARNING MSG: ";                                       \
          break;                                                              \
        case timestream::odbc::LogLevel::Type::ERROR_LEVEL:                   \
          msg_prefix = "ERROR MSG: ";                                         \
          break;                                                              \
        default:                                                              \
          msg_prefix = "";                                                    \
      }                                                                       \
      char tStr[1000];                                                        \
      time_t curTime = time(NULL);                                            \
      struct tm* locTime = localtime(&curTime);                               \
      strftime(tStr, 1000, "%T %x ", locTime);                                \
      /* Write the formatted message to the stream */                         \
      *lstream << "TID: " << std::this_thread::get_id() << " " << tStr        \
               << msg_prefix << " "                                           \
               << timestream::odbc::Logger::GetBaseFileName(__FILE__) << ":"  \
               << __LINE__ << " " << __FUNCTION__ << ": " << param;           \
      /* This will trigger the write to stream */                             \
      lstream = nullptr;                                                      \
      if (logStream != nullptr) {                                             \
        /* Restore the stream if it was set */                                \
        p.get()->SetLogStream(prevStream);                                    \
      }                                                                       \
    }                                                                         \
  }

// Debug messages are messages that are useful for debugging
#define LOG_DEBUG_MSG(param) \
  WRITE_LOG_MSG(param, timestream::odbc::LogLevel::Type::DEBUG_LEVEL)

#define LOG_DEBUG_MSG_TO_STREAM(param, logStream)                           \
  WRITE_MSG_TO_STREAM(param, timestream::odbc::LogLevel::Type::DEBUG_LEVEL, \
                      logStream)

// Info messages are messages that document the application flow
#define LOG_INFO_MSG(param) \
  WRITE_LOG_MSG(param, timestream::odbc::LogLevel::Type::INFO_LEVEL)

#define LOG_INFO_MSG_TO_STREAM(param, logStream)                           \
  WRITE_MSG_TO_STREAM(param, timestream::odbc::LogLevel::Type::INFO_LEVEL, \
                      logStream)

// Warning messages display warnings
#define LOG_WARNING_MSG(param) \
  WRITE_LOG_MSG(param, timestream::odbc::LogLevel::Type::WARNING_LEVEL)

#define LOG_WARNING_MSG_TO_STREAM(param, logStream)                           \
  WRITE_MSG_TO_STREAM(param, timestream::odbc::LogLevel::Type::WARNING_LEVEL, \
                      logStream)

// Error messages display errors
#define LOG_ERROR_MSG(param) \
  WRITE_LOG_MSG(param, timestream::odbc::LogLevel::Type::ERROR_LEVEL)

#define LOG_ERROR_MSG_TO_STREAM(param, logStream)                           \
  WRITE_MSG_TO_STREAM(param, timestream::odbc::LogLevel::Type::ERROR_LEVEL, \
                      logStream)

namespace timestream {
namespace odbc {
/* Forward declaration */
class Logger;

/**
 * Helper object providing stream operations for single log line.
 * Writes resulting string to Logger object upon destruction.
 */
class IGNITE_IMPORT_EXPORT LogStream : public std::basic_ostream< char > {
 public:
  /**
   * Constructor.
   * @param parent pointer to Logger.
   */
  LogStream(Logger* parent);

  /**
   * Conversion operator helpful to determine if log is enabled
   * @return True if logger is enabled
   */
  bool operator()() const;

  /**
   * Destructor.
   */
  virtual ~LogStream();

 private:
  IGNITE_NO_COPY_ASSIGNMENT(LogStream);

  /** String buffer. */
  std::basic_stringbuf< char > strbuf;

  /** Parent logger object */
  Logger* logger;
};

/**
 * Logging facility.
 */
class Logger {
 public:
  /**
   * Destructor.
   */
  ~Logger() = default;

  /**
   * Set the logger's set log level.
   */
  void SetLogLevel(LogLevel::Type level);

  /**
   * Set the logger's set log path.
   */
  void SetLogPath(const std::string& path);

  /**
   * Sets the stream to use for logging.
   */
  void SetLogStream(std::ostream* stream);

  /**
   * Gets the current stream to use for logging.
   * Be careful to use this ostream. It is not
   * multi-thread safe. Any modification to it shoud
   * be protected by lock.
   */
  std::ostream* GetLogStream() {
    return stream;
  }

  /**
   * Get default log path.
   * @return Logger default path.
   */
  static std::string GetDefaultLogPath();

  /**
   * Get singleton instance of Logger.
   * If there is no instance, create new instance.
   * @return Logger instance.
   */
  static std::shared_ptr< Logger > GetLoggerInstance() {
    if (!logger_) {
      // avoid to be created multiple times for multi-thread execution
      ignite::odbc::common::concurrent::CsLockGuard guard(mutexForCreation);
      if (!logger_) {
        logger_ = std::shared_ptr< Logger >(new Logger());
      }
    }

    return logger_;
  }

  /**
   * Get a file base name without path.
   * @return File base name.
   */
  static std::string GetBaseFileName(const std::string& s) {
#ifdef _WIN32
    char sep = '\\';
#else
    char sep = '/';
#endif

    size_t i = s.rfind(sep, s.length());
    if (i != std::string::npos) {
      return s.substr(i + 1, s.length() - i);
    }

    return "";
  }

  /**
   * Get the logger's set log level.
   * @return logLevel.
   */
  LogLevel::Type GetLogLevel() const;

  /**
   * Get the logger's set log path.
   * @return logPath.
   */
  std::string& GetLogPath();

  /**
   * Checks if file stream is opened.
   * @return True, if file stream is opened.
   */
  bool IsFileStreamOpen() const;

  /**
   * Checks if logging is enabled.
   * @return True, if logging is enabled.
   */
  bool IsEnabled() const;

  /**
   * Enable logging if log path is set and log level is not off.
   * @return True, if logging is enabled. False is logging cannot be enabled
   */
  bool EnableLog();

  /**
   * Outputs the message to log file
   * @param message The message to write
   */
  void WriteMessage(std::string const& message);

 private:
  static std::shared_ptr< Logger > logger_;  // a singleton instance

  /**
   * Constructor.
   */
  Logger() = default;

  /**
   * Creates the log file name based on date
   * Log file format: timestream_odbc_YYYYMMDD.log
   */
  std::string CreateFileName() const;

  IGNITE_NO_COPY_ASSIGNMENT(Logger);

  /** Mutex for writes synchronization. */
  CriticalSection mutex;

  /** Mutex for singleton creation. */
  static CriticalSection mutexForCreation;

  /**
   * File stream, it is not multi-thread safe.
   * Any modification to it should be protected by lock.
   */
  std::ofstream fileStream;

  /** Reference to logging stream */
  std::ostream* stream = nullptr;

  /** Log folder path */
  std::string logPath = DEFAULT_LOG_PATH;

  /** Log Level */
  LogLevel::Type logLevel = LogLevel::Type::WARNING_LEVEL;

  /** Log file name */
  std::string logFileName;

  /** Log file path */
  std::string logFilePath;
};

}  // namespace odbc
}  // namespace timestream

#endif  //_TIMESTREAM_ODBC_LOG
