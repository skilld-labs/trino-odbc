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
#include <sqltypes.h>
#include "timestream/odbc/utility.h"
#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <string>
#include <vector>

#include "odbc_test_suite.h"
#include "test_utils.h"

#include "timestream/odbc/log.h"
#include "timestream/odbc/config/configuration.h"
#include "timestream/odbc/config/connection_string_parser.h"
#include "timestream/odbc/utility.h"

using namespace timestream;
using namespace timestream_test;

using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct ApiRobustnessTestSuiteFixture : public timestream::odbc::OdbcTestSuite {
  /**
   * Constructor.
   */
  ApiRobustnessTestSuiteFixture() {
    // No-op
  }

  /**
   * Check that SQLFetchScroll does not crash with unsupported orientation.
   *
   * @param orientation Fetch orientation.
   */
  void CheckFetchScrollUnsupportedOrientation(SQLUSMALLINT orientation) {
    ConnectToTS();

    double doubleField = 0.0;

    // Binding column.
    SQLRETURN ret = SQLBindCol(stmt, 7, SQL_C_DOUBLE, &doubleField, 0, 0);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    std::vector< SQLWCHAR > request = MakeSqlBuffer(
        "select * from data_queries_test_db.TestComplexTypes order by time, measure_value::double");

    ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0);
    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_CHECK_EQUAL(abs(doubleField - 35.2) < 0.1, true);

    ret = SQLFetchScroll(stmt, orientation, 0);

    // Operation is not supported, only forward is supported. However, there
    // should be no crash.
    BOOST_CHECK(ret == SQL_ERROR);

    CheckSQLStatementDiagnosticError("HYC00");
  }

  /**
   * Destructor.
   */
  virtual ~ApiRobustnessTestSuiteFixture() {
    // No-op.
  }
};

BOOST_FIXTURE_TEST_SUITE(ApiRobustnessTestSuite, ApiRobustnessTestSuiteFixture)

#ifndef __APPLE__
BOOST_AUTO_TEST_CASE(TestSQLDriverConnect) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  Prepare();

  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  std::vector< SQLWCHAR > connectStr(dsnConnectionString.begin(),
                                     dsnConnectionString.end());
  SQLSMALLINT connectStrLen = static_cast< SQLSMALLINT >(connectStr.size());

  SQLWCHAR outStr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outStrLen;

  // Normal connect.
  SQLRETURN ret =
      SQLDriverConnect(dbc, NULL, &connectStr[0], connectStrLen, outStr,
                       ODBC_BUFFER_SIZE, &outStrLen, SQL_DRIVER_COMPLETE);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  SQLDisconnect(dbc);

  // Null out string resulting length.
  SQLDriverConnect(dbc, NULL, &connectStr[0], connectStrLen, outStr,
                   ODBC_BUFFER_SIZE, 0, SQL_DRIVER_COMPLETE);
  SQLDisconnect(dbc);

  // Null out string buffer length.
  SQLDriverConnect(dbc, NULL, &connectStr[0], connectStrLen, outStr, 0,
                   &outStrLen, SQL_DRIVER_COMPLETE);
  SQLDisconnect(dbc);

  // Null out string.
  SQLDriverConnect(dbc, NULL, &connectStr[0], connectStrLen, 0,
                   ODBC_BUFFER_SIZE, &outStrLen, SQL_DRIVER_COMPLETE);
  SQLDisconnect(dbc);

  // Null all.
  SQLDriverConnect(dbc, NULL, &connectStr[0], connectStrLen, 0, 0, 0,
                   SQL_DRIVER_COMPLETE);
  SQLDisconnect(dbc);

  // Reduced output buffer length. Test boundary condition of output buffer
  // 9 is chosen as arbitrary number guaranteed to be smaller than the actual
  // string
  SQLSMALLINT reducedOutStrLen = 9;
  ret = SQLDriverConnect(dbc, NULL, &connectStr[0], connectStrLen, outStr,
                         reducedOutStrLen + 1, &outStrLen, SQL_DRIVER_COMPLETE);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(outStrLen, reducedOutStrLen);
  std::vector< SQLWCHAR > expectedOutStr(connectStr.begin(),
                                         connectStr.begin() + reducedOutStrLen);
  expectedOutStr.push_back(0);
  std::vector< SQLWCHAR > actualOutStr(outStrLen + 1);
  for (int i = 0; i <= outStrLen; i++) {
    actualOutStr[i] = outStr[i];
  }

  // The following behavior should be considered when changing this test:
  // The ODBC Driver Manager on Windows changes the input connection string
  // before passing it to the SQLDriverConnect function.
  // I.e., "driver=...", "uid=...", "pwd=..." becomes "DRIVER=...", "UID=...",
  // "PWD=..." and are moved to the head of the connection string.
  BOOST_CHECK_EQUAL_COLLECTIONS(actualOutStr.begin(), actualOutStr.end(),
                                expectedOutStr.begin(), expectedOutStr.end());
  SQLDisconnect(dbc);
}
#endif

