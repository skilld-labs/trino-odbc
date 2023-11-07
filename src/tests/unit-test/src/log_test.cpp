/*
 * Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */
#include <odbc_unit_test_suite.h>

#include <timestream/odbc/connection.h>
#include <timestream/odbc/log.h>
#include <timestream/odbc/log_level.h>

/*@*/
#include <aws/core/utils/logging/LogLevel.h>

#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>
#include <random>
#include <string>

using namespace boost::unit_test;

using boost::unit_test::precondition;
using timestream::odbc::Logger;
using timestream::odbc::LogLevel;
using timestream::odbc::OdbcUnitTestSuite;

/**
 * Test setup fixture.
 */
struct LogUnitTestSuiteFixture : OdbcUnitTestSuite {
  LogUnitTestSuiteFixture() : OdbcUnitTestSuite() {
  }

  /**
   * Destructor.
   */
  ~LogUnitTestSuiteFixture() {
  }

  bool SaveLoggerVars(std::shared_ptr< Logger > logger,
                      boost::optional< std::string >& origLogPath,
                      boost::optional< LogLevel::Type >& origLogLevel) {
    if (logger->IsEnabled()) {
      origLogPath = logger->GetLogPath();
      origLogLevel = logger->GetLogLevel();

      return true;
    }
    origLogPath = boost::none;
    origLogLevel = boost::none;

    return false;
  }

  void setLoggerVars(std::shared_ptr< Logger > logger,
                     boost::optional< std::string >& origLogPath,
                     boost::optional< LogLevel::Type >& origLogLevel) {
    // pre-requiste: origLogPath and origLogLevel hold valid values

    logger->SetLogLevel(origLogLevel.get());
    logger->SetLogPath(origLogPath.get());
  }
};

BOOST_FIXTURE_TEST_SUITE(LogUnitTestSuite, LogUnitTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestLogStreamCreatedOnDefaultInstance) {
  std::minstd_rand randNum;
  randNum.seed(29);

  std::string logPath = DEFAULT_LOG_PATH;
  LogLevel::Type logLevel = LogLevel::Type::DEBUG_LEVEL;

  std::shared_ptr< Logger > logger = Logger::GetLoggerInstance();

  boost::optional< std::string > origLogPath;
  boost::optional< LogLevel::Type > origLogLevel;
  bool logVarSaved = SaveLoggerVars(logger, origLogPath, origLogLevel);

  // set log level and stream
  logger->SetLogLevel(logLevel);
  logger->SetLogPath(logPath);

  // check log level
  LogLevel::Type loggerLogLevel = logger->GetLogLevel();
  BOOST_CHECK(logLevel == loggerLogLevel);

  // check log path
  std::string loggerLogPath = logger->GetLogPath();
  BOOST_CHECK_EQUAL(logPath, loggerLogPath);

  std::stringstream stringStream;
  std::string testData;
  testData = "defTest" + std::to_string(randNum());

  // Write to log file.
  LOG_DEBUG_MSG(
      "TestLogStreamCreatedOnDefaultInstance begins. Log path/level changes "
      "are expected.");

  LOG_DEBUG_MSG(testData);

  // Check that log file is working
  BOOST_CHECK(logger->IsFileStreamOpen());
  BOOST_CHECK(logger->IsEnabled());

  // this boost check means that testData is not in stringStream
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));
  // find means finding the last instance of the string (param)

  // Write to stream.
  LOG_DEBUG_MSG_TO_STREAM(testData, &stringStream);

  // Check that logger is still enabled after writing to stream
  BOOST_CHECK(logger->IsEnabled());

  // Check that log stream is working
  // this boost check means that testData is in stringStream
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write error log to log stream, which should work
  testData = "debugLvlTest1" + std::to_string(randNum());
  LOG_ERROR_MSG_TO_STREAM(testData, &stringStream);

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write warning log to log stream, which should work
  testData = "debugLvlTest2" + std::to_string(randNum());
  LOG_WARNING_MSG_TO_STREAM(testData, &stringStream);

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write info log to log stream, which should work
  testData = "debugLvlTest3" + std::to_string(randNum());
  LOG_INFO_MSG_TO_STREAM(testData, &stringStream);

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  LOG_DEBUG_MSG(
      "TestLogStreamCreatedOnDefaultInstance ends. Log path/level changes are "
      "expected.");

  // set the original log level / log path back
  if (logVarSaved)
    setLoggerVars(logger, origLogPath, origLogLevel);
}

