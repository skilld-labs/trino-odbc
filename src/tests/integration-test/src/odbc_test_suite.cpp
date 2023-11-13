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

#ifdef _WIN32
#include <windows.h>
#endif

#include <ignite/common/include/common/fixed_size_array.h>
#include "trino/odbc/log.h"
#include <sql.h>
#include <sqlext.h>
#include <fstream>

#include <boost/test/unit_test.hpp>

#include <trino/odbc/utility.h>
#include <trino/odbc/dsn_config.h>
#include <trino/odbc/config/configuration.h>
#include "odbc_test_suite.h"
#include "test_utils.h"

using namespace trino_test;
using namespace boost::unit_test;
using namespace trino::odbc::config;

/**
 * Test setup config for test results
 */
struct OdbcConfig {
  OdbcConfig() : test_log("odbc_test_result.xml") {
    unit_test_log.set_stream(test_log);
    unit_test_log.set_format(OF_JUNIT);
  }
  ~OdbcConfig() {
    unit_test_log.set_stream(std::cout);
  }

  std::ofstream test_log;
};

BOOST_GLOBAL_FIXTURE(OdbcConfig);

namespace trino {
namespace odbc {
void OdbcTestSuite::Prepare(int32_t odbcVer) {
  // Allocate an environment handle
  SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

  BOOST_REQUIRE(env != NULL);

  // We want ODBC 3 support
  SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, reinterpret_cast< void* >(odbcVer),
                0);

  // Allocate a connection handle
  SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

  BOOST_REQUIRE(dbc != NULL);
}

// Designed for multi-thread test. It does not include boost macros
// as boost unit test framework is not multi-thread safe.
void OdbcTestSuite::Connect(const std::string& connectStr, int32_t odbcVer,
                            bool& result) {
  // Allocate an environment handle
  SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

  if (!env) {
    std::cerr << "Failed to allocate an environment handle" << std::endl;
    result = false;
    return;
  }

  // We want ODBC 3 support by default
  SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, reinterpret_cast< void* >(odbcVer),
                0);

  // Allocate a connection handle
  SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

  if (!dbc) {
    std::cerr << "Failed to allocate a connection handle" << std::endl;
    result = false;
    return;
  }

  // Connect string
  std::vector< SQLWCHAR > connectStr0(connectStr.begin(), connectStr.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret =
      SQLDriverConnect(dbc, NULL, &connectStr0[0],
                       static_cast< SQLSMALLINT >(connectStr0.size()), outstr,
                       ODBC_BUFFER_SIZE, &outstrlen, SQL_DRIVER_COMPLETE);

  if (!SQL_SUCCEEDED(ret)) {
    std::cerr << GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc) << std::endl;
    result = false;
    return;
  }

  // Allocate a statement handle
  SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

  if (!stmt) {
    std::cerr << "Failed to allocate a statement handle" << std::endl;
    result = false;
    return;
  }
  result = true;
}

void OdbcTestSuite::Connect(SQLHDBC& conn, SQLHSTMT& statement,
                            const std::string& connectStr) {
  // Allocate a connection handle
  SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);

  BOOST_REQUIRE(conn != NULL);

  // Connect string
  std::vector< SQLWCHAR > connectStr0(connectStr.begin(), connectStr.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret =
      SQLDriverConnect(conn, NULL, &connectStr0[0],
                       static_cast< SQLSMALLINT >(connectStr0.size()), outstr,
                       ODBC_BUFFER_SIZE, &outstrlen, SQL_DRIVER_COMPLETE);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, conn));
  }

  // Allocate a statement handle
  SQLAllocHandle(SQL_HANDLE_STMT, conn, &statement);

  BOOST_REQUIRE(statement != NULL);
}

void OdbcTestSuite::Connect(const std::string& connectStr, int32_t odbcVer) {
  Prepare(odbcVer);

  // Connect string
  std::vector< SQLWCHAR > connectStr0(connectStr.begin(), connectStr.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret =
      SQLDriverConnect(dbc, NULL, &connectStr0[0],
                       static_cast< SQLSMALLINT >(connectStr0.size()), outstr,
                       ODBC_BUFFER_SIZE, &outstrlen, SQL_DRIVER_COMPLETE);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));

  // Allocate a statement handle
  SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

  BOOST_REQUIRE(stmt != NULL);
}