BOOST_AUTO_TEST_CASE(TestSQLConnect) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.
  ConnectToTS();

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLSMALLINT resLen = 0;

  // Everything is ok.
  SQLRETURN ret =
      SQLGetInfo(dbc, SQL_DRIVER_NAME, buffer, sizeof(buffer), &resLen);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  // Resulting length is null.
  SQLGetInfo(dbc, SQL_DRIVER_NAME, buffer, sizeof(buffer), 0);

  // Buffer length is null.
  SQLGetInfo(dbc, SQL_DRIVER_NAME, buffer, 0, &resLen);

  // Buffer is null.
  SQLGetInfo(dbc, SQL_DRIVER_NAME, 0, sizeof(buffer), &resLen);

  // Unknown info.
  SQLGetInfo(dbc, -1, buffer, sizeof(buffer), &resLen);

  // All nulls.
  SQLGetInfo(dbc, SQL_DRIVER_NAME, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLPrepare) {
  ConnectToTS();

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"sampleDB\".IoTMulti");

  // Everything is ok.
  SQLRETURN ret = SQLPrepare(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLCloseCursor(stmt);

  // Value length is null.
  ret = SQLPrepare(stmt, sql.data(), 0);
#ifdef __APPLE__
  // iODBC returns SUCCESS
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
#else
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
#endif

  SQLCloseCursor(stmt);

  // Value is null.
  ret = SQLPrepare(stmt, 0, SQL_NTS);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  SQLCloseCursor(stmt);

  // All nulls.
  ret = SQLPrepare(stmt, 0, 0);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  SQLCloseCursor(stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLExecDirect) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.
  ConnectToTS();

  std::vector< SQLWCHAR > sql = MakeSqlBuffer("SELECT 1");

  // Everything is ok.
  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLCloseCursor(stmt);

  // Value length is null.
  SQLExecDirect(stmt, sql.data(), 0);

  SQLCloseCursor(stmt);

  // Value is null.
  SQLExecDirect(stmt, 0, SQL_NTS);

  SQLCloseCursor(stmt);

  // All nulls.
  SQLExecDirect(stmt, 0, 0);

  SQLCloseCursor(stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLExtendedFetch) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.
  ConnectToTS();

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM data_queries_test_db.TestComplexTypes");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLULEN rowCount;
  SQLUSMALLINT rowStatus[16];

  // Everything is ok.
  ret = SQLExtendedFetch(stmt, SQL_FETCH_NEXT, 0, &rowCount, rowStatus);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Row count is null.
  ret = SQLExtendedFetch(stmt, SQL_FETCH_NEXT, 0, 0, rowStatus);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Row statuses is null.
  SQLExtendedFetch(stmt, SQL_FETCH_NEXT, 0, &rowCount, 0);

  // All nulls.
  SQLExtendedFetch(stmt, SQL_FETCH_NEXT, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLNumResultCols) {
  ConnectToTS();

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM data_queries_test_db.TestComplexTypes");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLSMALLINT columnCount;

  // Everything is ok.
  ret = SQLNumResultCols(stmt, &columnCount);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_CHECK_EQUAL(7, columnCount);

  // Test with column count is null.
  ret = SQLNumResultCols(stmt, 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLTables) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  std::vector< SQLWCHAR > catalogName = {0};
  std::vector< SQLWCHAR > schemaName = {0};
  std::vector< SQLWCHAR > tableName = {0};
  std::vector< SQLWCHAR > tableType = {0};

  // Everything is ok.
  SQLRETURN ret =
      SQLTables(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                tableName.data(), SQL_NTS, tableType.data(), SQL_NTS);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Sizes are nulls.
  ret = SQLTables(stmt, catalogName.data(), 0, schemaName.data(), 0,
                  tableName.data(), 0, tableType.data(), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Values are nulls.
  ret = SQLTables(stmt, 0, SQL_NTS, 0, SQL_NTS, 0, SQL_NTS, 0, SQL_NTS);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // All nulls.
  ret = SQLTables(stmt, 0, 0, 0, 0, 0, 0, 0, 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLColumns) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  std::vector< SQLWCHAR > catalogName = {0};
  std::vector< SQLWCHAR > schemaName = {0};
  std::vector< SQLWCHAR > tableName = {0};
  std::vector< SQLWCHAR > columnName = {0};

  // Everything is ok.
  SQLRETURN ret =
      SQLColumns(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), SQL_NTS, columnName.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Sizes are nulls.
  ret = SQLColumns(stmt, catalogName.data(), 0, schemaName.data(), 0,
                   tableName.data(), 0, columnName.data(), 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Values are nulls.
  ret = SQLColumns(stmt, 0, SQL_NTS, 0, SQL_NTS, 0, SQL_NTS, 0, SQL_NTS);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // All nulls.
  ret = SQLColumns(stmt, 0, 0, 0, 0, 0, 0, 0, 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLBindCol) {
  ConnectToTS();

  SQLINTEGER ind1;
  SQLLEN len1 = 0;

  // Everything is ok.
  SQLRETURN ret = SQLBindCol(stmt, 1, SQL_C_SLONG, &ind1, sizeof(ind1), &len1);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Unsupported data types
  ret = SQLBindCol(stmt, 1, SQL_C_GUID, &ind1, sizeof(ind1), &len1);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
#ifdef _WIN32
  CheckSQLStatementDiagnosticError("HYC00");
#else
  CheckSQLStatementDiagnosticError("HY003");
#endif  // _WIN32

  // Size is negative.
  ret = SQLBindCol(stmt, 1, SQL_C_SLONG, &ind1, -1, &len1);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
#ifdef __APPLE__
  CheckSQLStatementDiagnosticError("S1090");
#else
  CheckSQLStatementDiagnosticError("HY090");
#endif

  // Size is 0 for string
  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, &ind1, 0, &len1);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HY090");

  // Size is 0 for non-string
  ret = SQLBindCol(stmt, 1, SQL_C_SLONG, &ind1, 0, &len1);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  // Res size is null.
  ret = SQLBindCol(stmt, 2, SQL_C_SLONG, &ind1, sizeof(ind1), 0);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  // Value is null.
  ret = SQLBindCol(stmt, 3, SQL_C_SLONG, 0, sizeof(ind1), &len1);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);
}

BOOST_AUTO_TEST_CASE(TestSQLNativeSql) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"sampleDB\".IoTMulti");
  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLINTEGER resLen = 0;

  // Everything is ok.
  SQLRETURN ret =
      SQLNativeSql(dbc, sql.data(), SQL_NTS, buffer, ODBC_BUFFER_SIZE, &resLen);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Confirm boundary condition.
  SQLINTEGER reducedLength = 8;
  ret = SQLNativeSql(dbc, sql.data(), SQL_NTS, buffer, reducedLength + 1,
                     &resLen);
  BOOST_CHECK_EQUAL(SQL_SUCCESS_WITH_INFO, ret);
  BOOST_CHECK_EQUAL(reducedLength, resLen);

  // Value size is null.
  ret = SQLNativeSql(dbc, sql.data(), 0, buffer, ODBC_BUFFER_SIZE, &resLen);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(0, resLen);

  // Buffer size is null.
  ret = SQLNativeSql(dbc, sql.data(), SQL_NTS, buffer, 0, &resLen);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  // Res size is null.
  ret =
      SQLNativeSql(dbc, sql.data(), SQL_NTS, buffer, ODBC_BUFFER_SIZE, nullptr);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Value is null.
  ret = SQLNativeSql(dbc, nullptr, SQL_NTS, buffer, ODBC_BUFFER_SIZE, &resLen);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  // Buffer is null.
  ret = SQLNativeSql(dbc, sql.data(), SQL_NTS, nullptr, ODBC_BUFFER_SIZE,
                     &resLen);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(sql.size() - 1, resLen);

  // All nulls.
  ret = SQLNativeSql(dbc, nullptr, 0, nullptr, 0, nullptr);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);
}

