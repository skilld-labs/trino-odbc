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

#include <sql.h>
#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

#include "trino/odbc/connection.h"
#include "trino/odbc/utility.h"
#include "odbc_test_suite.h"
#include "test_utils.h"

using namespace trino;
using namespace trino_test;

using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct AttributesTestSuiteFixture : odbc::OdbcTestSuite {
  /**
   * Constructor.
   */
  AttributesTestSuiteFixture() {
    // No-op
  }

  /**
   * Destructor.
   */
  ~AttributesTestSuiteFixture() override = default;
};

BOOST_FIXTURE_TEST_SUITE(AttributesTestSuite, AttributesTestSuiteFixture)

BOOST_AUTO_TEST_CASE(ConnectionAttributeConnectionDeadGet) {
  ConnectToTS();

  SQLUINTEGER dead;
  SQLRETURN ret;

  ret = SQLGetConnectAttr(dbc, SQL_ATTR_CONNECTION_DEAD, &dead, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  BOOST_REQUIRE_EQUAL(dead, SQL_CD_FALSE);
}

BOOST_AUTO_TEST_CASE(ConnectionAttributeConnectionDeadSet) {
  ConnectToTS();

  SQLUINTEGER dead = SQL_CD_TRUE;
  SQLRETURN ret;

  ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_DEAD, &dead, 0);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  // The error code (HY092) is determined according to
  // https://docs.microsoft.com/en-us/sql/odbc/reference/syntax/sqlsetconnectattr-function#diagnostics
  CheckSQLConnectionDiagnosticError("HY092");
}

BOOST_AUTO_TEST_CASE(ConnectionAttributeConnectionTimeoutGet) {
  ConnectToTS();

  SQLUINTEGER timeout;
  SQLRETURN ret;

  ret = SQLGetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT, &timeout, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  BOOST_REQUIRE_EQUAL(timeout, 0);
}

BOOST_AUTO_TEST_CASE(ConnectionAttributeConnectionTimeoutSet) {
  ConnectToTS();

  SQLRETURN ret;

  ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT,
                          reinterpret_cast< SQLPOINTER >(10), 0);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  std::string error = GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc);
  std::string pattern = "Specified attribute is not supported.";
  if (error.find(pattern) == std::string::npos)
    BOOST_FAIL("'" + error + "' does not match '" + pattern + "'");
}

BOOST_AUTO_TEST_CASE(ConnectionAttributeAutoCommit) {
  ConnectToTS();

  SQLUINTEGER autoCommit;
  SQLRETURN ret;

  ret = SQLGetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, &autoCommit, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(autoCommit, SQL_AUTOCOMMIT_ON);

  ret =
      SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT,
                        reinterpret_cast< SQLPOINTER >(SQL_AUTOCOMMIT_OFF), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  ret = SQLGetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, &autoCommit, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  BOOST_REQUIRE_EQUAL(autoCommit, SQL_AUTOCOMMIT_OFF);
}