void OdbcTestSuite::Connect(const std::string& dsn, const std::string& username,
                            const std::string& password) {
  Prepare();

  // Connect string
  std::vector< SQLWCHAR > wDsn(dsn.begin(), dsn.end());
  std::vector< SQLWCHAR > wUsername(username.begin(), username.end());
  std::vector< SQLWCHAR > wPassword(password.begin(), password.end());

  // Connecting to ODBC server.
  SQLRETURN ret = SQLConnect(
      dbc, wDsn.data(), static_cast< SQLSMALLINT >(wDsn.size()),
      wUsername.data(), static_cast< SQLSMALLINT >(wUsername.size()),
      wPassword.data(), static_cast< SQLSMALLINT >(wPassword.size()));
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));

  // Allocate a statement handle
  SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

  BOOST_REQUIRE(stmt != NULL);
}

void OdbcTestSuite::ParseConnectionString(const std::string& connectionString,
                                          Configuration& config) {
  ConnectionStringParser parser(config);
  parser.ParseConnectionString(connectionString, nullptr);
}

void OdbcTestSuite::WriteDsnConfiguration(const Configuration& config) {
  IgniteError error;
  if (!odbc::WriteDsnConfiguration(config, error)) {
    std::stringstream msg;
    msg << "Call to WriteDsnConfiguration failed: " << error.GetText()
        << ", IgniteError code: " << error.GetCode();
    BOOST_FAIL(msg.str());
  }
}

void OdbcTestSuite::WriteDsnConfiguration(const std::string& dsn,
                                          const std::string& connectionString,
                                          std::string& username,
                                          std::string& password) {
  Configuration config;
  ParseConnectionString(connectionString, config);

  username = config.GetDSNUserName();
  password = config.GetDSNPassword();

  // Update the DSN
  config.SetDsn(dsn);

  WriteDsnConfiguration(config);
}

void OdbcTestSuite::DeleteDsnConfiguration(const std::string& dsn) {
  IgniteError error;
  if (!odbc::DeleteDsnConfiguration(dsn, error)) {
    std::stringstream msg;
    msg << "Call to DeleteDsnConfiguration failed: " << error.GetText()
        << ", IgniteError code: " << error.GetCode();
    BOOST_FAIL(msg.str());
  }
}

std::string OdbcTestSuite::ExpectSQLTablesReject(
    SQLWCHAR* catalogName, SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, SQLWCHAR* tableName, SQLSMALLINT tableNameLen,
    SQLWCHAR* tableType, SQLSMALLINT tableTypeLen,
    const std::string& expectedState, const std::string& expectedError) {
  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret =
      SQLTables(stmt, catalogName, catalogNameLen, schemaName, schemaNameLen,
                tableName, tableNameLen, tableType, tableTypeLen);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  OdbcClientError error = GetOdbcError(SQL_HANDLE_STMT, stmt);
  BOOST_CHECK_EQUAL(error.sqlstate, expectedState);
  size_t start = 0;
  if (error.message.substr(0, 10) == "[unixODBC]") {
    start += 10;
  }

  BOOST_REQUIRE_MESSAGE(
      error.message.substr(start, expectedError.size()) == expectedError,
      error.message.substr(start, expectedError.size()) + "!=" + expectedError);

  return GetOdbcErrorState(SQL_HANDLE_STMT, stmt);
}

