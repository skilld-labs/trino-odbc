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
 * Modifications Copyright Amazon.com, Inc. or its affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <string>
#include <vector>
#include <array>

#include "odbc_test_suite.h"
#include "test_utils.h"
#include <timestream/odbc/utility.h>

using namespace timestream;
using namespace timestream_test;

using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct ODBCExecutionTestSuiteFixture : public odbc::OdbcTestSuite {
  /**
   * Constructor.
   */
  ODBCExecutionTestSuiteFixture() {
    // No-op
  }

  /**
   * Destructor.
   */
  virtual ~ODBCExecutionTestSuiteFixture() {
    // No-op.
  }
};

BOOST_FIXTURE_TEST_SUITE(ODBCExecutionTestSuite, ODBCExecutionTestSuiteFixture)

// Test unsupported functions
BOOST_AUTO_TEST_CASE(TestSQLEndTran) {
  ConnectToTS();

  SQLRETURN ret =
      SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  ret = SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_COMMIT);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLConnectionDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: SQLEndTran is not supported.",
                      GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
}

BOOST_AUTO_TEST_CASE(TestSQLBrowseConnect) {
  Prepare();

  SQLWCHAR InConnectionString[ODBC_BUFFER_SIZE];
  SQLWCHAR OutConnectionString[ODBC_BUFFER_SIZE];
  SQLSMALLINT reslen;

  SQLRETURN ret = SQLBrowseConnect(dbc, InConnectionString, 0,
                                   InConnectionString, 0, &reslen);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLConnectionDiagnosticError("IM002");
}

