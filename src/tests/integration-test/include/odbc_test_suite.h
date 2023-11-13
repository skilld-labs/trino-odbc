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

#ifndef ODBC_TEST_ODBC_TEST_SUITE
#define ODBC_TEST_ODBC_TEST_SUITE

#ifdef _WIN32
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>
#include <string>
#include <boost/test/unit_test.hpp>

#include <ignite/common/include/common/platform_utils.h>
#include <trino/odbc/config/configuration.h>
#include <trino/odbc/config/connection_string_parser.h>
#include <trino/odbc/authentication/auth_type.h>
#include <trino/odbc/system/odbc_constants.h>

#ifndef BOOST_TEST_CONTEXT
#define BOOST_TEST_CONTEXT(...)
#endif

#ifndef BOOST_TEST_INFO
#define BOOST_TEST_INFO(...)
#endif

using boost::unit_test::test_unit_id;
using ignite::odbc::common::GetEnv;
using namespace trino::odbc::config;

namespace trino {
namespace odbc {
/**
 * Test setup fixture.
 */
struct OdbcTestSuite {
  /**
   * Prepare environment.
   *
   * @param odbcVer ODBC Version to test, default is ODBC3
   */
  void Prepare(int32_t odbcVer = SQL_OV_ODBC3);

  /**
   * Establish connection to node using provided handles.
   * For multi-thread test use, no boost macro involved
   *
   * @param connectStr Connection string.
   * @param odbcVer ODBC Version
   * @param result the function execution result
   */
  void Connect(const std::string& connectStr, int32_t odbcVer, bool& result);

  /**
   * Establish connection to node using provided handles.
   *
   * @param conn Connection.
   * @param statement Statement to allocate.
   * @param connectStr Connection string.
   */
  void Connect(SQLHDBC& conn, SQLHSTMT& statement,
               const std::string& connectStr);

  /**
   * Establish connection to node using default handles.
   *
   * @param connectStr Connection string.
   * @param odbcVer ODBC Version, default is ODBC3
   */
  void Connect(const std::string& connectStr, int32_t odbcVer = SQL_OV_ODBC3);

  /**
   * Establish connection to node using default handles.
   *
   * @param dsn The DSN of the connection
   * @param username user name
   * @param password password
   */
  void Connect(const std::string& dsn, const std::string& username,
               const std::string& password);

  /**
   * Parse the connection string into configuration.
   *
   * @param connectionString the connection string to parse.
   * @param config the configuration to update.
   */
  void ParseConnectionString(const std::string& connectionString,
                             Configuration& config);

  /**
   * Writes the DSN configuration
   *
   * @param dsn The DSN to write
   * @param config The configuration to write
   */
  void WriteDsnConfiguration(const Configuration& config);

  /**
   * Writes the DSN configuration for a local connection.
   * Sets the username and password parameters to be used
   * with SQLConnect.
   *
   * @param dsn the DSN to write configuration for.
   * @param username the updated username for the connection.
   * @param password the updated password for the connection.
   */
  void WriteDsnConfiguration(const std::string& dsn,
                             const std::string& connectionString,
                             std::string& username, std::string& password);

  /**
   * Removes the DSN configuration
   *
   * @param dsn The DSN to write
   */
  void DeleteDsnConfiguration(const std::string& dsn);

  /**
   * Expect SQLTables to fail.
   *
   * @param catalogName Catalog name
   * @param catalogNameLen Catalog name len
   * @param schemaName Schema name
   * @param schemaNameLen Schema name len
   * @param tableName Table name
   * @param tableNameLen Table name len
   * @param tableType Table type
   * @param tableTypeLen Table type len
   * @param expectedState Expected error state
   * @param expectedError Expected error message
   * @return SQL State.
   */
  std::string ExpectSQLTablesReject(
      SQLWCHAR* catalogName, SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
      SQLSMALLINT schemaNameLen, SQLWCHAR* tableName, SQLSMALLINT tableNameLen,
      SQLWCHAR* tableType, SQLSMALLINT tableTypeLen,
      const std::string& expectedState, const std::string& expectedError);

