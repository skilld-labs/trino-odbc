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
#include "trino/odbc/utility.h"
#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <string>
#include <vector>

#include "odbc_test_suite.h"
#include "test_utils.h"

#include "trino/odbc/log.h"
#include "trino/odbc/config/configuration.h"

using namespace trino;
using namespace trino_test;

using namespace boost::unit_test;

#define CHECK_DESC_GET_FIELD_FAILURE(fieldId)                                  \
  {                                                                            \
    SQLSMALLINT tmp;                                                           \
    ret = SQLGetDescField(ard, 1, fieldId, &tmp, 0, NULL);                     \
    BOOST_CHECK_EQUAL(SQL_ERROR, ret);                                         \
    OdbcClientError error = GetOdbcError(SQL_HANDLE_DESC, ard);                \
    BOOST_CHECK_EQUAL(error.sqlstate, "HY000");                                \
    BOOST_CHECK(error.message.find(                                            \
                    "Current descriptor type ARD is not allowed to get field") \
                != std::string::npos);                                         \
  }

#define CHECK_DESC_SET_FIELD_FAILURE(fieldId, expectedErrMsg)             \
  {                                                                       \
    SQLSMALLINT tmp = 0;                                                  \
    ret = SQLSetDescField(ard, 1, fieldId, (SQLPOINTER)tmp, 0);           \
    BOOST_CHECK_EQUAL(SQL_ERROR, ret);                                    \
    OdbcClientError error = GetOdbcError(SQL_HANDLE_DESC, ard);           \
    BOOST_CHECK_EQUAL(error.sqlstate, "HY091");                           \
    BOOST_CHECK(error.message.find(expectedErrMsg) != std::string::npos); \
  }

/**
 * Test setup fixture.
 */
struct DescriptorTestSuiteFixture : public trino::odbc::OdbcTestSuite {
  /**
   * Constructor.
   */
  DescriptorTestSuiteFixture() {
    ConnectToTS();
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DESC, dbc, &ard);
    BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

    ret = SQLSetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, ard, 0);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  }

  /**
   * Destructor.
   */
  virtual ~DescriptorTestSuiteFixture() {
    SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_DESC, ard);
    BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);
  }

  SQLHANDLE ard;
};

BOOST_FIXTURE_TEST_SUITE(DescriptorTestSuite, DescriptorTestSuiteFixture)