BOOST_AUTO_TEST_CASE(TestLogStreamWithInfoLevel) {
  std::minstd_rand randNum;
  randNum.seed(31);

  std::string logPath = DEFAULT_LOG_PATH;
  LogLevel::Type logLevel = LogLevel::Type::INFO_LEVEL;

  std::shared_ptr< Logger > logger = Logger::GetLoggerInstance();

  // save the original log path / log level
  boost::optional< std::string > origLogPath;
  boost::optional< LogLevel::Type > origLogLevel;
  bool logVarSaved = SaveLoggerVars(logger, origLogPath, origLogLevel);

  // set log level and stream
  logger->SetLogLevel(logLevel);
  logger->SetLogPath(logPath);

  // check log level
  LogLevel::Type loggerLogLevel = logger->GetLogLevel();
  BOOST_CHECK(logLevel == loggerLogLevel);

  std::stringstream stringStream;
  std::string testData;
  testData = "infoLvlTest1" + std::to_string(randNum());

  // Write to log file.
  LOG_INFO_MSG(
      "TestLogStreamWithInfoLevel begins. Log path/level changes are "
      "expected.");

  LOG_INFO_MSG(testData);

  // Check that log file is working
  BOOST_CHECK(logger->IsFileStreamOpen());
  BOOST_CHECK(logger->IsEnabled());

  // check that stringStream does not have testData
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write debug log to log file, which should fail
  testData = "infoLvlTest2" + std::to_string(randNum());
  LOG_DEBUG_MSG(testData);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  testData = "infoLvlTest3" + std::to_string(randNum());
  // Write to stream.
  LOG_INFO_MSG_TO_STREAM(testData, &stringStream);

  // Check that logger is still enabled after writing to stream
  BOOST_CHECK(logger->IsEnabled());

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write error log to log stream, which should work
  testData = "infoLvlTest4" + std::to_string(randNum());
  LOG_ERROR_MSG_TO_STREAM(testData, &stringStream);

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write warning log to log stream, which should work
  testData = "infoLvlTest5" + std::to_string(randNum());
  LOG_WARNING_MSG_TO_STREAM(testData, &stringStream);

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write debug log to log stream, which should fail
  testData = "infoLvlTest6" + std::to_string(randNum());
  LOG_DEBUG_MSG_TO_STREAM(testData, &stringStream);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  LOG_INFO_MSG(
      "TestLogStreamWithInfoLevel ends. Log path/level changes are expected.");

  // set the original log level / log path back
  if (logVarSaved)
    setLoggerVars(logger, origLogPath, origLogLevel);
}

BOOST_AUTO_TEST_CASE(TestLogStreamWithWarningLevel) {
  std::minstd_rand randNum;
  randNum.seed(31);

  std::string logPath = DEFAULT_LOG_PATH;
  LogLevel::Type logLevel = LogLevel::Type::WARNING_LEVEL;

  std::shared_ptr< Logger > logger = Logger::GetLoggerInstance();

  // save the original log path / log level
  boost::optional< std::string > origLogPath;
  boost::optional< LogLevel::Type > origLogLevel;
  bool logVarSaved = SaveLoggerVars(logger, origLogPath, origLogLevel);

  // set log level and stream
  logger->SetLogLevel(logLevel);
  logger->SetLogPath(logPath);

  // check log level
  LogLevel::Type loggerLogLevel = logger->GetLogLevel();
  BOOST_CHECK(logLevel == loggerLogLevel);

  std::stringstream stringStream;
  std::string testData;
  testData = "warningLvlTest1" + std::to_string(randNum());

  // Write to log file.
  LOG_WARNING_MSG(
      "TestLogStreamWithWarningLevel begins. Log path/level changes are "
      "expected.");

  LOG_WARNING_MSG(testData);

  // Check that log file is working
  BOOST_CHECK(logger->IsFileStreamOpen());
  BOOST_CHECK(logger->IsEnabled());

  // check that stringStream does not have testData
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write debug log to log file, which should fail
  testData = "warningLvlTest2" + std::to_string(randNum());
  LOG_DEBUG_MSG(testData);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write info log to log file, which should fail
  testData = "warningLvlTest3" + std::to_string(randNum());
  LOG_INFO_MSG(testData);

  // Check that the info log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  testData = "warningLvlTest4" + std::to_string(randNum());
  // Write to stream.
  LOG_WARNING_MSG_TO_STREAM(testData, &stringStream);

  // Check that logger is still enabled after writing to stream
  BOOST_CHECK(logger->IsEnabled());

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write error log to log stream, which should work
  testData = "warningLvlTest5" + std::to_string(randNum());
  LOG_ERROR_MSG_TO_STREAM(testData, &stringStream);

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write debug log to log stream, which should fail
  testData = "warningLvlTest6" + std::to_string(randNum());
  LOG_DEBUG_MSG_TO_STREAM(testData, &stringStream);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write info log to log stream, which should fail
  testData = "warningLvlTest7" + std::to_string(randNum());
  LOG_INFO_MSG_TO_STREAM(testData, &stringStream);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  LOG_WARNING_MSG(
      "TestLogStreamWithWarningLevel ends. Log path/level changes are "
      "expected.");

  // set the original log level / log path back
  if (logVarSaved)
    setLoggerVars(logger, origLogPath, origLogLevel);
}