std::string OdbcTestSuite::ExpectConnectionReject(
    const std::string& connectStr, const std::string& expectedState,
    const std::string& expectedError) {
  Prepare();

  // Connect string
  std::vector< SQLWCHAR > connectStr0(connectStr.begin(), connectStr.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret =
      SQLDriverConnect(dbc, NULL, &connectStr0[0],
                       static_cast< SQLSMALLINT >(connectStr0.size()), outstr,
                       ODBC_BUFFER_SIZE, &outstrlen, SQL_DRIVER_COMPLETE);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  OdbcClientError error = GetOdbcError(SQL_HANDLE_DBC, dbc);
  BOOST_CHECK_EQUAL(error.sqlstate, expectedState);
  size_t start = 0;
  if (error.message.substr(0, 10) == "[unixODBC]") {
    start += 10;
  }

  BOOST_REQUIRE_MESSAGE(
      error.message.substr(start, expectedError.size()) == expectedError,
      error.message.substr(start, expectedError.size()) + "!=" + expectedError);

  return GetOdbcErrorState(SQL_HANDLE_DBC, dbc);
}

std::string OdbcTestSuite::ExpectConnectionReject(
    const std::string& dsn, const std::string& username,
    const std::string& password, const std::string& expectedState,
    const std::string& expectedError) {
  Prepare();

  std::vector< SQLWCHAR > wDsn(dsn.begin(), dsn.end());
  std::vector< SQLWCHAR > wUsername(username.begin(), username.end());
  std::vector< SQLWCHAR > wPassword(password.begin(), password.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret = SQLConnect(
      dbc, wDsn.data(), static_cast< SQLSMALLINT >(wDsn.size()),
      wUsername.data(), static_cast< SQLSMALLINT >(wUsername.size()),
      wPassword.data(), static_cast< SQLSMALLINT >(wPassword.size()));

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  OdbcClientError error = GetOdbcError(SQL_HANDLE_DBC, dbc);
  BOOST_CHECK_EQUAL(error.sqlstate, expectedState);
  size_t prefixLen = std::string("[unixODBC]").size();
  BOOST_REQUIRE(error.message.substr(0, expectedError.size()) == expectedError
                || error.message.substr(prefixLen, expectedError.size())
                       == expectedError);

  return GetOdbcErrorState(SQL_HANDLE_DBC, dbc);
}

/**
 * Connect to Trino
 */
void OdbcTestSuite::ConnectToTS(int32_t odbcVer) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);

  Connect(dsnConnectionString, odbcVer);
}

void OdbcTestSuite::Disconnect() {
  if (stmt) {
    // Releasing statement handle.
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    stmt = nullptr;
  }

  if (dbc) {
    // Disconneting from the server.
    SQLDisconnect(dbc);

    // Releasing allocated handles.
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    dbc = nullptr;
  }
}

void OdbcTestSuite::CleanUp() {
  Disconnect();

  if (env) {
    // Releasing allocated handles.
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    env = NULL;
  }
}

OdbcTestSuite::OdbcTestSuite() : env(NULL), dbc(NULL), stmt(NULL) {
  // No-op.
}

OdbcTestSuite::~OdbcTestSuite() {
  CleanUp();
}

int8_t OdbcTestSuite::GetTestI8Field(int64_t idx) {
  return static_cast< int8_t >(idx * 8);
}

void OdbcTestSuite::CheckTestI8Value(int idx, int8_t value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestI8Field(idx));
}

int16_t OdbcTestSuite::GetTestI16Field(int64_t idx) {
  return static_cast< int16_t >(idx * 16);
}

void OdbcTestSuite::CheckTestI16Value(int idx, int16_t value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestI16Field(idx));
}

int32_t OdbcTestSuite::GetTestI32Field(int64_t idx) {
  return static_cast< int32_t >(idx * 32);
}

void OdbcTestSuite::CheckTestI32Value(int idx, int32_t value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestI32Field(idx));
}

std::string OdbcTestSuite::GetTestString(int64_t idx) {
  std::stringstream builder;

  builder << "String#" << idx;

  return builder.str();
}

void OdbcTestSuite::CheckTestStringValue(int idx, const std::string& value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestString(idx));
}

float OdbcTestSuite::GetTestFloatField(int64_t idx) {
  return static_cast< float >(idx) * 0.5f;
}

void OdbcTestSuite::CheckTestFloatValue(int idx, float value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestFloatField(idx));
}

double OdbcTestSuite::GetTestDoubleField(int64_t idx) {
  return static_cast< double >(idx) * 0.25f;
}

void OdbcTestSuite::CheckTestDoubleValue(int idx, double value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestDoubleField(idx));
}

bool OdbcTestSuite::GetTestBoolField(int64_t idx) {
  return ((idx % 2) == 0);
}

void OdbcTestSuite::CheckTestBoolValue(int idx, bool value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestBoolField(idx));
}

void OdbcTestSuite::GetTestDateField(int64_t idx, SQL_DATE_STRUCT& val) {
  val.year = static_cast< SQLSMALLINT >(2017 + idx / 365);
  val.month = static_cast< SQLUSMALLINT >(((idx / 28) % 12) + 1);
  val.day = static_cast< SQLUSMALLINT >((idx % 28) + 1);
}

