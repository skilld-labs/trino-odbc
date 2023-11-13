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

#include <string>

#include <odbc_unit_test_suite.h>
#include "trino/odbc/log.h"
#include "trino/odbc/log_level.h"
#include <ignite/common/include/common/platform_utils.h>
#include <trino/odbc/authentication/auth_type.h>
#include "trino/odbc/statement.h"
#include "trino/odbc/utility.h"

using trino::odbc::AuthType;
using trino::odbc::MockConnection;
using trino::odbc::MockTrinoService;
using trino::odbc::OdbcUnitTestSuite;
using trino::odbc::Statement;
using trino::odbc::config::Configuration;
using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct DataQueryUnitTestSuiteFixture : OdbcUnitTestSuite {
  DataQueryUnitTestSuiteFixture() : OdbcUnitTestSuite() {
    stmt = dbc->CreateStatement();
  }

  /**
   * Destructor.
   */
  ~DataQueryUnitTestSuiteFixture() = default;

  void getLogOptions(Configuration& config) const {
    using ignite::odbc::common::GetEnv;
    using trino::odbc::LogLevel;

    std::string logPath = GetEnv("TRINO_LOG_PATH", "");
    std::string logLevelStr = GetEnv("TRINO_LOG_LEVEL", "2");

    LogLevel::Type logLevel = LogLevel::FromString(logLevelStr);
    config.SetLogLevel(logLevel);
    config.SetLogPath(logPath);
  }

  bool IsSuccessful() {
    if (!stmt) {
      return false;
    }
    return stmt->GetDiagnosticRecords().IsSuccessful();
  }

  int GetReturnCode() {
    if (!stmt) {
      return SQL_ERROR;
    }
    return stmt->GetDiagnosticRecords().GetReturnCode();
  }

  std::string GetSqlState() {
    if (!stmt) {
      return "";
    }
    return stmt->GetDiagnosticRecords()
        .GetStatusRecord(stmt->GetDiagnosticRecords().GetLastNonRetrieved())
        .GetSqlState();
  }

  void Connect() {
    Configuration cfg;
    cfg.SetAuthType(AuthType::Type::IAM);
    cfg.SetAccessKeyId("AwsTSUnitTestKeyId");
    cfg.SetSecretKey("AwsTSUnitTestSecretKey");
    getLogOptions(cfg);

    dbc->Establish(cfg);
  }
};

BOOST_FIXTURE_TEST_SUITE(DataQueryUnitTestSuite, DataQueryUnitTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestDataQuery) {
  // Test a normal query which resultset has 3 rows and returns in one page
  Connect();

  std::string sql = "select measure, time from mockDB.mockTable";
  stmt->ExecuteSqlQuery(sql);

  BOOST_CHECK(IsSuccessful());

  const int32_t buf_size = 1024;
  SQLWCHAR measure[buf_size]{};
  SQLLEN measure_len = 0;

  stmt->BindColumn(1, SQL_C_WCHAR, measure, sizeof(measure), &measure_len);

  SQL_TIMESTAMP_STRUCT timestamp;
  SQLLEN timestamp_len = 0;
  stmt->BindColumn(2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);

  stmt->FetchRow();
  BOOST_CHECK(IsSuccessful());

  BOOST_CHECK_EQUAL("cpu_usage", trino::odbc::utility::SqlWcharToString(
                                     measure, measure_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 11);
  BOOST_CHECK_EQUAL(timestamp.day, 9);
  BOOST_CHECK_EQUAL(timestamp.hour, 23);
  BOOST_CHECK_EQUAL(timestamp.minute, 52);
  BOOST_CHECK_EQUAL(timestamp.second, 51);
  BOOST_CHECK_EQUAL(timestamp.fraction, 554000000);

  stmt->FetchRow();
  BOOST_CHECK(IsSuccessful());

  stmt->FetchRow();
  BOOST_CHECK(IsSuccessful());

  stmt->FetchRow();
  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestDataQuery10000Rows) {
  // Test fetching 10000 rows and each page contains 3 rows
  Connect();

  std::string sql = "select measure, time from mockDB.mockTable10000";
  stmt->ExecuteSqlQuery(sql);

  BOOST_CHECK(IsSuccessful());

  for (int i = 0; i < 10000; i++) {
    stmt->FetchRow();
    BOOST_CHECK(IsSuccessful());
  }
}

BOOST_AUTO_TEST_CASE(TestDataQuery10RowWithError) {
  // Test fetching 10 rows and each page contains 3 rows.
  // When fetch the 10th row, the outcome contains an error.
  Connect();

  std::string sql = "select measure, time from mockDB.mockTable10Error";
  stmt->ExecuteSqlQuery(sql);

  BOOST_CHECK(IsSuccessful());

  for (int i = 0; i < 9; i++) {
    stmt->FetchRow();
    BOOST_CHECK(IsSuccessful());
  }

  // The 10th row fetching fails due to response outcome containing an error
  stmt->FetchRow();
  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);

  // no data for the following fetching
  stmt->FetchRow();
  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_NO_DATA);
}

BOOST_AUTO_TEST_SUITE_END()