BOOST_AUTO_TEST_CASE(TestLogStreamWithErrorLevel) {
  std::minstd_rand randNum;
  randNum.seed(42);

  std::string logPath = DEFAULT_LOG_PATH;
  LogLevel::Type logLevel = LogLevel::Type::ERROR_LEVEL;

  std::shared_ptr< Logger > logger = Logger::GetLoggerInstance();

  // save the original log path / log level
  boost::optional< std::string > origLogPath;
  boost::optional< LogLevel::Type > origLogLevel;
  bool logVarSaved = SaveLoggerVars(logger, origLogPath, origLogLevel);

  // set log level and stream
  logger->SetLogLevel(logLevel);
  logger->SetLogPath(logPath);

  // check log level
  LogLevel::Type loggerLogLevel = logger->GetLogLevel();
  BOOST_CHECK(logLevel == loggerLogLevel);

  std::stringstream stringStream;
  std::string testData;
  testData = "errLvlTest1" + std::to_string(randNum());

  // Write to log file.
  LOG_ERROR_MSG(
      "(Not an actual error, logged for clarity) TestLogStreamWithErrorLevel "
      "begins. Log path/level changes are expected.");

  LOG_ERROR_MSG(testData);

  // Check that log file is working
  BOOST_CHECK(logger->IsFileStreamOpen());
  BOOST_CHECK(logger->IsEnabled());

  // check that stringStream does not have testData
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write debug log to log file, which should fail
  testData = "errLvlTest2" + std::to_string(randNum());
  LOG_DEBUG_MSG(testData);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write info log to log file, which should fail
  testData = "errLvlTest3" + std::to_string(randNum());
  LOG_INFO_MSG(testData);

  // Check that the info log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write warning log to log file, which should fail
  testData = "errLvlTest4" + std::to_string(randNum());
  LOG_WARNING_MSG(testData);

  // Check that the warning log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  testData = "errLvlTest5" + std::to_string(randNum());
  // Write to stream.
  LOG_ERROR_MSG_TO_STREAM(testData, &stringStream);

  // Check that logger is still enabled after writing to stream
  BOOST_CHECK(logger->IsEnabled());

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write debug log to log stream, which should fail
  testData = "errLvlTest6" + std::to_string(randNum());
  LOG_DEBUG_MSG_TO_STREAM(testData, &stringStream);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write info log to log stream, which should fail
  testData = "errLvlTest7" + std::to_string(randNum());
  LOG_INFO_MSG_TO_STREAM(testData, &stringStream);

  // Check that the info log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  LOG_ERROR_MSG(
      "(Not an actual error, logged for clarity) TestLogStreamWithErrorLevel "
      "ends. Log path/level changes are expected.");

  // set the original log level / log path back
  if (logVarSaved)
    setLoggerVars(logger, origLogPath, origLogLevel);
}

BOOST_AUTO_TEST_CASE(TestLogSetInvalidLogPath) {
  std::string logPath = "invalid\\log\\path";

  std::shared_ptr< Logger > logger = Logger::GetLoggerInstance();

  // save the original log path / log level
  boost::optional< std::string > origLogPath;
  boost::optional< LogLevel::Type > origLogLevel;
  bool logVarSaved = SaveLoggerVars(logger, origLogPath, origLogLevel);

  // attempt to set wrong log path
  logger->SetLogPath(logPath);

  // check that invalid log path is not set and the original log path remains
  // (if not null)
  BOOST_CHECK_NE(logPath, logger->GetLogPath());
  if (origLogPath)
    BOOST_CHECK_EQUAL(origLogPath.get(), logger->GetLogPath());

  // set the original log level / log path back
  // in case the invalid log path is set, still ensure to change it back at end
  // of the test
  if (logVarSaved)
    setLoggerVars(logger, origLogPath, origLogLevel);
}

BOOST_AUTO_TEST_CASE(TestAWSLogLevelParseMixedCases) {
  using timestream::odbc::Connection;

  // Check default value is Warn
  BOOST_CHECK(Connection::GetAWSLogLevelFromString("")
              == Aws::Utils::Logging::LogLevel::Warn);

  // Check parse strings
  BOOST_CHECK(Connection::GetAWSLogLevelFromString("OfF")
              == Aws::Utils::Logging::LogLevel::Off);

  BOOST_CHECK(Connection::GetAWSLogLevelFromString("FatAl")
              == Aws::Utils::Logging::LogLevel::Fatal);

  BOOST_CHECK(Connection::GetAWSLogLevelFromString("ErroR")
              == Aws::Utils::Logging::LogLevel::Error);

  BOOST_CHECK(Connection::GetAWSLogLevelFromString("WARn")
              == Aws::Utils::Logging::LogLevel::Warn);

  BOOST_CHECK(Connection::GetAWSLogLevelFromString("infO")
              == Aws::Utils::Logging::LogLevel::Info);

  BOOST_CHECK(Connection::GetAWSLogLevelFromString("dEbUg")
              == Aws::Utils::Logging::LogLevel::Debug);

  BOOST_CHECK(Connection::GetAWSLogLevelFromString("trace")
              == Aws::Utils::Logging::LogLevel::Trace);
}

BOOST_AUTO_TEST_SUITE_END()