void OdbcTestSuite::CheckTestDateValue(int idx, const SQL_DATE_STRUCT& val) {
  BOOST_TEST_CONTEXT("Test index: " << idx) {
    SQL_DATE_STRUCT expected;
    GetTestDateField(idx, expected);

    BOOST_CHECK_EQUAL(val.year, expected.year);
    BOOST_CHECK_EQUAL(val.month, expected.month);
    BOOST_CHECK_EQUAL(val.day, expected.day);
  }
}

void OdbcTestSuite::GetTestTimeField(int64_t idx, SQL_TIME_STRUCT& val) {
  val.hour = (idx / 3600) % 24;
  val.minute = (idx / 60) % 60;
  val.second = idx % 60;
}

void OdbcTestSuite::CheckTestTimeValue(int idx, const SQL_TIME_STRUCT& val) {
  BOOST_TEST_CONTEXT("Test index: " << idx) {
    SQL_TIME_STRUCT expected;
    GetTestTimeField(idx, expected);

    BOOST_CHECK_EQUAL(val.hour, expected.hour);
    BOOST_CHECK_EQUAL(val.minute, expected.minute);
    BOOST_CHECK_EQUAL(val.second, expected.second);
  }
}

void OdbcTestSuite::GetTestTimestampField(int64_t idx,
                                          SQL_TIMESTAMP_STRUCT& val) {
  SQL_DATE_STRUCT date;
  GetTestDateField(idx, date);

  SQL_TIME_STRUCT time;
  GetTestTimeField(idx, time);

  val.year = date.year;
  val.month = date.month;
  val.day = date.day;
  val.hour = time.hour;
  val.minute = time.minute;
  val.second = time.second;
  val.fraction = static_cast< uint64_t >(std::abs(idx * 914873)) % 1000000000;
}

void OdbcTestSuite::CheckTestTimestampValue(int idx,
                                            const SQL_TIMESTAMP_STRUCT& val) {
  BOOST_TEST_CONTEXT("Test index: " << idx) {
    SQL_TIMESTAMP_STRUCT expected;
    GetTestTimestampField(idx, expected);

    BOOST_CHECK_EQUAL(val.year, expected.year);
    BOOST_CHECK_EQUAL(val.month, expected.month);
    BOOST_CHECK_EQUAL(val.day, expected.day);
    BOOST_CHECK_EQUAL(val.hour, expected.hour);
    BOOST_CHECK_EQUAL(val.minute, expected.minute);
    BOOST_CHECK_EQUAL(val.second, expected.second);
    BOOST_CHECK_EQUAL(val.fraction, expected.fraction);
  }
}

void OdbcTestSuite::CheckSQLDiagnosticError(
    int16_t handleType, SQLHANDLE handle,
    const std::string& expectedSqlStateStr) {
  SQLWCHAR state[ODBC_BUFFER_SIZE];
  SQLINTEGER nativeError = 0;
  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT messageLen = 0;

  SQLRETURN ret = SQLGetDiagRec(handleType, handle, 1, state, &nativeError,
                                message, ODBC_BUFFER_SIZE, &messageLen);

  const std::string sqlState =
      trino::odbc::utility::SqlWcharToString(state);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);
  BOOST_REQUIRE_EQUAL(sqlState, expectedSqlStateStr);
  BOOST_REQUIRE(messageLen > 0);
}

void OdbcTestSuite::CheckSQLStatementDiagnosticError(
    const std::string& expectSqlState) {
  CheckSQLDiagnosticError(SQL_HANDLE_STMT, stmt, expectSqlState);
}

void OdbcTestSuite::CheckSQLConnectionDiagnosticError(
    const std::string& expectSqlState) {
  CheckSQLDiagnosticError(SQL_HANDLE_DBC, dbc, expectSqlState);
}

std::vector< SQLWCHAR > OdbcTestSuite::MakeSqlBuffer(const std::string& value) {
  return utility::ToWCHARVector(value);
}

SQLRETURN OdbcTestSuite::ExecQuery(const std::string& qry) {
  std::vector< SQLWCHAR > sql = MakeSqlBuffer(qry);

  return SQLExecDirect(stmt, sql.data(), static_cast< SQLINTEGER >(sql.size()));
}