  /**
   * Expect connection to be rejected by the server.
   *
   * @param connectStr Connection string.
   * @return SQL State.
   */
  std::string ExpectConnectionReject(const std::string& connectStr,
                                     const std::string& expectedState,
                                     const std::string& expectedError);

  /**
   * Expect connection to be rejected by the server.
   *
   * @param dsn the DSN for the connection
   * @param username the username for the connection.
   * @param password the password for the connection.
   * @return SQL State.
   */
  std::string ExpectConnectionReject(const std::string& dsn,
                                     const std::string& username,
                                     const std::string& password,
                                     const std::string& expectedState,
                                     const std::string& expectedError);

  /**
   * Connect to Trino (ODBC Ver is 3 by default)
   */
  void ConnectToTS(int odbcVer = SQL_OV_ODBC3);

  /**
   * Disconnect.
   */
  void Disconnect();

  /**
   * Clean up.
   */
  void CleanUp();

  /**
   * Constructor.
   */
  OdbcTestSuite();

  /**
   * Destructor.
   */
  virtual ~OdbcTestSuite();

  /**
   * Get test i8Field.
   *
   * @param idx Index.
   * @return Corresponding i8Field value.
   */
  static int8_t GetTestI8Field(int64_t idx);

  /**
   * Check i8Field test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestI8Value(int idx, int8_t value);

  /**
   * Get test i16Field.
   *
   * @param idx Index.
   * @return Corresponding i16Field value.
   */
  static int16_t GetTestI16Field(int64_t idx);

  /**
   * Check i16Field test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestI16Value(int idx, int16_t value);

  /**
   * Get test i32Field.
   *
   * @param idx Index.
   * @return Corresponding i32Field value.
   */
  static int32_t GetTestI32Field(int64_t idx);

  /**
   * Check i32Field test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestI32Value(int idx, int32_t value);

  /**
   * Get test string.
   *
   * @param idx Index.
   * @return Corresponding test string.
   */
  static std::string GetTestString(int64_t idx);

  /**
   * Check strField test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestStringValue(int idx, const std::string& value);

  /**
   * Get test floatField.
   *
   * @param idx Index.
   * @return Corresponding floatField value.
   */
  static float GetTestFloatField(int64_t idx);

  /**
   * Check floatField test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestFloatValue(int idx, float value);

  /**
   * Get test doubleField.
   *
   * @param idx Index.
   * @return Corresponding doubleField value.
   */
  static double GetTestDoubleField(int64_t idx);

  /**
   * Check doubleField test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestDoubleValue(int idx, double value);

  /**
   * Get test boolField.
   *
   * @param idx Index.
   * @return Corresponding boolField value.
   */
  static bool GetTestBoolField(int64_t idx);

  /**
   * Check boolField test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestBoolValue(int idx, bool value);

  /**
   * Get test dateField.
   *
   * @param idx Index.
   * @param val Output value.
   */
  static void GetTestDateField(int64_t idx, SQL_DATE_STRUCT& val);

  /**
   * Check dateField test value.
   *
   * @param idx Index.
   * @param val Value to test.
   */
  static void CheckTestDateValue(int idx, const SQL_DATE_STRUCT& val);

  /**
   * Get test timeField.
   *
   * @param idx Index.
   * @param val Output value.
   */
  static void GetTestTimeField(int64_t idx, SQL_TIME_STRUCT& val);

  /**
   * Check timeField test value.
   *
   * @param idx Index.
   * @param val Value to test.
   */
  static void CheckTestTimeValue(int idx, const SQL_TIME_STRUCT& val);

  /**
   * Get test timestampField.
   *
   * @param idx Index.
   * @param val Output value.
   */
  static void GetTestTimestampField(int64_t idx, SQL_TIMESTAMP_STRUCT& val);

  /**
   * Check timestampField test value.
   *
   * @param idx Index.
   * @param val Value to test.
   */
  static void CheckTestTimestampValue(int idx, const SQL_TIMESTAMP_STRUCT& val);