BOOST_AUTO_TEST_CASE(TestSQLColAttribute) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM meta_queries_test_db.TestColumnsMetadata1");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLSMALLINT resLen = 0;
  SQLLEN numericAttr = 0;

  // Everything is ok. Character attribute.
  ret = SQLColAttribute(stmt, 1, SQL_COLUMN_TABLE_NAME, buffer, sizeof(buffer),
                        &resLen, &numericAttr);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Everything is ok. Numeric attribute.
  ret = SQLColAttribute(stmt, 1, SQL_DESC_COUNT, buffer, sizeof(buffer),
                        &resLen, &numericAttr);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLColAttribute(stmt, 1, SQL_DESC_BASE_COLUMN_NAME, buffer, sizeof(buffer),
                  &resLen, 0);
#ifndef __APPLE__
  // On macOS machine with iODBC, iODBC driver manager will attempt to access
  // StringLengthPtr (the 6th parameter of SQLColAttribute) when buffer is
  // non-empty, and segmentation fault will occur if StringLengthPtr is nullptr.
  // This behavior is out of the driver's control.
  SQLColAttribute(stmt, 1, SQL_DESC_BASE_COLUMN_NAME, buffer, sizeof(buffer), 0,
                  &numericAttr);
  SQLColAttribute(stmt, 1, SQL_DESC_BASE_COLUMN_NAME, buffer, sizeof(buffer), 0,
                  0);