SQLRETURN OdbcTestSuite::PrepareQuery(const std::string& qry) {
  std::vector< SQLWCHAR > sql = MakeSqlBuffer(qry);

  return SQLPrepare(stmt, sql.data(), static_cast< SQLINTEGER >(sql.size()));
}

void OdbcTestSuite::GetIAMCredentials(std::string& accessKeyId,
                                      std::string& secretKey) const {
  accessKeyId = GetEnv("AWS_ACCESS_KEY_ID");
  secretKey = GetEnv("AWS_SECRET_ACCESS_KEY");

  if (accessKeyId.empty())
    std::cout << "accessKeyId is empty, please set AWS_ACCESS_KEY_ID "
                 "environment variable for tests to operate successfully"
              << std::endl;
  if (secretKey.empty())
    std::cout << "secretKey is empty, please set AWS_SECRET_ACCESS_KEY "
                 "environment variable for tests to operate successfully"
              << std::endl;
}

void OdbcTestSuite::CreateGenericDsnConnectionString(
    std::string& connectionString, AuthType::Type testAuthType,
    const std::string& uid, const std::string& pwd, const bool includeTSCred,
    const std::string& TSUsername, const std::string& TSPassword,
    const std::string& miscOptions) const {
  std::string sessionToken = GetEnv("AWS_SESSION_TOKEN", "");
  std::string logPath = GetEnv("TRINO_LOG_PATH", "");
  std::string logLevel = GetEnv("TRINO_LOG_LEVEL", "2");
  std::string region = GetEnv("AWS_REGION", "us-west-2");

  connectionString =
            "driver={Amazon Trino ODBC Driver};"
            "dsn={" + Configuration::DefaultValue::dsn + "};"
            "auth=" + AuthType::ToString(testAuthType) + ";"
            "uid=" + uid + ";"
            "pwd=" + pwd + ";"
            "region=" + region + ";"
            "logOutput=" + logPath + ";"
            "logLevel=" + logLevel + ";";

  if (testAuthType == AuthType::Type::IAM) {
    connectionString.append("sessionToken=" + sessionToken + ";");
  }

  if (includeTSCred) {
    // Append the Trino credentials
    std::string tsAuthentication = "";

    switch (testAuthType) {
      case AuthType::Type::IAM:
        tsAuthentication = 
            "accessKeyId=" + TSUsername + ";"
            "secretKey=" + TSPassword + ";";
      default:
        break;
    }

    if (!tsAuthentication.empty())
      connectionString.append(tsAuthentication);
  }

  if (!miscOptions.empty())
    connectionString.append(miscOptions);
}

void OdbcTestSuite::CreateAADDsnConnectionString(
    std::string& connectionString, const char* uid, const char* pwd,
    const char* appId, const char* tenantId, const char* clientSecret,
    const char* roleArn, const char* idpArn) const {
  std::string logPath = GetEnv("TRINO_LOG_PATH", "");
  std::string logLevel = GetEnv("TRINO_LOG_LEVEL", "2");
  std::string region = GetEnv("AWS_REGION", "us-west-2");

  connectionString =
      "driver={Amazon Trino ODBC Driver};"
      "dsn={" + Configuration::DefaultValue::dsn + "};"
      "auth=" + AuthType::ToString(AuthType::Type::AAD) + ";"
      "region=" + region + ";"
      "logOutput=" + logPath + ";"
      "logLevel=" + logLevel + ";";

  std::string tsAuthentication = 
      "idPUsername=" + (!uid ? GetEnv("AAD_USER") : std::string(uid))  + ";"
      "idPPassword=" + (!pwd ? GetEnv("AAD_USER_PWD") : std::string(pwd)) + ";"
      "aadApplicationID=" + (!appId ? GetEnv("AAD_APP_ID") : std::string(appId)) + ";"
      "aadClientSecret=" + (!clientSecret ? GetEnv("AAD_CLIENT_SECRET") : std::string(clientSecret)) + ";"
      "aadTenant=" + (!tenantId ? GetEnv("AAD_TENANT") : std::string(tenantId)) + ";"
      "roleARN=" + (!roleArn ? GetEnv("AAD_ROLE_ARN") : std::string(roleArn)) + ";"
      "idPARN=" + (!idpArn ? GetEnv("AAD_IDP_ARN") : std::string(idpArn))  + ";";

  connectionString.append(tsAuthentication);
}