// Test ARD fields which is set when SQLBindCol is executed
BOOST_AUTO_TEST_CASE(TestDescriptorGetFromBindCol) {
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time, flag, rebuffering_ratio, video_startup_time, "
      "date(TIMESTAMP '2022-07-07 17:44:43.771000000'), current_time, interval "
      "'4' year + interval '2' month,"
      "interval '6' day + interval '4' hour, current_timestamp from "
      "data_queries_test_db.TestScalarTypes order by device_id limit 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLSMALLINT si;
  ret = SQLGetDescField(ard, 1, SQL_DESC_CONCISE_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, SQL_WCHAR);

  ret = SQLGetDescField(ard, 1, SQL_DESC_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, SQL_WCHAR);

  SQLPOINTER dataPtr;
  ret = SQLGetDescField(ard, 1, SQL_DESC_DATA_PTR, &dataPtr, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(dataPtr, static_cast< SQLPOINTER >(id));

  SQLULEN len;
  ret = SQLGetDescField(ard, 1, SQL_DESC_LENGTH, &len, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(len, sizeof(id));

  ret = SQLGetDescField(ard, 1, SQL_DESC_PRECISION, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, 0);

  ret = SQLGetDescField(ard, 1, SQL_DESC_SCALE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, 0);

  ret = SQLGetDescField(ard, 1, SQL_DESC_OCTET_LENGTH, &len, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(len, sizeof(id));

  SQLLEN* lenPtr;
  ret = SQLGetDescField(ard, 1, SQL_DESC_OCTET_LENGTH_PTR, &lenPtr, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(lenPtr, &id_len);

  ret = SQLGetDescField(ard, 1, SQL_DESC_INDICATOR_PTR, &lenPtr, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(lenPtr, &id_len);
}

// Test SQLSetDescField using char type
BOOST_AUTO_TEST_CASE(TestDescriptorSetCharType) {
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time, flag, rebuffering_ratio, video_startup_time, "
      "date(TIMESTAMP '2022-07-07 17:44:43.771000000'), current_time, interval "
      "'4' year + interval '2' month,"
      "interval '6' day + interval '4' hour, current_timestamp from "
      "data_queries_test_db.TestScalarTypes order by device_id limit 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLSMALLINT siv;
  siv = SQL_CHAR;
  // set SQL_DESC_CONCISE_TYPE
  ret = SQLSetDescField(ard, 1, SQL_DESC_CONCISE_TYPE, (SQLPOINTER)siv, 500);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLSMALLINT si;
  ret = SQLGetDescField(ard, 1, SQL_DESC_CONCISE_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, siv);

  ret = SQLGetDescField(ard, 1, SQL_DESC_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, siv);

  const int32_t buf_size2 = 500;
  SQLCHAR id2[buf_size2]{};
  // set SQL_DESC_DATA_PTR
  ret = SQLSetDescField(ard, 1, SQL_DESC_DATA_PTR, id2, buf_size2);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLPOINTER dataPtr;
  ret = SQLGetDescField(ard, 1, SQL_DESC_DATA_PTR, &dataPtr, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(dataPtr, static_cast< SQLPOINTER >(id2));

  SQLULEN len;
  ret = SQLGetDescField(ard, 1, SQL_DESC_LENGTH, &len, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(len, 500);

  ret = SQLGetDescField(ard, 1, SQL_DESC_OCTET_LENGTH, &len, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(len, 500);

  SQLLEN ind = 50;
  // set SQL_DESC_INDICATOR_PTR
  ret = SQLSetDescField(ard, 1, SQL_DESC_INDICATOR_PTR, &ind, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLLEN* lenPtr;
  ret = SQLGetDescField(ard, 1, SQL_DESC_INDICATOR_PTR, &lenPtr, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(lenPtr, &ind);

  // set SQL_DESC_OCTET_LENGTH_PTR
  ret =
      SQLSetDescField(ard, 1, SQL_DESC_OCTET_LENGTH_PTR, &ind, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetDescField(ard, 1, SQL_DESC_OCTET_LENGTH_PTR, &lenPtr, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(lenPtr, &ind);

  // set SQL_DESC_OCTET_LENGTH
  ret = SQLSetDescField(ard, 1, SQL_DESC_OCTET_LENGTH, (SQLPOINTER)ind,
                        SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetDescField(ard, 1, SQL_DESC_OCTET_LENGTH, &len, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(len, ind);

  // set SQL_DESC_LENGTH
  ret =
      SQLSetDescField(ard, 1, SQL_DESC_LENGTH, (SQLPOINTER)ind, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetDescField(ard, 1, SQL_DESC_LENGTH, &len, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(len, ind);

  // set SQL_DESC_TYPE
  ret = SQLSetDescField(ard, 1, SQL_DESC_TYPE, (SQLPOINTER)siv, 500);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetDescField(ard, 1, SQL_DESC_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, siv);

  siv = SQL_CODE_DATE;
}

// Test SQLSetDescField using timestamp type
BOOST_AUTO_TEST_CASE(TestDescriptorSetTimestampType) {
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time, flag, rebuffering_ratio, video_startup_time, "
      "date(TIMESTAMP '2022-07-07 17:44:43.771000000'), current_time, interval "
      "'4' year + interval '2' month,"
      "interval '6' day + interval '4' hour, current_timestamp from "
      "data_queries_test_db.TestScalarTypes order by device_id limit 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQL_TIMESTAMP_STRUCT timestamp;
  SQLLEN timestamp_len = 0;
  ret = SQLBindCol(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLSMALLINT siv;
  siv = SQL_TYPE_TIME;
  // set SQL_DESC_CONCISE_TYPE
  ret = SQLSetDescField(ard, 1, SQL_DESC_CONCISE_TYPE, (SQLPOINTER)siv,
                        SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLSMALLINT si;
  ret = SQLGetDescField(ard, 1, SQL_DESC_CONCISE_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, siv);

  ret = SQLGetDescField(ard, 1, SQL_DESC_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, SQL_DATETIME);

  ret = SQLGetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_CODE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, SQL_CODE_TIME);

  siv = SQL_DATETIME;
  // set SQL_DESC_TYPE
  ret = SQLSetDescField(ard, 1, SQL_DESC_TYPE, (SQLPOINTER)siv, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetDescField(ard, 1, SQL_DESC_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, siv);

  siv = SQL_CODE_DATE;
  // set SQL_DESC_DATETIME_INTERVAL_CODE
  ret = SQLSetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_CODE,
                        (SQLPOINTER)siv, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_CODE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, siv);

  // set SQL_DESC_DATETIME_INTERVAL_PRECISION
  siv = 5;
  ret = SQLSetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_PRECISION,
                        (SQLPOINTER)siv, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  OdbcClientError error = GetOdbcError(SQL_HANDLE_DESC, ard);
  BOOST_CHECK_EQUAL(error.sqlstate, "HY000");

  BOOST_CHECK_EQUAL(
      error.message,
      "Interval precision could only be set when SQL_DESC_TYPE is "
      "set to SQL_INTERVAL");
}

// Test SQLSetDescField using interval type
BOOST_AUTO_TEST_CASE(TestDescriptorSetIntervalType) {
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time, flag, rebuffering_ratio, video_startup_time, "
      "date(TIMESTAMP '2022-07-07 17:44:43.771000000'), current_time, interval "
      "'4' year + interval '2' month,"
      "interval '6' day + interval '4' hour, current_timestamp from "
      "data_queries_test_db.TestScalarTypes order by device_id limit 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQL_INTERVAL_STRUCT yearMonth;
  SQLLEN yearMonth_len = 0;
  ret = SQLBindCol(stmt, 8, SQL_C_INTERVAL_YEAR_TO_MONTH, &yearMonth,
                   sizeof(yearMonth), &yearMonth_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLSMALLINT siv;

  // set SQL_DESC_CONCISE_TYPE
  siv = SQL_INTERVAL_DAY_TO_SECOND;
  ret = SQLSetDescField(ard, 1, SQL_DESC_CONCISE_TYPE, (SQLPOINTER)siv,
                        SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLSMALLINT si;
  ret = SQLGetDescField(ard, 1, SQL_DESC_CONCISE_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, siv);

  ret = SQLGetDescField(ard, 1, SQL_DESC_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, SQL_INTERVAL);

  ret = SQLGetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_CODE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, SQL_CODE_DAY_TO_SECOND);

  // set SQL_DESC_TYPE
  siv = SQL_INTERVAL;
  ret = SQLSetDescField(ard, 1, SQL_DESC_TYPE, (SQLPOINTER)siv, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetDescField(ard, 1, SQL_DESC_TYPE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, siv);

  // set SQL_DESC_DATETIME_INTERVAL_CODE
  siv = SQL_CODE_YEAR_TO_MONTH;
  ret = SQLSetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_CODE,
                        (SQLPOINTER)siv, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_CODE, &si, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(si, siv);

  // set SQL_DESC_DATETIME_INTERVAL_PRECISION
  SQLINTEGER iv = 5;
  ret = SQLSetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_PRECISION,
                        (SQLPOINTER)iv, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLINTEGER iv2;
  ret = SQLGetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_PRECISION, &iv2, 0,
                        NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(iv2, iv);
}

// Test SQLSetDescField not supported case
BOOST_AUTO_TEST_CASE(TestDescriptorSetRainyCase) {
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time, flag, rebuffering_ratio, video_startup_time, "
      "date(TIMESTAMP '2022-07-07 17:44:43.771000000'), current_time, interval "
      "'4' year + interval '2' month,"
      "interval '6' day + interval '4' hour, current_timestamp from "
      "data_queries_test_db.TestScalarTypes order by device_id limit 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  const int32_t buf_size2 = 500;
  SQLCHAR id2[buf_size2]{};
  // set SQL_DESC_DATA_PTR
  ret = SQLSetDescField(ard, 1, SQL_DESC_DATA_PTR, id2, -20);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  OdbcClientError error = GetOdbcError(SQL_HANDLE_DESC, ard);
  BOOST_CHECK_EQUAL(error.sqlstate, "HY000");
  BOOST_CHECK_EQUAL(error.message, "Invalid buffer length -20");

  SQLSMALLINT siv;
  siv = SQL_INTERVAL_DAY_TO_HOUR;
  // set SQL_DESC_CONCISE_TYPE to invalid type
  ret = SQLSetDescField(ard, 1, SQL_DESC_CONCISE_TYPE, (SQLPOINTER)siv, 500);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  error = GetOdbcError(SQL_HANDLE_DESC, ard);
  BOOST_CHECK_EQUAL(error.sqlstate, "HY000");
  BOOST_CHECK_EQUAL(error.message,
                    "Invalid concise type SQL_INTERVAL_DAY_TO_HOUR");

  // set SQL_DESC_DATETIME_INTERVAL_CODE to invalid interval code
  siv = SQL_CODE_DATE;
  ret = SQLSetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_CODE,
                        (SQLPOINTER)siv, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  error = GetOdbcError(SQL_HANDLE_DESC, ard);
  BOOST_CHECK_EQUAL(error.sqlstate, "HY000");
  BOOST_CHECK_EQUAL(error.message,
                    "Invalid interval code SQL_CODE_DATE for type SQL_WCHAR");

  // set SQL_DESC_DATETIME_INTERVAL_PRECISION for non internal type
  siv = 5;
  ret = SQLSetDescField(ard, 1, SQL_DESC_DATETIME_INTERVAL_PRECISION,
                        (SQLPOINTER)siv, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  error = GetOdbcError(SQL_HANDLE_DESC, ard);
  BOOST_CHECK_EQUAL(error.sqlstate, "HY000");
  BOOST_CHECK_EQUAL(error.message,
                    "Interval precision could only be set when SQL_DESC_TYPE "
                    "is set to SQL_INTERVAL");

  SQLBIGINT fieldLong;
  SQLLEN fieldLong_len = 0;
  ret = SQLBindCol(stmt, 5, SQL_C_SBIGINT, &fieldLong, sizeof(fieldLong),
                   &fieldLong_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  siv = 5;
  // set SQL_DESC_OCTET_LENGTH for fixed length type
  ret = SQLSetDescField(ard, 5, SQL_DESC_OCTET_LENGTH, (SQLPOINTER)siv,
                        SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  error = GetOdbcError(SQL_HANDLE_DESC, ard);
  BOOST_CHECK_EQUAL(error.sqlstate, "HY000");
  BOOST_CHECK_EQUAL(
      error.message,
      "SQL_DESC_LENGTH could not be set for fixed length type -25");

  // set SQL_DESC_LENGTH for fixed length type
  ret =
      SQLSetDescField(ard, 5, SQL_DESC_LENGTH, (SQLPOINTER)siv, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  error = GetOdbcError(SQL_HANDLE_DESC, ard);
  BOOST_CHECK_EQUAL(error.sqlstate, "HY000");
  BOOST_CHECK_EQUAL(
      error.message,
      "SQL_DESC_LENGTH could not be set for fixed length type -25");

  siv = SQL_TYPE_TIMESTAMP;
  // set SQL_DESC_TYPE to not supported type
  ret = SQLSetDescField(ard, 5, SQL_DESC_TYPE, (SQLPOINTER)siv, SQL_IS_POINTER);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  error = GetOdbcError(SQL_HANDLE_DESC, ard);
  BOOST_CHECK_EQUAL(error.sqlstate, "HY000");
  BOOST_CHECK_EQUAL(error.message, "Invalid type SQL_TYPE_TIMESTAMP");
}

// Test SQLSetDescField not supported fieldIds
BOOST_AUTO_TEST_CASE(TestDescriptorSetRainyCase2) {
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time, flag, rebuffering_ratio, video_startup_time, "
      "date(TIMESTAMP '2022-07-07 17:44:43.771000000'), current_time, interval "
      "'4' year + interval '2' month,"
      "interval '6' day + interval '4' hour, current_timestamp from "
      "data_queries_test_db.TestScalarTypes order by device_id limit 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

#ifdef _WIN32
  // error from Windows driver manager
  CHECK_DESC_SET_FIELD_FAILURE(SQL_DESC_ALLOC_TYPE,
                               "Descriptor type out of range");
#else
  CHECK_DESC_SET_FIELD_FAILURE(SQL_DESC_ALLOC_TYPE,
                               "Invalid descriptor field id");
#endif
  CHECK_DESC_SET_FIELD_FAILURE(SQL_DESC_ARRAY_SIZE,
                               "Invalid descriptor field id");
  CHECK_DESC_SET_FIELD_FAILURE(SQL_DESC_ARRAY_STATUS_PTR,
                               "Invalid descriptor field id");
  CHECK_DESC_SET_FIELD_FAILURE(SQL_DESC_BIND_OFFSET_PTR,
                               "Invalid descriptor field id");
  CHECK_DESC_SET_FIELD_FAILURE(SQL_DESC_BIND_TYPE,
                               "Invalid descriptor field id");
  CHECK_DESC_SET_FIELD_FAILURE(SQL_DESC_COUNT, "Invalid descriptor field id");
  CHECK_DESC_SET_FIELD_FAILURE(SQL_DESC_ROWS_PROCESSED_PTR,
                               "Invalid descriptor field id");
}

// Test SQLGetDescField not supported fieldIds for ARD
BOOST_AUTO_TEST_CASE(TestDescriptorGetRainyCase) {
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time, flag, rebuffering_ratio, video_startup_time, "
      "date(TIMESTAMP '2022-07-07 17:44:43.771000000'), current_time, interval "
      "'4' year + interval '2' month,"
      "interval '6' day + interval '4' hour, current_timestamp from "
      "data_queries_test_db.TestScalarTypes order by device_id limit 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_ROWS_PROCESSED_PTR);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_AUTO_UNIQUE_VALUE);

  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_BASE_COLUMN_NAME);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_BASE_TABLE_NAME);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_CASE_SENSITIVE);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_CATALOG_NAME);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_DISPLAY_SIZE);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_FIXED_PREC_SCALE);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_LABEL);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_LITERAL_PREFIX);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_LITERAL_SUFFIX);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_LOCAL_TYPE_NAME);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_NAME);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_NULLABLE);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_PARAMETER_TYPE);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_ROWVER);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_SCHEMA_NAME);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_SEARCHABLE);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_TABLE_NAME);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_TYPE_NAME);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_UNNAMED);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_UNSIGNED);
  CHECK_DESC_GET_FIELD_FAILURE(SQL_DESC_UPDATABLE);
}

// Test SQLCopyDesc for ARD
BOOST_AUTO_TEST_CASE(TestCopyDescriptor) {
  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  SQLRETURN ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLHANDLE dst;
  ret = SQLAllocHandle(SQL_HANDLE_DESC, dbc, &dst);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLCopyDesc(ard, dst);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  SQLSMALLINT siSrc;
  ret = SQLGetDescField(ard, 1, SQL_DESC_CONCISE_TYPE, &siSrc, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLSMALLINT siDst;
  ret = SQLGetDescField(dst, 1, SQL_DESC_CONCISE_TYPE, &siDst, 0, NULL);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL(siSrc, siDst);

  ret = SQLFreeHandle(SQL_HANDLE_DESC, dst);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
}

// Test SQLCopyDesc rainy case
BOOST_AUTO_TEST_CASE(TestCopyDescriptorRainyCase) {
  SQLHANDLE ird;

  SQLRETURN ret = SQLGetStmtAttr(stmt, SQL_ATTR_IMP_ROW_DESC, &ird, 0, nullptr);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLCopyDesc(ard, ird);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

// For Linux driver manager SQLGetDiagRec is called instead of our
// SQLGetDiagRec. Driver manager SQLGetDiagRec does not return any error info.
// So no error check for Linux.
#if defined(_WIN32)
  OdbcClientError error = GetOdbcError(SQL_HANDLE_DESC, ird);
  BOOST_CHECK_EQUAL(error.sqlstate, "HY016");
  BOOST_CHECK_EQUAL(error.message,
                    "[Microsoft][ODBC Driver Manager] Cannot modify an IRD");
#elif defined(__APPLE__)
  OdbcClientError error = GetOdbcError(SQL_HANDLE_DESC, ard);
  BOOST_CHECK_EQUAL(error.sqlstate, "HY016");
  BOOST_CHECK_EQUAL(error.message,
                    "Cannot modify an implementation row descriptor");
#endif
}

BOOST_AUTO_TEST_SUITE_END()