#endif  // __APPLE__
  SQLColAttribute(stmt, 1, SQL_DESC_BASE_COLUMN_NAME, buffer, 0, &resLen,
                  &numericAttr);
  SQLColAttribute(stmt, 1, SQL_DESC_BASE_COLUMN_NAME, 0, sizeof(buffer),
                  &resLen, &numericAttr);
  SQLColAttribute(stmt, 1, SQL_DESC_BASE_COLUMN_NAME, 0, 0, 0, 0);

  SQLColAttribute(stmt, 1, SQL_DESC_COUNT, buffer, sizeof(buffer), &resLen, 0);
  SQLColAttribute(stmt, 1, SQL_DESC_COUNT, buffer, sizeof(buffer), 0,
                  &numericAttr);
  SQLColAttribute(stmt, 1, SQL_DESC_COUNT, buffer, 0, &resLen, &numericAttr);
  SQLColAttribute(stmt, 1, SQL_DESC_COUNT, 0, sizeof(buffer), &resLen,
                  &numericAttr);
  SQLColAttribute(stmt, 1, SQL_DESC_COUNT, 0, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLDescribeCol) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("select * from data_queries_test_db.TestScalarTypes");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLWCHAR columnName[ODBC_BUFFER_SIZE];
  SQLSMALLINT columnNameLen = 0;
  SQLSMALLINT dataType = 0;
  SQLULEN columnSize = 0;
  SQLSMALLINT decimalDigits = 0;
  SQLSMALLINT nullable = 0;

  // Everything is ok.
  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, &columnNameLen,
                       &dataType, &columnSize, &decimalDigits, &nullable);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Confirm boundary condition.
  SQLSMALLINT reducedNameLen = 4;
  ret = SQLDescribeCol(stmt, 1, columnName, reducedNameLen + 1, &columnNameLen,
                       &dataType, &columnSize, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  BOOST_CHECK_EQUAL(columnNameLen, reducedNameLen);

  ret = SQLDescribeCol(stmt, 1, 0, ODBC_BUFFER_SIZE, &columnNameLen, &dataType,
                       &columnSize, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, columnName, 0, &columnNameLen, &dataType,
                       &columnSize, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);
  BOOST_CHECK_EQUAL(columnNameLen, 0);

  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, 0, &dataType,
                       &columnSize, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, &columnNameLen, 0,
                       &columnSize, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, &columnNameLen,
                       &dataType, 0, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, &columnNameLen,
                       &dataType, &columnSize, 0, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, &columnNameLen,
                       &dataType, &columnSize, &decimalDigits, 0);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, 0, 0, 0, 0, 0, 0, 0);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, -1, 0, 0, 0, 0, 0, 0, 0);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);

  ret = SQLDescribeCol(nullptr, 1, 0, 0, 0, 0, 0, 0, 0);
  BOOST_CHECK_EQUAL(ret, SQL_INVALID_HANDLE);
}