  /**
   * Check that SQL error has expected SQL state.
   *
   * @param handleType Handle type.
   * @param handle Handle.
   * @param expectSqlState Expected state.
   */
  void CheckSQLDiagnosticError(int16_t handleType, SQLHANDLE handle,
                               const std::string& expectSqlState);

  /**
   * Check that statement SQL error has expected SQL state.
   *
   * @param expectSqlState Expected state.
   */
  void CheckSQLStatementDiagnosticError(const std::string& expectSqlState);

  /**
   * Check that connection SQL error has expected SQL state.
   *
   * @param expectSqlState Expected state.
   */
  void CheckSQLConnectionDiagnosticError(const std::string& expectSqlState);

  /**
   * Convert string to vector of SQLWCHARs.
   *
   * @param value Query.
   * @return Corresponding vector.
   */
  static std::vector< SQLWCHAR > MakeSqlBuffer(const std::string& value);

  /**
   * Performs SQL query.
   *
   * @param qry Query.
   * @return Result.
   */
  SQLRETURN ExecQuery(const std::string& qry);

  /**
   * Prepares SQL query.
   *
   * @param qry Query.
   * @return Result.
   */
  SQLRETURN PrepareQuery(const std::string& qry);

  /**
   * Retrieve the IAM credentials from environment varibles
   *
   * @param accessKeyId AWSAccessKeyID
   * @param secretKey AWSSecretkey
   */
  void GetIAMCredentials(std::string& accessKeyId,
                         std::string& secretKey) const;

  /**
   * Creates the Okta DSN connection string
   */
  void CreateOktaDsnConnectionString(std::string& connectionString,
                                     const char* host = nullptr,
                                     const char* uid = nullptr,
                                     const char* pwd = nullptr,
                                     const char* appId = nullptr,
                                     const char* roleArn = nullptr,
                                     const char* idpArn = nullptr) const;

  /**
   * Creates the AAD DSN connection string
   */
  void CreateAADDsnConnectionString(
      std::string& connectionString, const char* uid = nullptr,
      const char* pwd = nullptr, const char* appId = nullptr,
      const char* tenantId = nullptr, const char* clientSecret = nullptr,
      const char* roleArn = nullptr, const char* idpArn = nullptr) const;

  /**
   * Creates the generic DSN connection string with uid/pwd and one of
   * { accessKeyId/secretKey, idPUsername, idPPassword } if @param includeTSCred
   * is set to true.
   */
  void CreateGenericDsnConnectionString(
      std::string& connectionString, AuthType::Type testAuthType,
      const std::string& uid, const std::string& pwd,
      bool includeTSCred = false, const std::string& TSUsername = std::string(),
      const std::string& TSPassword = std::string(),
      const std::string& miscOptions = std::string()) const;

  /**
   * Creates the standard DSN connection string for AWS.
   */
  void CreateDsnConnectionStringForAWS(
      std::string& connectionString, const std::string& keyId = std::string(),
      const std::string& secret = std::string(),
      const std::string& miscOptions = std::string()) const;

  /**
   * Add maxRowPerPage to the DSN connection string for AWS.
   * @param connectionString Connection string.
   * @param value The maxRowPerPage value.
   */
  void AddMaxRowPerPage(std::string& connectionString,
                        const std::string& value);

  /**
   * Creates the standard DSN connection string for AWS.
   */
  void CreateDsnConnectionStringForAWS(
      std::string& connectionString, AuthType::Type testAuthType,
      const std::string& credentialsFile,
      const std::string& miscOptions = std::string()) const;

  /** ODBC Environment. */
  SQLHENV env;

  /** ODBC Connect. */
  SQLHDBC dbc;

  /** ODBC Statement. */
  SQLHSTMT stmt;
};

struct if_integration {
  boost::test_tools::assertion_result operator()(test_unit_id) const {
    // implement based on need
    return false;
  }
};
}  // namespace odbc
}  // namespace trino

#endif  // ODBC_TEST_ODBC_TEST_SUITE