BOOST_AUTO_TEST_CASE(ConnectionAttributeMetadataId) {
  ConnectToTS();

  SQLUINTEGER id = -1;
  SQLRETURN ret = SQLGetConnectAttr(dbc, SQL_ATTR_METADATA_ID, &id, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(id, SQL_FALSE);

  ret = SQLSetConnectAttr(dbc, SQL_ATTR_METADATA_ID,
                          reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  id = -1;

  ret = SQLGetConnectAttr(dbc, SQL_ATTR_METADATA_ID, &id, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(id, SQL_TRUE);
}

BOOST_AUTO_TEST_CASE(ConnectionAttributeAutoIPD) {
  ConnectToTS();

  SQLUINTEGER id = -1;
  SQLRETURN ret = SQLGetConnectAttr(dbc, SQL_ATTR_AUTO_IPD, &id, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(id, SQL_FALSE);
}

BOOST_AUTO_TEST_CASE(ConnectionAttributeAsyncEnable) {
  ConnectToTS();

  SQLUINTEGER id = -1;
  SQLRETURN ret = SQLGetConnectAttr(dbc, SQL_ATTR_ASYNC_ENABLE, &id, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(id, SQL_ASYNC_ENABLE_OFF);

  ret =
      SQLSetConnectAttr(dbc, SQL_ATTR_ASYNC_ENABLE,
                        reinterpret_cast< SQLPOINTER >(SQL_ASYNC_ENABLE_ON), 0);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  std::string error = GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc);
  std::string pattern = "Specified attribute is not supported.";
  if (error.find(pattern) == std::string::npos)
    BOOST_FAIL("'" + error + "' does not match '" + pattern + "'");
}

BOOST_AUTO_TEST_CASE(ConnectionAttributeTSLogDebug) {
  ConnectToTS();

  SQLUINTEGER id = -1;
  SQLRETURN ret = SQLSetConnectAttr(
      dbc, SQL_ATTR_TSLOG_DEBUG,
      reinterpret_cast< SQLPOINTER >(LogLevel::Type::DEBUG_LEVEL), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  ret = SQLGetConnectAttr(dbc, SQL_ATTR_TSLOG_DEBUG, &id, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(id,
                      static_cast< SQLUINTEGER >(LogLevel::Type::DEBUG_LEVEL));
}

BOOST_AUTO_TEST_CASE(StatementAttributeCursorScrollable) {
  ConnectToTS();

  SQLULEN scrollable = -1;
  SQLRETURN ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_CURSOR_SCROLLABLE, &scrollable, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(scrollable, SQL_NONSCROLLABLE);
}

BOOST_AUTO_TEST_CASE(StatementAttributeCursorSensitivity) {
  ConnectToTS();

  SQLULEN sensitivity = -1;
  SQLRETURN ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_CURSOR_SENSITIVITY, &sensitivity, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(sensitivity, SQL_INSENSITIVE);
}

BOOST_AUTO_TEST_CASE(StatementAttributeAutoIPD) {
  ConnectToTS();

  SQLULEN autoIPD = -1;
  SQLRETURN ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_ENABLE_AUTO_IPD, &autoIPD, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(autoIPD, SQL_FALSE);
}

BOOST_AUTO_TEST_CASE(StatementAttributeConcurrency) {
  ConnectToTS();

  SQLULEN concurrency = -1;
  SQLRETURN ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_CONCURRENCY, &concurrency, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(concurrency, SQL_CONCUR_READ_ONLY);

  // Attempt to set to supported value
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_CONCURRENCY,
                       reinterpret_cast< SQLPOINTER >(SQL_CONCUR_READ_ONLY), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Attempt to set to unsupported value
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_CONCURRENCY,
                       reinterpret_cast< SQLPOINTER >(SQL_CONCUR_VALUES), 0);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: Only read-only cursors are supported",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(StatementAttributeCursorType) {
  ConnectToTS();

  SQLULEN cursorType = -1;
  SQLRETURN ret = SQLGetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE, &cursorType, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(cursorType, SQL_CURSOR_FORWARD_ONLY);

  // Attempt to set to supported value
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE,
                       reinterpret_cast< SQLPOINTER >(SQL_CURSOR_FORWARD_ONLY),
                       0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Attempt to set to unsupported value
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE,
                       reinterpret_cast< SQLPOINTER >(SQL_CURSOR_STATIC), 0);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: Only forward cursors are currently supported",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(StatementAttributeRowArraySize) {
  // check that statement array size cannot be set to values other than 1
  ConnectToTS();

  SQLINTEGER actual_row_array_size;
  SQLINTEGER resLen = 0;

  // check that statement array size cannot be set to values not equal to 1
  // repeat test for different values
  SQLULEN val = 5;
  SQLRETURN ret =
      SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                     reinterpret_cast< SQLPOINTER >(val), sizeof(val));

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, &actual_row_array_size,
                       sizeof(actual_row_array_size), &resLen);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_CHECK_EQUAL(actual_row_array_size, 5);
}

BOOST_AUTO_TEST_CASE(StatementAttributeRetrieveData) {
  ConnectToTS();

  SQLULEN retrieveData = -1;
  SQLRETURN ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_RETRIEVE_DATA, &retrieveData, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(retrieveData, SQL_RD_ON);

  // Attempt to set to supported value
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_RETRIEVE_DATA,
                       reinterpret_cast< SQLPOINTER >(SQL_RD_ON), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Attempt to set to unsupported value
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_RETRIEVE_DATA,
                       reinterpret_cast< SQLPOINTER >(SQL_RD_OFF), 0);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL(
      "HYC00: SQLFetch can only retrieve data after it positions the cursor",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(StatementAttributeRowBindType) {
  ConnectToTS();

  SQLULEN rowBindType = -1;
  SQLRETURN ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_ROW_BIND_TYPE, &rowBindType, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(rowBindType, SQL_BIND_BY_COLUMN);

  // Attempt to set to supported value
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_BIND_TYPE,
                       reinterpret_cast< SQLPOINTER >(SQL_BIND_BY_COLUMN), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Attempt to set to unsupported value
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_BIND_TYPE,
                       reinterpret_cast< SQLPOINTER >(1UL), 0);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: Only binding by column is currently supported",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(StatementAttributeRowBindOffset) {
  ConnectToTS();

  // Check default value is returned
  SQLULEN* rowBindOffset;
  SQLRETURN ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_ROW_BIND_OFFSET_PTR, &rowBindOffset, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(rowBindOffset, nullptr);

  // Attempt to set pointer
  SQLULEN rowBindOffset1[1] = {2};
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_BIND_OFFSET_PTR,
                       reinterpret_cast< SQLPOINTER >(&rowBindOffset1), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Check pointer is set successfully
  SQLULEN* rowBindOffset2;
  ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_ROW_BIND_OFFSET_PTR, &rowBindOffset2, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_REQUIRE_EQUAL(rowBindOffset2[0], 2);
}

BOOST_AUTO_TEST_CASE(StatementAttributeRowsFetchedPtr) {
  ConnectToTS();

  // Test default value is returned
  SQLULEN* rowsFetchedPtr;
  SQLRETURN ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetchedPtr, 0, 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_REQUIRE_EQUAL(rowsFetchedPtr, nullptr);

  SQLULEN rowsFetchedPtr1[1];

  ret = SQLSetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR,
                       reinterpret_cast< SQLPOINTER >(&rowsFetchedPtr1), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Fetch data once
  std::vector< SQLWCHAR > request =
      MakeSqlBuffer("SELECT * FROM data_queries_test_db.TestScalarTypes");
  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLFetch(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Test setting rowsFetchedPtr has worked and its value is updated
  BOOST_REQUIRE_EQUAL(rowsFetchedPtr1[0], 1);

  // Test non-default value is returned
  SQLULEN* rowsFetchedPtr2;
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetchedPtr2, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_REQUIRE_EQUAL(rowsFetchedPtr2[0], 1);
}

BOOST_AUTO_TEST_CASE(StatementAttributeRowsStatusesPtr) {
  ConnectToTS();

  // Test default value is returned
  SQLUSMALLINT* rowsStatusesPtr;
  SQLRETURN ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, &rowsStatusesPtr, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_REQUIRE_EQUAL(rowsStatusesPtr, nullptr);

  // Row Array Size is 1, so only one row status can be returned at a time.
  SQLUSMALLINT rowsStatusesPtr1[1];
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR,
                       reinterpret_cast< SQLPOINTER >(&rowsStatusesPtr1), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  std::vector< SQLWCHAR > request = MakeSqlBuffer("SELECT 1");
  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Fetch data
  ret = SQLFetch(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_REQUIRE_EQUAL(rowsStatusesPtr1[0], SQL_ROW_SUCCESS);

  ret = SQLFetch(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);

  BOOST_REQUIRE_EQUAL(rowsStatusesPtr1[0], SQL_ROW_NOROW);

  // Test non-default value is returned
  SQLUSMALLINT* rowsStatusesPtr2;
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, &rowsStatusesPtr2, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_REQUIRE_EQUAL(rowsStatusesPtr2[0], SQL_ROW_NOROW);
}

BOOST_AUTO_TEST_CASE(StatementAttributeParamBindType) {
  ConnectToTS();

  SQLULEN paramBindType = -1;
  SQLRETURN ret =
      SQLGetStmtAttr(stmt, SQL_ATTR_PARAM_BIND_TYPE, &paramBindType, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(paramBindType, SQL_PARAM_BIND_BY_COLUMN);

  // Attempt to set to supported value
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_PARAM_BIND_TYPE,
                       reinterpret_cast< SQLPOINTER >(SQL_PARAM_BIND_BY_COLUMN),
                       0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Attempt to set to unsupported arbitrary value 1UL
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_PARAM_BIND_TYPE,
                       reinterpret_cast< SQLPOINTER >(1UL), 0);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: Only binding by column is currently supported",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(StatementAttributeMetadataId) {
  ConnectToTS();

  SQLULEN id = -1;
  SQLRETURN ret = SQLGetStmtAttr(stmt, SQL_ATTR_METADATA_ID, &id, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(id, SQL_FALSE);

  // Attempt to set to change value
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_METADATA_ID,
                       reinterpret_cast< SQLPOINTER >(SQL_TRUE), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLGetStmtAttr(stmt, SQL_ATTR_METADATA_ID, &id, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(id, SQL_TRUE);
}

BOOST_AUTO_TEST_CASE(StatementAttributeRowNumberSQLExecDirect) {
  ConnectToTS();

  std::vector< SQLWCHAR > request =
      MakeSqlBuffer("SELECT * FROM data_queries_test_db.TestScalarTypes");
  SQLRETURN ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLFetch(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Check row number 1 is returned
  SQLULEN rowNum = -1;
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(rowNum, 1);

  SQLULEN rowCount = 1;
  do {
    ret = SQLFetch(stmt);
    if (ret == SQL_NO_DATA) {
      break;
    } else if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }
    rowCount++;
    ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
    BOOST_REQUIRE_EQUAL(rowNum, rowCount);
  } while (true);

  // Total row count should be 6
  BOOST_REQUIRE_EQUAL(rowNum, 6);

  // Get row number after SQLFetch returns SQL_NO_DATA. This behavior is
  // controlled by the driver manager
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL("24000: [unixODBC][Driver Manager]Invalid cursor state",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#elif defined(__APPLE__)
  // iODBC driver manager does not return error message after calling SQLFetch
  // returns SQL_NO_DATA. This behavior is controlled by the driver manager.
  BOOST_REQUIRE(SQL_SUCCEEDED(ret));
  BOOST_REQUIRE_EQUAL(rowNum, 0);
#else
  BOOST_REQUIRE_EQUAL(
      "24000: [Microsoft][ODBC Driver Manager] Invalid cursor state",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(StatementAttributeRowNumberSQLTables) {
  ConnectToTS();

  std::vector< SQLWCHAR > empty = {0};
  std::vector< SQLWCHAR > testTableName = MakeSqlBuffer("IoTMulti");
  std::vector< SQLWCHAR > databaseName = MakeSqlBuffer("meta_queries_test_db");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLTables(stmt, empty.data(), SQL_NTS, databaseName.data(), SQL_NTS,
                    testTableName.data(), SQL_NTS, empty.data(), SQL_NTS);
  } else {
    ret = SQLTables(stmt, databaseName.data(), SQL_NTS, empty.data(), SQL_NTS,
                    testTableName.data(), SQL_NTS, empty.data(), SQL_NTS);
  }

  ret = SQLFetch(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Check row number 1 is returned
  SQLULEN rowNum = -1;
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(rowNum, 1);

  SQLULEN rowCount = 1;
  do {
    ret = SQLFetch(stmt);
    if (ret == SQL_NO_DATA) {
      break;
    } else if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }
    rowCount++;
    ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
    BOOST_REQUIRE_EQUAL(rowNum, rowCount);
  } while (true);

  // Get row number after SQLFetch returns SQL_NO_DATA. This behavior is
  // controlled by the driver manager
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL("24000: [unixODBC][Driver Manager]Invalid cursor state",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#elif defined(__APPLE__)
  // iODBC driver manager does not return error message after calling SQLFetch
  // returns SQL_NO_DATA. This behavior is controlled by the driver manager.
  BOOST_REQUIRE(SQL_SUCCEEDED(ret));
  BOOST_REQUIRE_EQUAL(rowNum, 0);
#else
  BOOST_REQUIRE_EQUAL(
      "24000: [Microsoft][ODBC Driver Manager] Invalid cursor state",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(StatementAttributeRowNumberSQLColumns) {
  ConnectToTS();

  std::vector< SQLWCHAR > table = MakeSqlBuffer("TestScalarTypes");
  std::vector< SQLWCHAR > databaseName = MakeSqlBuffer("data_queries_test_db");
  SQLRETURN ret;

  if (DATABASE_AS_SCHEMA) {
    ret = SQLColumns(stmt, nullptr, 0, databaseName.data(), SQL_NTS,
                     table.data(), SQL_NTS, nullptr, 0);
  } else {
    ret = SQLColumns(stmt, databaseName.data(), SQL_NTS, nullptr, 0,
                     table.data(), SQL_NTS, nullptr, 0);
  }

  ret = SQLFetch(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Check row number 1 is returned
  SQLULEN rowNum = -1;
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(rowNum, 1);

  SQLULEN rowCount = 1;
  do {
    ret = SQLFetch(stmt);
    if (ret == SQL_NO_DATA) {
      break;
    } else if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }
    rowCount++;
    ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
    BOOST_REQUIRE_EQUAL(rowNum, rowCount);
  } while (true);

  // Get row number after SQLFetch returns SQL_NO_DATA. This behavior is
  // controlled by the driver manager
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL("24000: [unixODBC][Driver Manager]Invalid cursor state",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#elif defined(__APPLE__)
  // iODBC driver manager does not return error message after calling SQLFetch
  // returns SQL_NO_DATA. This behavior is controlled by the driver manager.
  BOOST_REQUIRE(SQL_SUCCEEDED(ret));
  BOOST_REQUIRE_EQUAL(rowNum, 0);
#else
  BOOST_REQUIRE_EQUAL(
      "24000: [Microsoft][ODBC Driver Manager] Invalid cursor state",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(StatementAttributeRowNumberSQLGetTypeInfo) {
  ConnectToTS();

  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_ALL_TYPES);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLFetch(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Check row number 1 is returned
  SQLULEN rowNum = -1;
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(rowNum, 1);

  SQLULEN rowCount = 1;
  do {
    ret = SQLFetch(stmt);
    if (ret == SQL_NO_DATA) {
      break;
    } else if (!SQL_SUCCEEDED(ret)) {
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }
    rowCount++;
    ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
    BOOST_REQUIRE_EQUAL(rowNum, rowCount);
  } while (true);

  // Get row number after SQLFetch returns SQL_NO_DATA. This behavior is
  // controlled by the driver manager
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNum, 0, 0);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL("24000: [unixODBC][Driver Manager]Invalid cursor state",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#elif defined(__APPLE__)
  // iODBC driver manager does not return error message after calling SQLFetch
  // returns SQL_NO_DATA. This behavior is controlled by the driver manager.
  BOOST_REQUIRE(SQL_SUCCEEDED(ret));
  BOOST_REQUIRE_EQUAL(rowNum, 0);
#else
  BOOST_REQUIRE_EQUAL(
      "24000: [Microsoft][ODBC Driver Manager] Invalid cursor state",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

/**
 * Check that environment returns expected version of ODBC standard.
 *
 * 1. Establish connection using ODBC driver.
 * 2. Get current ODBC version from env handle.
 * 3. Check that version is of the expected value.
 */
BOOST_AUTO_TEST_CASE(EnvironmentAttributeDriverVersion) {
  ConnectToTS();

  SQLINTEGER version;
  SQLRETURN ret = SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, &version, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_ENV, env);

  BOOST_CHECK_EQUAL(version, SQL_OV_ODBC3);
}

BOOST_AUTO_TEST_CASE(EnvironmentAttributeDriverVersionODBC2) {
  // set ODBC version to 2 and check the value is correct.
  ConnectToTS(SQL_OV_ODBC2);

  SQLINTEGER version;
  SQLRETURN ret = SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, &version, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_ENV, env);

  BOOST_CHECK_EQUAL(version, SQL_OV_ODBC2);
}

BOOST_AUTO_TEST_CASE(EnvironmentAttributeCPMatch) {
  SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

  BOOST_REQUIRE(env != NULL);

  // We want ODBC 3 support
  SQLRETURN ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION,
                                reinterpret_cast< void* >(SQL_OV_ODBC3), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_ENV, env);

  // Attempt to set SQL_ATTR_CP_MATCH to non-default value
  ret = SQLSetEnvAttr(env, SQL_ATTR_CP_MATCH,
                      reinterpret_cast< void* >(SQL_CP_RELAXED_MATCH), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_ENV, env);

  SQLINTEGER cpMatch;
  ret = SQLGetEnvAttr(env, SQL_ATTR_CP_MATCH, &cpMatch, 0, 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_ENV, env);

  BOOST_CHECK_EQUAL(cpMatch, SQL_CP_RELAXED_MATCH);
}

BOOST_AUTO_TEST_CASE(EnvironmentAttributeCPMatchDefault) {
  ConnectToTS();

  SQLINTEGER cpMatch;
  SQLRETURN ret = SQLGetEnvAttr(env, SQL_ATTR_CP_MATCH, &cpMatch, 0, 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_ENV, env);

  // Check default value is returned
  BOOST_CHECK_EQUAL(cpMatch, SQL_CP_STRICT_MATCH);
}

#if defined(__APPLE__)
#define CHECK_SET_IGNORED_OPTION(option, value)              \
  {                                                          \
    SQLRETURN ret = SQLSetConnectOption(dbc, option, value); \
    BOOST_CHECK_EQUAL(ret, SQL_SUCCESS_WITH_INFO);           \
    CheckSQLConnectionDiagnosticError("01000");              \
  }
#else
#define CHECK_SET_IGNORED_OPTION(option, value)                  \
  {                                                              \
    SQLRETURN ret = SQLSetConnectOption(dbc, option, value);     \
    BOOST_CHECK_EQUAL(ret, SQL_SUCCESS_WITH_INFO);               \
    CheckSQLConnectionDiagnosticError("01000");                  \
    BOOST_CHECK_EQUAL("01000: Specified attribute is ignored.",  \
                      GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc)); \
  }
#endif

// Test options that could be set
BOOST_AUTO_TEST_CASE(ConnectionSetConnectOption) {
  ConnectToTS(SQL_OV_ODBC2);

  SQLRETURN ret = SQLSetConnectOption(dbc, SQL_BIND_TYPE, SQL_BIND_BY_COLUMN);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  ret = SQLSetConnectOption(dbc, SQL_CONCURRENCY, SQL_CONCUR_READ_ONLY);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  ret = SQLSetConnectOption(dbc, SQL_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  ret = SQLSetConnectOption(dbc, SQL_RETRIEVE_DATA, SQL_RD_ON);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  ret = SQLSetConnectOption(dbc, SQL_ROWSET_SIZE, 100);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
}

BOOST_AUTO_TEST_CASE(ConnectionSetConnectOptionUnsupportedValue) {
  ConnectToTS(SQL_OV_ODBC2);
  // error messages are hidden by driver manager
  SQLRETURN ret = SQLSetConnectOption(dbc, SQL_BIND_TYPE, 1);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  ret = SQLSetConnectOption(dbc, SQL_CONCURRENCY, SQL_CONCUR_LOCK);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  ret = SQLSetConnectOption(dbc, SQL_CURSOR_TYPE, SQL_CURSOR_KEYSET_DRIVEN);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  ret = SQLSetConnectOption(dbc, SQL_RETRIEVE_DATA, SQL_RD_OFF);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  ret = SQLSetConnectOption(dbc, SQL_ROWSET_SIZE, 2000);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
}

// Test not supported connection options
BOOST_AUTO_TEST_CASE(ConnectionSetConnectOptionUnsupportedOption) {
  ConnectToTS(SQL_OV_ODBC2);

  SQLRETURN ret = SQLSetConnectOption(dbc, SQL_USE_BOOKMARKS, SQL_TRUE);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);
#if defined(__linux__)
  CheckSQLConnectionDiagnosticError("S1C00");
#else
  CheckSQLConnectionDiagnosticError("HYC00");
#endif

#if defined(__APPLE__)
  std::string pattern = "Cannot find ODBC error message";
#else
  std::string pattern = "Specified attribute is not supported";
#endif

  BOOST_CHECK(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc).find(pattern)
              != std::string::npos);

  ret = SQLSetConnectOption(dbc, SQL_SIMULATE_CURSOR, SQL_TRUE);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);
#if defined(__linux__)
  CheckSQLConnectionDiagnosticError("S1C00");
#else
  CheckSQLConnectionDiagnosticError("HYC00");
#endif
  BOOST_CHECK(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc).find(pattern)
              != std::string::npos);
}

// Test ignored connection options
BOOST_AUTO_TEST_CASE(ConnectionSetConnectOptionIgnored) {
  ConnectToTS(SQL_OV_ODBC2);

  CHECK_SET_IGNORED_OPTION(SQL_NOSCAN, SQL_TRUE);
  CHECK_SET_IGNORED_OPTION(SQL_QUERY_TIMEOUT, 10);
  CHECK_SET_IGNORED_OPTION(SQL_MAX_ROWS, 20);
  CHECK_SET_IGNORED_OPTION(SQL_MAX_LENGTH, 20);
  CHECK_SET_IGNORED_OPTION(SQL_KEYSET_SIZE, 100);
  CHECK_SET_IGNORED_OPTION(SQL_ASYNC_ENABLE, SQL_TRUE);
  CHECK_SET_IGNORED_OPTION(SQL_TXN_ISOLATION, SQL_TXN_READ_COMMITTED);
  CHECK_SET_IGNORED_OPTION(SQL_ACCESS_MODE, SQL_MODE_READ_ONLY);
  CHECK_SET_IGNORED_OPTION(SQL_CURRENT_QUALIFIER,
                           reinterpret_cast< SQLULEN >("test"));
  CHECK_SET_IGNORED_OPTION(SQL_PACKET_SIZE, 100);
  CHECK_SET_IGNORED_OPTION(SQL_QUIET_MODE, NULL);
  CHECK_SET_IGNORED_OPTION(SQL_LOGIN_TIMEOUT, 10);
  CHECK_SET_IGNORED_OPTION(SQL_TRANSLATE_DLL,
                           reinterpret_cast< SQLULEN >("trace"));
  CHECK_SET_IGNORED_OPTION(SQL_TRANSLATE_OPTION, SQL_FALSE);
}

// Test connection options set by driver manager
BOOST_AUTO_TEST_CASE(ConnectionSetConnectOptionDMCase) {
  ConnectToTS(SQL_OV_ODBC2);

  SQLRETURN ret =
      SQLSetConnectOption(dbc, SQL_ODBC_CURSORS, SQL_CUR_USE_DRIVER);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);

  ret = SQLSetConnectOption(dbc, SQL_OPT_TRACE, SQL_OPT_TRACE_ON);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  ret = SQLSetConnectOption(dbc, SQL_OPT_TRACEFILE,
                            reinterpret_cast< SQLULEN >("trace"));
#if defined(__APPLE__)
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);
  CheckSQLConnectionDiagnosticError("IM013");
  BOOST_CHECK(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc).find("Trace file error")
              != std::string::npos);
#else
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
#endif
}

// SQL_ATTR_METADATA_ID could not be set/get by
// SQLSetConnectOption/SQLGetConnectOption on macOS
#if !defined(__APPLE__)
BOOST_AUTO_TEST_CASE(ConnectionSetAndGetConnectOption) {
  ConnectToTS();

  SQLRETURN ret = SQLSetConnectOption(dbc, SQL_ATTR_METADATA_ID, SQL_TRUE);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  SQLUINTEGER id = 0;
  ret = SQLGetConnectOption(dbc, SQL_ATTR_METADATA_ID, &id);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(id, SQL_TRUE);
}
#endif

#if defined(__APPLE__)
#define CHECK_GET_IGNORED_OPTION(option)                      \
  {                                                           \
    SQLULEN value = 0;                                        \
    SQLRETURN ret = SQLGetConnectOption(dbc, option, &value); \
    BOOST_CHECK_EQUAL(ret, SQL_SUCCESS_WITH_INFO);            \
    CheckSQLConnectionDiagnosticError("01000");               \
  }
#else
#define CHECK_GET_IGNORED_OPTION(option)                         \
  {                                                              \
    SQLULEN value = 0;                                           \
    SQLRETURN ret = SQLGetConnectOption(dbc, option, &value);    \
    BOOST_CHECK_EQUAL(ret, SQL_SUCCESS_WITH_INFO);               \
    CheckSQLConnectionDiagnosticError("01000");                  \
    BOOST_CHECK_EQUAL("01000: Specified attribute is ignored.",  \
                      GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc)); \
  }
#endif

// Test options returned from driver
BOOST_AUTO_TEST_CASE(ConnectionGetConnectOption) {
  ConnectToTS(SQL_OV_ODBC2);

  SQLUINTEGER value;

  SQLRETURN ret = SQLGetConnectOption(dbc, SQL_AUTOCOMMIT, &value);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(value, SQL_AUTOCOMMIT_ON);

  ret = SQLGetConnectOption(dbc, SQL_ATTR_CONNECTION_DEAD, &value);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(value, SQL_FALSE);
}

// Test ignored connection options
BOOST_AUTO_TEST_CASE(ConnectionGetConnectOptionIgnored) {
  ConnectToTS(SQL_OV_ODBC2);

  CHECK_GET_IGNORED_OPTION(SQL_QUERY_TIMEOUT);
  CHECK_GET_IGNORED_OPTION(SQL_ACCESS_MODE);
  CHECK_GET_IGNORED_OPTION(SQL_TXN_ISOLATION);
  CHECK_GET_IGNORED_OPTION(SQL_CURRENT_QUALIFIER);
  CHECK_GET_IGNORED_OPTION(SQL_PACKET_SIZE);
  CHECK_GET_IGNORED_OPTION(SQL_QUIET_MODE);
  CHECK_GET_IGNORED_OPTION(SQL_LOGIN_TIMEOUT);
  CHECK_GET_IGNORED_OPTION(SQL_TRANSLATE_DLL);
  CHECK_GET_IGNORED_OPTION(SQL_TRANSLATE_OPTION);
#if defined(__APPLE__)
  CHECK_GET_IGNORED_OPTION(SQL_ODBC_CURSORS);
#endif
}

// Test connection options returned from driver manager
BOOST_AUTO_TEST_CASE(ConnectionGetConnectOptionDMCase) {
  ConnectToTS(SQL_OV_ODBC2);

  SQLULEN value = 0;

  SQLRETURN ret = SQLGetConnectOption(dbc, SQL_OPT_TRACE, &value);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

#if defined(_WIN32) || defined(__linux__)
  ret = SQLGetConnectOption(dbc, SQL_ODBC_CURSORS, &value);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
#endif

  SQLCHAR strvalue[1024];
  ret = SQLGetConnectOption(dbc, SQL_OPT_TRACEFILE, strvalue);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
}

BOOST_AUTO_TEST_CASE(StatementOptionSupported) {
  ConnectToTS(SQL_OV_ODBC2);

  SQLULEN value = 0;
  SQLRETURN ret = SQLGetStmtOption(stmt, SQL_ROWSET_SIZE, (SQLPOINTER)&value);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_CHECK_EQUAL(value, 1);

  ret = SQLGetStmtOption(stmt, SQL_BIND_TYPE, (SQLPOINTER)&value);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_CHECK_EQUAL(value, SQL_BIND_BY_COLUMN);

  ret = SQLGetStmtOption(stmt, SQL_CONCURRENCY, (SQLPOINTER)&value);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_CHECK_EQUAL(value, SQL_CONCUR_READ_ONLY);

  ret = SQLGetStmtOption(stmt, SQL_CURSOR_TYPE, (SQLPOINTER)&value);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_CHECK_EQUAL(value, SQL_CURSOR_FORWARD_ONLY);

  ret = SQLGetStmtOption(stmt, SQL_RETRIEVE_DATA, (SQLPOINTER)&value);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_CHECK_EQUAL(value, SQL_RD_ON);
}

#define CHECK_GET_OPTION_NOTSUPPORTED(option)                           \
  {                                                                     \
    SQLULEN value = 0;                                                  \
    SQLRETURN ret = SQLGetStmtOption(stmt, option, (SQLPOINTER)&value); \
    BOOST_CHECK_EQUAL(ret, SQL_ERROR);                                  \
  }

BOOST_AUTO_TEST_CASE(StatementOptionNotSupported) {
  ConnectToTS(SQL_OV_ODBC2);

  // These unsupported options are blocked by driver manager
  CHECK_GET_OPTION_NOTSUPPORTED(SQL_GET_BOOKMARK);
  CHECK_GET_OPTION_NOTSUPPORTED(SQL_ROW_NUMBER);
  CHECK_GET_OPTION_NOTSUPPORTED(SQL_ASYNC_ENABLE);
  CHECK_GET_OPTION_NOTSUPPORTED(SQL_KEYSET_SIZE);
  CHECK_GET_OPTION_NOTSUPPORTED(SQL_MAX_LENGTH);
  CHECK_GET_OPTION_NOTSUPPORTED(SQL_MAX_ROWS);
  CHECK_GET_OPTION_NOTSUPPORTED(SQL_NOSCAN);
  CHECK_GET_OPTION_NOTSUPPORTED(SQL_QUERY_TIMEOUT);
  CHECK_GET_OPTION_NOTSUPPORTED(SQL_SIMULATE_CURSOR);
  CHECK_GET_OPTION_NOTSUPPORTED(SQL_USE_BOOKMARKS);
}

BOOST_AUTO_TEST_SUITE_END()