BOOST_AUTO_TEST_CASE(TestSQLRowCount) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("select * from data_queries_test_db.TestScalarTypes");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLLEN rows = 0;

  // Everything is ok.
  ret = SQLRowCount(stmt, &rows);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLRowCount(stmt, nullptr);
}

BOOST_AUTO_TEST_CASE(TestSQLGetStmtAttr) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLINTEGER resLen = 0;

  // Everything is ok.
  SQLRETURN ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, buffer,
                                 sizeof(buffer), &resLen);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, 0, sizeof(buffer), &resLen);
  SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, buffer, 0, &resLen);
  SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, buffer, sizeof(buffer), 0);
  SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLSetStmtAttr) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  SQLULEN val = 1;

  // Everything is ok.
  SQLRETURN ret =
      SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                     reinterpret_cast< SQLPOINTER >(val), sizeof(val));

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, 0, sizeof(val));
  SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                 reinterpret_cast< SQLPOINTER >(val), 0);
  SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLGetDiagField) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.
  ConnectToTS();

  // Should fail.
  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_GUID);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLSMALLINT resLen = 0;

  // Everything is ok
  ret = SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_MESSAGE_TEXT, buffer,
                        sizeof(buffer), &resLen);

  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_MESSAGE_TEXT, 0,
                  sizeof(buffer), &resLen);
  SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_MESSAGE_TEXT, buffer, 0,
                  &resLen);
  SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_MESSAGE_TEXT, buffer,
                  sizeof(buffer), 0);
  SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_MESSAGE_TEXT, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLGetDiagRec) {
  ConnectToTS();

  SQLWCHAR state[ODBC_BUFFER_SIZE];
  SQLINTEGER nativeError = 0;
  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT messageLen = 0;

  // Generating error.
  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_GUID);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  // Everything is ok.
  ret = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message,
                      ODBC_BUFFER_SIZE, &messageLen);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);
  std::vector< SQLWCHAR > actualMessage;
  actualMessage.insert(actualMessage.end(), &message[0],
                       &message[messageLen + 1]);

  // Should return error.
  ret = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message,
                      -1, &messageLen);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  // Should return message length.
  ret = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message, 0,
                      &messageLen);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);

  // Check boundary condition on reduced output buffer.
  SQLSMALLINT reducedMessageLen = 8;
  ret = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message,
                      reducedMessageLen + 1, &messageLen);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  BOOST_REQUIRE_EQUAL(messageLen, reducedMessageLen);
  std::vector< SQLWCHAR > reducedExpectedMessage(
      actualMessage.begin(), actualMessage.begin() + reducedMessageLen);
  reducedExpectedMessage.push_back(0);
  std::vector< SQLWCHAR > reducedMessage;
  reducedMessage.insert(reducedMessage.end(), &message[0],
                        &message[messageLen + 1]);
  BOOST_CHECK_EQUAL_COLLECTIONS(reducedMessage.begin(), reducedMessage.end(),
                                reducedExpectedMessage.begin(),
                                reducedExpectedMessage.end());

  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, 0, &nativeError, message,
                ODBC_BUFFER_SIZE, &messageLen);
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, 0, message, ODBC_BUFFER_SIZE,
                &messageLen);
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, 0,
                ODBC_BUFFER_SIZE, &messageLen);
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message, 0,
                &messageLen);
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message,
                ODBC_BUFFER_SIZE, 0);
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, 0, 0, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLGetData) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("select * from data_queries_test_db.TestScalarTypes");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLFetch(stmt);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLLEN resLen = 0;

  // Everything is ok.
  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, buffer, sizeof(buffer), &resLen);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLFetch(stmt);

  SQLGetData(stmt, 1, SQL_C_WCHAR, 0, sizeof(buffer), &resLen);

  SQLFetch(stmt);

  SQLGetData(stmt, 1, SQL_C_WCHAR, buffer, 0, &resLen);

  SQLFetch(stmt);

  SQLGetData(stmt, 1, SQL_C_WCHAR, buffer, sizeof(buffer), 0);

  SQLFetch(stmt);

  SQLGetData(stmt, 1, SQL_C_WCHAR, 0, 0, 0);

  SQLFetch(stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLGetEnvAttr) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLINTEGER resLen = 0;

  // Everything is ok.
  SQLRETURN ret = SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, buffer,
                                sizeof(buffer), &resLen);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_ENV, env);

  SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, nullptr, sizeof(buffer), &resLen);
  SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, buffer, 0, &resLen);
  SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, buffer, sizeof(buffer), 0);
  SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, nullptr, 0, nullptr);
}