#if (ODBCVER >= 0x0380)
BOOST_AUTO_TEST_CASE(TestSQLCancelHandle) {
  ConnectToTS();

  SQLRETURN ret = SQLCancelHandle(SQL_HANDLE_STMT, stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HY010");
  BOOST_REQUIRE_EQUAL("HY010: Query does not exist.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}
#endif

BOOST_AUTO_TEST_CASE(TestSQLTransact) {
  // SQLTransact is deprecated function and will be mapped to SQLEndTran
  // by driver manager.
  ConnectToTS();

  SQLRETURN ret =
      SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  ret = SQLTransact(env, dbc, SQL_COMMIT);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLConnectionDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: SQLEndTran is not supported.",
                      GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
}

BOOST_AUTO_TEST_CASE(TestSQLDescribeParam) {
  ConnectToTS();

  SQLSMALLINT sqlType;
  SQLULEN paramNum;
  SQLSMALLINT scale;
  SQLSMALLINT nullable;
  std::vector< SQLWCHAR > request = MakeSqlBuffer("SELECT 1");

  SQLRETURN ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLDescribeParam(stmt, 1, &sqlType, &paramNum, &scale, &nullable);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: SQLDescribeParam is not supported.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestSQLParamData) {
  ConnectToTS();

  void* val;
  std::vector< SQLWCHAR > request = MakeSqlBuffer("SELECT 1");

  SQLRETURN ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLParamData(stmt, &val);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

// "Function sequence error" from the driver manager (DM) is returned.
// This is because SQL_NEED_DATA (unsupported state) is not returned by
// SQLExecDirect prior to calling SQLParamData.
#ifdef __linux__
  BOOST_REQUIRE_EQUAL(
      "HY010: [unixODBC][Driver Manager]Function sequence error",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#elif defined(__APPLE__)
  BOOST_REQUIRE_EQUAL("S1010: [iODBC][Driver Manager]Function sequence error",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#else
  BOOST_REQUIRE_EQUAL(
      "HY010: [Microsoft][ODBC Driver Manager] Function sequence error",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(TestSQLNumParams) {
  ConnectToTS();

  SQLSMALLINT num = 0;
  std::vector< SQLWCHAR > request = MakeSqlBuffer("SELECT 1");

  SQLRETURN ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLNumParams(stmt, &num);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: SQLNumParams is not supported.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestSQLPutData) {
  ConnectToTS();

  int value = 1;

  SQLRETURN ret = SQLPutData(stmt, &value, 0);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

// "Function sequence error" from the driver manager (DM) is returned.
// This is because the previous function call must be a successful call to
// SQLParamData(), and SQL_NEED_DATA (unsupported state) needs to be returned.
#ifdef __linux__
  BOOST_REQUIRE_EQUAL(
      "HY010: [unixODBC][Driver Manager]Function sequence error",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#elif defined(__APPLE__)
  BOOST_REQUIRE_EQUAL("S1010: [iODBC][Driver Manager]Function sequence error",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#else
  BOOST_REQUIRE_EQUAL(
      "HY010: [Microsoft][ODBC Driver Manager] Function sequence error",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(TestSQLBindParameter) {
  ConnectToTS();

  SQLINTEGER int1;
  SQLLEN len1 = SQL_DATA_AT_EXEC;

  SQLRETURN ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
                                   SQL_INTEGER, 0, 0, &int1, 0, &len1);

  BOOST_REQUIRE(!SQL_SUCCEEDED(ret));

#ifdef __APPLE__
  // On macOS BigSur calling SQLBindParameter returns SQL_INVALID_HANDLE
  // even SQLGetFunctions returns true for SQLBindParameter. This behavior
  // is determined by iODBC driver manager.
  if (ret != SQL_ERROR)
    BOOST_REQUIRE_EQUAL(ret, SQL_INVALID_HANDLE);
  else {
    // On Ventura (macOS 13), iODBC calls SQLBindParameter normally and returns
    // SQL_ERROR as expected.
    CheckSQLStatementDiagnosticError("HYC00");
    BOOST_REQUIRE_EQUAL("HYC00: SQLBindParameter is not supported.",
                        GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
#else
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: SQLBindParameter is not supported.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif  //__APPLE__
}

BOOST_AUTO_TEST_CASE(TestSQLSetParam) {
  // SQLSetParam is deprecated function and will be mapped to SQLBindParameter
  // by driver manager.
  ConnectToTS();

  SQLINTEGER int1;
  SQLLEN len = 0;

  SQLRETURN ret =
      SQLSetParam(stmt, 1, SQL_PARAM_INPUT, SQL_INTEGER, len, 100, &int1, &len);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
}

BOOST_AUTO_TEST_CASE(TestSQLBulkOperations) {
  ConnectToTS();

  SQLRETURN ret = SQLBulkOperations(stmt, SQL_ADD);

  BOOST_REQUIRE(!SQL_SUCCEEDED(ret));

#ifdef __APPLE__
  // SQLBulkOperations is not a supported function, this is indicated in
  // SQLGetInfo. iODBC returns SQL_INVALID_HANDLE when SQLBulkOperations is
  // called after a connection is made. This behavior is outside of driver's
  // control.
  BOOST_REQUIRE_EQUAL(ret, SQL_INVALID_HANDLE);
#else
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
#endif  // __APPLE__
}

BOOST_AUTO_TEST_CASE(TestSQLSetPos) {
  ConnectToTS();

  std::vector< SQLWCHAR > request = MakeSqlBuffer("SELECT 1");

  SQLRETURN ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLSetPos(stmt, 0, SQL_POSITION, SQL_LOCK_NO_CHANGE);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: SQLSetPos is not supported.",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestSQLSetDescRec) {
  ConnectToTS();

  SQLHDESC desc;
  SQLINTEGER data = 10;
  SQLRETURN ret = SQLGetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, &desc, 0, NULL);

  ret = SQLSetDescRec(desc, 2, SQL_INTEGER, 0, 0, 0, 0, (SQLPOINTER)&data, NULL,
                      NULL);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLDiagnosticError(SQL_HANDLE_DESC, desc, "HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: SQLSetDescRec is not supported.",
                      GetOdbcErrorMessage(SQL_HANDLE_DESC, desc));
}

BOOST_AUTO_TEST_CASE(TestSQLGetDescRec) {
  ConnectToTS();

  SQLHDESC desc;
  SQLINTEGER data = 10;
  SQLRETURN ret = SQLGetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, &desc, 0, NULL);

  std::vector< SQLWCHAR > column = MakeSqlBuffer("Region");
  ret = SQLGetDescRec(desc, 1, column.data(), 10, NULL, NULL, NULL, NULL, NULL,
                      NULL, NULL);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLDiagnosticError(SQL_HANDLE_DESC, desc, "HYC00");
  BOOST_REQUIRE_EQUAL("HYC00: SQLGetDescRec is not supported.",
                      GetOdbcErrorMessage(SQL_HANDLE_DESC, desc));
}

BOOST_AUTO_TEST_CASE(TestSetGetCursorName) {
  ConnectToTS();
  std::vector< SQLWCHAR > cursorName = MakeSqlBuffer("cursor1");

  SQLRETURN ret = SQLSetCursorName(stmt, cursorName.data(), 7);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  SQLWCHAR cursorNameRes[20];
  SQLSMALLINT resLen;

  ret = SQLGetCursorName(stmt, cursorNameRes, 7, &resLen);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(
      timestream::odbc::utility::SqlWcharToString(cursorName.data()),
      timestream::odbc::utility::SqlWcharToString(cursorNameRes));
  BOOST_CHECK_EQUAL(resLen, 7);
}

BOOST_AUTO_TEST_CASE(TestSQLGetCursorNameTruncated) {
  ConnectToTS();
  std::vector< SQLWCHAR > cursorName = MakeSqlBuffer("cursor1");

  SQLRETURN ret = SQLSetCursorName(stmt, cursorName.data(), 7);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  SQLWCHAR cursorNameRes[20];
  SQLSMALLINT resLen;

  // cursor name is truncated when call SQLGetCursorName
  ret = SQLGetCursorName(stmt, cursorNameRes, 6, &resLen);

  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  CheckSQLStatementDiagnosticError("01000");
  BOOST_CHECK_EQUAL("01000: Buffer is too small for the cursor name.",
                    GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  BOOST_CHECK_EQUAL(resLen, 6);
}

BOOST_AUTO_TEST_CASE(TestSQLSetCursorNameTruncated) {
  ConnectToTS();
  std::vector< SQLWCHAR > cursorName = MakeSqlBuffer("cursor1");

  // cursor name is truncated when call SQLSetCursorName
  SQLRETURN ret = SQLSetCursorName(stmt, cursorName.data(), 5);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  SQLWCHAR cursorNameRes[20];
  SQLSMALLINT resLen;

  ret = SQLGetCursorName(stmt, cursorNameRes, 10, &resLen);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(timestream::odbc::utility::SqlWcharToString(cursorNameRes),
                    "curso");
  BOOST_CHECK_EQUAL(resLen, 5);
}

BOOST_AUTO_TEST_CASE(TestSQLSetCursorNameMultipleTimes) {
  ConnectToTS();
  std::vector< SQLWCHAR > cursorName = MakeSqlBuffer("cursor1");

  SQLRETURN ret = SQLSetCursorName(stmt, cursorName.data(), 10);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // duplicated cursor name could not be set
  ret = SQLSetCursorName(stmt, cursorName.data(), 10);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("3C000");
  BOOST_CHECK_EQUAL("3C000: Cursor name \"cursor1\" has already been used.",
                    GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  std::vector< SQLWCHAR > cursorName2 = MakeSqlBuffer("cursor2");
  ret = SQLSetCursorName(stmt, cursorName2.data(), 10);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  SQLWCHAR cursorNameRes[20];
  SQLSMALLINT resLen;

  ret = SQLGetCursorName(stmt, cursorNameRes, 20, &resLen);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(
      timestream::odbc::utility::SqlWcharToString(cursorName2.data()),
      timestream::odbc::utility::SqlWcharToString(cursorNameRes));
  BOOST_CHECK_EQUAL(resLen, 7);
}

BOOST_AUTO_TEST_CASE(TestSQLSetCursorNameErrorCase) {
  ConnectToTS();
  std::vector< SQLWCHAR > cursorName = MakeSqlBuffer("veryverylongcursorname");

  // cursor name should not exceed 18 characters
  SQLRETURN ret = SQLSetCursorName(stmt, cursorName.data(), 20);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("3C000");
  BOOST_CHECK_EQUAL(
      "3C000: The number of characters in cursor name (20) exceeds the maximum "
      "allowed "
      "number (18)",
      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // cursor name should not start with SQL_CUR
  std::vector< SQLWCHAR > cursorName2 = MakeSqlBuffer("SQL_CUR1");
  ret = SQLSetCursorName(stmt, cursorName2.data(), 10);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);
  CheckSQLStatementDiagnosticError("34000");
  BOOST_CHECK_EQUAL("34000: Cursor name should not start with SQL_CUR",
                    GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

// SQLGetFunctions is from driver manager for Windows and Linux
// For macOS SQLGetFunctions is using our own implementation due to
// iODBC driver mangaer does not provide SQLGetFunctions on BigSur
BOOST_AUTO_TEST_CASE(TestSQLGetFunctions) {
  ConnectToTS();
  SQLUSMALLINT fExists[SQL_API_ODBC3_ALL_FUNCTIONS_SIZE];
  std::array< int, 58 > supportedFuncArray = {SQL_API_SQLALLOCHANDLE,
                                              SQL_API_SQLGETDESCFIELD,
                                              SQL_API_SQLBINDCOL,
                                              SQL_API_SQLGETDESCREC,
                                              SQL_API_SQLCANCEL,
                                              SQL_API_SQLGETDIAGFIELD,
                                              SQL_API_SQLCLOSECURSOR,
                                              SQL_API_SQLGETDIAGREC,
                                              SQL_API_SQLCOLATTRIBUTE,
                                              SQL_API_SQLGETENVATTR,
                                              SQL_API_SQLCONNECT,
                                              SQL_API_SQLGETFUNCTIONS,
                                              SQL_API_SQLCOPYDESC,
                                              SQL_API_SQLGETINFO,
                                              SQL_API_SQLDATASOURCES,
                                              SQL_API_SQLGETSTMTATTR,
                                              SQL_API_SQLDESCRIBECOL,
                                              SQL_API_SQLGETTYPEINFO,
                                              SQL_API_SQLDISCONNECT,
                                              SQL_API_SQLNUMRESULTCOLS,
                                              SQL_API_SQLDRIVERS,
                                              SQL_API_SQLPARAMDATA,
                                              SQL_API_SQLENDTRAN,
                                              SQL_API_SQLPREPARE,
                                              SQL_API_SQLEXECDIRECT,
                                              SQL_API_SQLPUTDATA,
                                              SQL_API_SQLEXECUTE,
                                              SQL_API_SQLROWCOUNT,
                                              SQL_API_SQLFETCH,
                                              SQL_API_SQLSETCONNECTATTR,
                                              SQL_API_SQLFETCHSCROLL,
                                              SQL_API_SQLSETCURSORNAME,
                                              SQL_API_SQLFREEHANDLE,
                                              SQL_API_SQLSETDESCFIELD,
                                              SQL_API_SQLFREESTMT,
                                              SQL_API_SQLSETDESCREC,
                                              SQL_API_SQLGETCONNECTATTR,
                                              SQL_API_SQLSETENVATTR,
                                              SQL_API_SQLGETCURSORNAME,
                                              SQL_API_SQLSETSTMTATTR,
                                              SQL_API_SQLGETDATA,
                                              SQL_API_SQLCOLUMNS,
                                              SQL_API_SQLSTATISTICS,
                                              SQL_API_SQLSPECIALCOLUMNS,
                                              SQL_API_SQLTABLES,
                                              SQL_API_SQLNATIVESQL,
                                              SQL_API_SQLBROWSECONNECT,
                                              SQL_API_SQLNUMPARAMS,
                                              SQL_API_SQLPRIMARYKEYS,
                                              SQL_API_SQLCOLUMNPRIVILEGES,
                                              SQL_API_SQLPROCEDURECOLUMNS,
                                              SQL_API_SQLDESCRIBEPARAM,
                                              SQL_API_SQLPROCEDURES,
                                              SQL_API_SQLDRIVERCONNECT,
                                              SQL_API_SQLSETPOS,
                                              SQL_API_SQLFOREIGNKEYS,
                                              SQL_API_SQLTABLEPRIVILEGES,
                                              SQL_API_SQLMORERESULTS};

  RETCODE retcode = SQLGetFunctions(dbc, SQL_API_ODBC3_ALL_FUNCTIONS, fExists);
  if (retcode != SQL_SUCCESS) {
    BOOST_FAIL("Failed for SQL_API_ODBC3_ALL_FUNCTIONS, error is "
               + GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
  }

  // verify result for SQL_API_ODBC3_ALL_FUNCTIONS
  for (int i = 0; i < 58; i++) {
    BOOST_CHECK_EQUAL(SQL_FUNC_EXISTS(fExists, supportedFuncArray[i]), true);
  }
  // SQL_API_SQLBULKOPERATIONS result is different on different platforms
#if defined(_WIN32) || defined(__APPLE__)
  BOOST_CHECK_EQUAL(SQL_FUNC_EXISTS(fExists, SQL_API_SQLBULKOPERATIONS), true);
#else
  BOOST_CHECK_EQUAL(SQL_FUNC_EXISTS(fExists, SQL_API_SQLBULKOPERATIONS), false);
#endif

  // test for each function Id
  SQLUSMALLINT exists;
  for (int i = 0; i < 58; i++) {
    retcode = SQLGetFunctions(dbc, supportedFuncArray[i], &exists);
    if (retcode != SQL_SUCCESS) {
      BOOST_FAIL("Failed for " + timestream::odbc::common::LongToString(i)
                 + ", error is " + GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
    }
    BOOST_CHECK_EQUAL(exists, true);
  }

  // SQL_API_SQLBULKOPERATIONS result is different on different platforms
  retcode = SQLGetFunctions(dbc, SQL_API_SQLBULKOPERATIONS, &exists);
  if (retcode != SQL_SUCCESS) {
    BOOST_FAIL("Failed for SQL_API_SQLBULKOPERATIONS, error is "
               + GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
  }
#if defined(_WIN32) || defined(__APPLE__)
  BOOST_CHECK_EQUAL(exists, true);
#else
  BOOST_CHECK_EQUAL(exists, false);
#endif
}

// For Windows and Linux, SQLGetFunctions implementation from driver manager is
// used. For macOS, the driver's implementation of SQLGetFunctions is used due
// to iODBC driver mangaer not providing SQLGetFunctions on BigSur
BOOST_AUTO_TEST_CASE(TestSQLGetFunctionsForODBC2) {
  ConnectToTS();
  SQLUSMALLINT fExists[SQL_API_ALL_FUNCTIONS_SIZE];
  std::array< int, 54 > supportedFuncArray = {
      SQL_API_SQLALLOCCONNECT,     SQL_API_SQLALLOCENV,
      SQL_API_SQLALLOCSTMT,        SQL_API_SQLBINDCOL,
      SQL_API_SQLBINDPARAMETER,    SQL_API_SQLBROWSECONNECT,
      SQL_API_SQLCANCEL,           SQL_API_SQLCOLATTRIBUTES,
      SQL_API_SQLCOLUMNPRIVILEGES, SQL_API_SQLCOLUMNS,
      SQL_API_SQLCONNECT,          SQL_API_SQLDATASOURCES,
      SQL_API_SQLDESCRIBECOL,      SQL_API_SQLDESCRIBEPARAM,
      SQL_API_SQLDISCONNECT,       SQL_API_SQLDRIVERCONNECT,
      SQL_API_SQLDRIVERS,          SQL_API_SQLERROR,
      SQL_API_SQLEXECDIRECT,       SQL_API_SQLEXECUTE,
      SQL_API_SQLEXTENDEDFETCH,    SQL_API_SQLFETCH,
      SQL_API_SQLFOREIGNKEYS,      SQL_API_SQLFREECONNECT,
      SQL_API_SQLFREEENV,          SQL_API_SQLFREESTMT,
      SQL_API_SQLGETCURSORNAME,    SQL_API_SQLGETDATA,
      SQL_API_SQLGETFUNCTIONS,     SQL_API_SQLGETINFO,
      SQL_API_SQLGETSTMTOPTION,    SQL_API_SQLGETTYPEINFO,
      SQL_API_SQLMORERESULTS,      SQL_API_SQLNATIVESQL,
      SQL_API_SQLNUMPARAMS,        SQL_API_SQLNUMRESULTCOLS,
      SQL_API_SQLPARAMDATA,        SQL_API_SQLPARAMOPTIONS,
      SQL_API_SQLPREPARE,          SQL_API_SQLPRIMARYKEYS,
      SQL_API_SQLPROCEDURECOLUMNS, SQL_API_SQLPROCEDURES,
      SQL_API_SQLPUTDATA,          SQL_API_SQLROWCOUNT,
      SQL_API_SQLSETCURSORNAME,    SQL_API_SQLSETPARAM,
      SQL_API_SQLSETPOS,           SQL_API_SQLSETSCROLLOPTIONS,
      SQL_API_SQLSETSTMTOPTION,    SQL_API_SQLSPECIALCOLUMNS,
      SQL_API_SQLSTATISTICS,       SQL_API_SQLTABLEPRIVILEGES,
      SQL_API_SQLTABLES,           SQL_API_SQLTRANSACT};

  RETCODE retcode = SQLGetFunctions(dbc, SQL_API_ALL_FUNCTIONS, fExists);
  if (retcode != SQL_SUCCESS) {
    BOOST_FAIL("Failed for SQL_API_ALL_FUNCTIONS, error is "
               + GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
  }

  // verify result for SQL_API_ALL_FUNCTIONS
  for (int i = 0; i < 54; i++) {
    BOOST_CHECK_EQUAL(fExists[supportedFuncArray[i]], true);
  }

  BOOST_CHECK_EQUAL(fExists[SQL_API_SQLGETCONNECTOPTION], true);
  BOOST_CHECK_EQUAL(fExists[SQL_API_SQLSETCONNECTOPTION], true);

  // test for each function Id
  SQLUSMALLINT exists;
  for (int i = 0; i < 54; i++) {
    retcode = SQLGetFunctions(dbc, supportedFuncArray[i], &exists);
    if (retcode != SQL_SUCCESS) {
      BOOST_FAIL("Failed for " + timestream::odbc::common::LongToString(i)
                 + ", error is " + GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
    }
    BOOST_CHECK_EQUAL(exists, true);
  }

  // SQL_API_SQLGETCONNECTOPTION result is different on different platforms
  retcode = SQLGetFunctions(dbc, SQL_API_SQLGETCONNECTOPTION, &exists);
  if (retcode != SQL_SUCCESS) {
    BOOST_FAIL("Failed for SQL_API_SQLGETCONNECTOPTION, error is "
               + GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
  }
  BOOST_CHECK_EQUAL(exists, true);

  // SQL_API_SQLSETCONNECTOPTION result is different on different platforms
  retcode = SQLGetFunctions(dbc, SQL_API_SQLSETCONNECTOPTION, &exists);
  if (retcode != SQL_SUCCESS) {
    BOOST_FAIL("Failed for SQL_API_SQLSETCONNECTOPTION, error is "
               + GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
  }
  BOOST_CHECK_EQUAL(exists, true);
}

BOOST_AUTO_TEST_SUITE_END()