void OdbcTestSuite::CreateOktaDsnConnectionString(
    std::string& connectionString, const char* host, const char* uid,
    const char* pwd, const char* appId, const char* roleArn,
    const char* idpArn) const {
  std::string logPath = GetEnv("TRINO_LOG_PATH", "");
  std::string logLevel = GetEnv("TRINO_LOG_LEVEL", "2");
  std::string region = GetEnv("AWS_REGION", "us-west-2");

  connectionString =
      "driver={Amazon Trino ODBC Driver};"
      "dsn={" + Configuration::DefaultValue::dsn + "};"
      "auth=" + AuthType::ToString(AuthType::Type::OKTA) + ";"
      "region=" + region + ";"
      "logOutput=" + logPath + ";"
      "logLevel=" + logLevel + ";";

  std::string tsAuthentication = 
      "idPHost=" + (!host ? GetEnv("OKTA_HOST") : std::string(host)) + ";" +
      "idPUsername=" + (!uid ? GetEnv("OKTA_USER") : std::string(uid)) + ";"
      "idPPassword=" + (!pwd ? GetEnv("OKTA_USER_PWD") : std::string(pwd)) + ";"
      "OktaApplicationID=" + (!appId ? GetEnv("OKTA_APP_ID") : std::string(appId)) + ";"
      "roleARN=" + (!roleArn ? GetEnv("OKTA_ROLE_ARN") : std::string(roleArn)) + ";"
      "idPARN=" + (!idpArn ? GetEnv("OKTA_IDP_ARN") : std::string(idpArn)) + ";";

  connectionString.append(tsAuthentication);
}

void OdbcTestSuite::CreateDsnConnectionStringForAWS(
    std::string& connectionString, const std::string& keyId,
    const std::string& secret, const std::string& miscOptions) const {
  std::string accessKeyId = GetEnv("AWS_ACCESS_KEY_ID");
  std::string secretKey = GetEnv("AWS_SECRET_ACCESS_KEY");
  std::string sessionToken = GetEnv("AWS_SESSION_TOKEN", "");
  std::string region = GetEnv("AWS_REGION", "us-west-2");
  std::string logPath = GetEnv("TRINO_LOG_PATH", "");
  std::string logLevel = GetEnv("TRINO_LOG_LEVEL", "2");

  if (!keyId.empty()) {
    accessKeyId = keyId;
  }
  if (!secret.empty()) {
    secretKey = secret;
  }

  // "DRIVER" is capitalized to allow TestSQLDriverConnect in
  // api_robustness_test to pass.

  connectionString =
            "DRIVER={Amazon Trino ODBC Driver};"
            "dsn={" + Configuration::DefaultValue::dsn + "};"
            "auth=" + AuthType::ToString(AuthType::Type::IAM) + ";"
            "accessKeyId=" + accessKeyId + ";"
            "secretKey=" + secretKey + ";"
            "sessionToken=" + sessionToken + ";"
            "region=" + region + ";"
            "logOutput=" + logPath + ";"
            "logLevel=" + logLevel + ";";

  if (!miscOptions.empty())
    connectionString.append(miscOptions);
}

void OdbcTestSuite::AddMaxRowPerPage(std::string& connectionString,
                                     const std::string& value) {
  connectionString.append("maxRowPerPage=" + value + ";");
}

void OdbcTestSuite::CreateDsnConnectionStringForAWS(
    std::string& connectionString, AuthType::Type testAuthType,
    const std::string& profileName, const std::string& miscOptions) const {
  std::string region = GetEnv("AWS_REGION", "us-west-2");
  std::string logPath = GetEnv("TRINO_LOG_PATH", "");
  std::string logLevel = GetEnv("TRINO_LOG_LEVEL", "2");

  connectionString =
            "driver={Amazon Trino ODBC Driver};"
            "dsn={" + Configuration::DefaultValue::dsn + "};"
            "auth=" + AuthType::ToString(testAuthType) + ";"
            "profileName=" + profileName + ";"
            "region=" + region + ";"
            "logOutput=" + logPath + ";"
            "logLevel=" + logLevel + ";";

  if (!miscOptions.empty())
    connectionString.append(miscOptions);
}
}  // namespace odbc
}  // namespace trino