BOOST_AUTO_TEST_CASE(TestFetchScrollLast) {
  CheckFetchScrollUnsupportedOrientation(SQL_FETCH_LAST);
}

BOOST_AUTO_TEST_CASE(TestFetchScrollPrior) {
  CheckFetchScrollUnsupportedOrientation(SQL_FETCH_PRIOR);
}

BOOST_AUTO_TEST_CASE(TestFetchScrollFirst) {
  CheckFetchScrollUnsupportedOrientation(SQL_FETCH_FIRST);
}

BOOST_AUTO_TEST_CASE(TestSQLError) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  ConnectToTS();

  SQLWCHAR state[6] = {0};
  SQLINTEGER nativeCode = 0;
  SQLWCHAR message[ODBC_BUFFER_SIZE] = {0};
  SQLSMALLINT messageLen = 0;

  // Generating error by passing unsupported SQL Type (SQL_INTERVAL_MONTH).
  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_INTERVAL_MONTH);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  // Everything is ok.
  ret = SQLError(env, dbc, stmt, state, &nativeCode, message, ODBC_BUFFER_SIZE,
                 &messageLen);
  BOOST_REQUIRE_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(message[messageLen], 0);

  ret = SQLError(0, dbc, 0, state, &nativeCode, message, ODBC_BUFFER_SIZE,
                 &messageLen);
  if (ret != SQL_SUCCESS && ret != SQL_NO_DATA)
    BOOST_FAIL("Unexpected error");

#ifndef __APPLE__
  // This code could lead to segment fault on macOS BigSur, but not on Ventura
  // This is a limitaion of iODBC driver manger on BigSur
  ret = SQLError(0, 0, stmt, state, &nativeCode, message, ODBC_BUFFER_SIZE,
                 &messageLen);
  if (ret != SQL_SUCCESS && ret != SQL_NO_DATA)
    BOOST_FAIL("Unexpected error");

  SQLError(0, 0, 0, state, &nativeCode, message, ODBC_BUFFER_SIZE, &messageLen);

  SQLError(0, 0, stmt, 0, &nativeCode, message, ODBC_BUFFER_SIZE, &messageLen);

  SQLError(0, 0, stmt, state, 0, message, ODBC_BUFFER_SIZE, &messageLen);

  SQLError(0, 0, stmt, state, &nativeCode, 0, ODBC_BUFFER_SIZE, &messageLen);

  SQLError(0, 0, stmt, state, &nativeCode, message, 0, &messageLen);

  SQLError(0, 0, stmt, state, &nativeCode, message, ODBC_BUFFER_SIZE, 0);

  SQLError(0, 0, stmt, 0, 0, 0, 0, 0);

  SQLError(0, 0, 0, 0, 0, 0, 0, 0);
#endif  // __APPLE__
}

BOOST_AUTO_TEST_CASE(TestSQLCancelForTypeInfo) {
  ConnectToTS();

  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_ALL_TYPES);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  ret = SQLFetch(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  ret = SQLCancel(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  ret = SQLFetch(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
#ifdef __linux__
  BOOST_REQUIRE_EQUAL(
      "HY010: [unixODBC][Driver Manager]Function sequence error",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#elif defined(__APPLE__)
  BOOST_REQUIRE_EQUAL("S1010: [iODBC][Driver Manager]Function sequence error",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#else
  BOOST_REQUIRE_EQUAL("HY010: Query was not executed.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(TestSQLCloseCursorForTypeInfo) {
  ConnectToTS();

  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_ALL_TYPES);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  ret = SQLFetch(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  ret = SQLCloseCursor(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  ret = SQLFetch(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  ret = SQLCloseCursor(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL("24000: [unixODBC][Driver Manager]Invalid cursor state",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#else
  BOOST_REQUIRE_EQUAL("24000: No cursor was open",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(TestSQLAllocFreeDesc) {
  ConnectToTS();

  SQLHANDLE desc;

  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DESC, dbc, &desc);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  // Set the allocated desciptor to statement ARD
  ret = SQLSetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, desc, 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLFreeHandle(SQL_HANDLE_DESC, desc);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  SQLHDESC ard = nullptr;
  SQLINTEGER resLen = 0;

  // Implicit ARD should be returned when the explicit ARD is freed
  ret = SQLGetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, &ard, 0, NULL);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Implicit ARD is not equal to explicit ARD
  BOOST_CHECK(ard != 0 && ard != desc);
}

BOOST_AUTO_TEST_CASE(TestRowArraySizeGreaterThanOne) {
  ConnectToTS();

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, rebuffering_ratio from "
      "data_queries_test_db.TestScalarTypes order by device_id limit 3");

  SQLRETURN ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLULEN val = 3;

  // set row array size to be 3
  ret =
      SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                     reinterpret_cast< SQLPOINTER >(val), sizeof(val));

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // output buffer should be an array of 3
  const int32_t buf_size = 1024;
  SQLWCHAR id[3][buf_size];
  SQLLEN id_len[3]{0};

  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, id, sizeof(id[0]), &id_len[0]);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // output buffer should be an array of 3
  double fieldDouble[3]{0};
  SQLLEN fieldDouble_len[3]{0};
  ret = SQLBindCol(stmt, 2, SQL_C_DOUBLE, &fieldDouble, sizeof(fieldDouble[0]),
                   &fieldDouble_len[0]);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // fetch 3 rows in one fetch
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // verify results
  BOOST_CHECK_EQUAL("00000001", timestream::odbc::utility::SqlWcharToString(
                                    id[0], id_len[0], true));
  BOOST_CHECK_EQUAL("00000002", timestream::odbc::utility::SqlWcharToString(
                                    id[1], id_len[1], true));
  BOOST_CHECK_EQUAL("00000003", timestream::odbc::utility::SqlWcharToString(
                                    id[2], id_len[2], true));

  BOOST_CHECK_EQUAL(0.1, fieldDouble[0]);
  BOOST_CHECK_EQUAL(0.2, fieldDouble[1]);
  BOOST_CHECK_EQUAL(0.3, fieldDouble[2]);
}

BOOST_AUTO_TEST_SUITE_END()
