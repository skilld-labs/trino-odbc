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

#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include <chrono>

#include "timestream/odbc/utility.h"
#include "odbc_test_suite.h"
#include "test_utils.h"

using namespace timestream;
using namespace timestream_test;
using namespace timestream::odbc;
using timestream::odbc::utility::CheckEnvVarSetToTrue;

using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct QueriesTestSuiteFixture : odbc::OdbcTestSuite {
  template < typename T >
  void CheckTwoRowsInt(SQLSMALLINT type) {
    ConnectToTS();

    SQLRETURN ret;

    const SQLSMALLINT columnsCnt = 6;

    T columns[columnsCnt];

    std::memset(&columns, 0, sizeof(columns));

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]),
                       nullptr);

      if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    std::vector< SQLWCHAR > request = MakeSqlBuffer(
        "select device_id, cast(video_startup_time AS int), "
        "video_startup_time, rebuffering_ratio,"
        "flag from data_queries_test_db.TestScalarTypes where "
        "video_startup_time < 3 order by device_id");

    ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFetch(stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_CHECK_EQUAL(columns[0], 1);
    BOOST_CHECK_EQUAL(columns[1], 1);
    BOOST_CHECK_EQUAL(columns[2], 1);
    BOOST_CHECK_EQUAL(columns[3], 0);
    BOOST_CHECK_EQUAL(columns[4], 1);

    SQLLEN columnLens[columnsCnt];

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]),
                       &columnLens[i]);

      if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    ret = SQLFetch(stmt);
    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_CHECK_EQUAL(columns[0], 2);
    BOOST_CHECK_EQUAL(columns[1], 2);
    BOOST_CHECK_EQUAL(columns[2], 2);
    BOOST_CHECK_EQUAL(columns[3], 0);
    BOOST_CHECK_EQUAL(columns[4], 0);

    BOOST_CHECK_EQUAL(columnLens[0], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[1], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[2], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[3], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[4], static_cast< SQLLEN >(sizeof(T)));

    ret = SQLFetch(stmt);
    BOOST_CHECK(ret == SQL_NO_DATA);
  }

  int CountRows(SQLHSTMT stmt) {
    int res = 0;

    SQLRETURN ret = SQL_SUCCESS;

    while (ret == SQL_SUCCESS) {
      ret = SQLFetch(stmt);

      if (ret == SQL_NO_DATA)
        break;

      if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

      ++res;
    }

    return res;
  }

  std::string BigTablePaginationTestIsEnabled;
};

BOOST_FIXTURE_TEST_SUITE(QueriesTestSuite, QueriesTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestSingleResultUsingBindCol) {
  ConnectToTS();
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

  SQL_TIMESTAMP_STRUCT timestamp;
  SQLLEN timestamp_len = 0;
  ret = SQLBindCol(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  bool fieldBoolean = false;
  SQLLEN fieldBoolean_len = 0;
  ret = SQLBindCol(stmt, 3, SQL_C_BIT, &fieldBoolean, sizeof(fieldBoolean),
                   &fieldBoolean_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  double fieldDouble = 0;
  SQLLEN fieldDouble_len = 0;
  ret = SQLBindCol(stmt, 4, SQL_C_DOUBLE, &fieldDouble, sizeof(fieldDouble),
                   &fieldDouble_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLBIGINT fieldLong;
  SQLLEN fieldLong_len = 0;
  ret = SQLBindCol(stmt, 5, SQL_C_SBIGINT, &fieldLong, sizeof(fieldLong),
                   &fieldLong_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  DATE_STRUCT fieldDate{};
  SQLLEN fieldDate_len = 0;
  ret = SQLBindCol(stmt, 6, SQL_C_TYPE_DATE, &fieldDate, sizeof(fieldDate),
                   &fieldDate_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  TIME_STRUCT timeValue;
  SQLLEN timeValue_len = 0;
  ret = SQLBindCol(stmt, 7, SQL_C_TYPE_TIME, &timeValue, sizeof(timeValue),
                   &timeValue_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_INTERVAL_STRUCT yearMonth;
  SQLLEN yearMonth_len = 0;
  ret = SQLBindCol(stmt, 8, SQL_C_INTERVAL_YEAR_TO_MONTH, &yearMonth,
                   sizeof(yearMonth), &yearMonth_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_INTERVAL_STRUCT daySecond;
  SQLLEN daySecond_len = 0;
  ret = SQLBindCol(stmt, 9, SQL_C_INTERVAL_DAY_TO_SECOND, &daySecond,
                   sizeof(daySecond), &daySecond_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_TIMESTAMP_STRUCT current_timestamp;
  SQLLEN current_timestamp_len = 0;
  ret = SQLBindCol(stmt, 10, SQL_C_TYPE_TIMESTAMP, &current_timestamp,
                   sizeof(current_timestamp), &current_timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("00000001", utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 10);
  BOOST_CHECK_EQUAL(timestamp.day, 20);
  BOOST_CHECK_EQUAL(true, fieldBoolean);
  BOOST_CHECK_EQUAL(0.1, fieldDouble);
  BOOST_CHECK_EQUAL(1, fieldLong);

  BOOST_CHECK_EQUAL(fieldDate.year, 2022);
  BOOST_CHECK_EQUAL(fieldDate.month, 7);
  BOOST_CHECK_EQUAL(fieldDate.day, 7);

  BOOST_CHECK_EQUAL(timeValue.hour, current_timestamp.hour);
  BOOST_CHECK_EQUAL(timeValue.minute, current_timestamp.minute);
  BOOST_CHECK_EQUAL(timeValue.second, current_timestamp.second);

  BOOST_CHECK_EQUAL(yearMonth.interval_type, SQL_IS_YEAR_TO_MONTH);
  BOOST_CHECK_EQUAL(yearMonth.intval.year_month.year, 4);
  BOOST_CHECK_EQUAL(yearMonth.intval.year_month.month, 2);

  BOOST_CHECK_EQUAL(daySecond.interval_type, SQL_IS_DAY_TO_SECOND);
  BOOST_CHECK_EQUAL(daySecond.intval.day_second.day, 6);
  BOOST_CHECK_EQUAL(daySecond.intval.day_second.hour, 4);

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestNoDataErrorMessage) {
  ConnectToTS();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select * from data_queries_test_db.TestScalarTypes limit 1");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLMoreResults(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
  BOOST_REQUIRE_EQUAL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt),
                      "Cannot find ODBC error message");
}

BOOST_AUTO_TEST_CASE(TestSingleResultUsingGetData) {
  ConnectToTS();
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

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_TIMESTAMP_STRUCT timestamp;
  SQLLEN timestamp_len = 0;
  ret = SQLGetData(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  bool fieldBoolean = false;
  SQLLEN fieldBoolean_len = 0;
  ret = SQLGetData(stmt, 3, SQL_C_BIT, &fieldBoolean, sizeof(fieldBoolean),
                   &fieldBoolean_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  double fieldDouble = 0;
  SQLLEN fieldDouble_len = 0;
  ret = SQLGetData(stmt, 4, SQL_C_DOUBLE, &fieldDouble, sizeof(fieldDouble),
                   &fieldDouble_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLBIGINT fieldLong;
  SQLLEN fieldLong_len = 0;
  ret = SQLGetData(stmt, 5, SQL_C_SBIGINT, &fieldLong, sizeof(fieldLong),
                   &fieldLong_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  DATE_STRUCT fieldDate{};
  SQLLEN fieldDate_len = 0;
  ret = SQLGetData(stmt, 6, SQL_C_TYPE_DATE, &fieldDate, sizeof(fieldDate),
                   &fieldDate_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  TIME_STRUCT timeValue;
  SQLLEN timeValue_len = 0;
  ret = SQLGetData(stmt, 7, SQL_C_TYPE_TIME, &timeValue, sizeof(timeValue),
                   &timeValue_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS_WITH_INFO, ret);

  SQL_INTERVAL_STRUCT yearMonth;
  SQLLEN yearMonth_len = 0;
  ret = SQLGetData(stmt, 8, SQL_C_INTERVAL_YEAR_TO_MONTH, &yearMonth,
                   sizeof(yearMonth), &yearMonth_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_INTERVAL_STRUCT daySecond;
  SQLLEN daySecond_len = 0;
  ret = SQLGetData(stmt, 9, SQL_C_INTERVAL_DAY_TO_SECOND, &daySecond,
                   sizeof(daySecond), &daySecond_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_TIMESTAMP_STRUCT current_timestamp;
  SQLLEN current_timestamp_len = 0;
  ret = SQLGetData(stmt, 10, SQL_C_TYPE_TIMESTAMP, &current_timestamp,
                   sizeof(current_timestamp), &current_timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("00000001", utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 10);
  BOOST_CHECK_EQUAL(timestamp.day, 20);
  BOOST_CHECK_EQUAL(true, fieldBoolean);
  BOOST_CHECK_EQUAL(0.1, fieldDouble);
  BOOST_CHECK_EQUAL(1, fieldLong);

  BOOST_CHECK_EQUAL(fieldDate.year, 2022);
  BOOST_CHECK_EQUAL(fieldDate.month, 7);
  BOOST_CHECK_EQUAL(fieldDate.day, 7);

  BOOST_CHECK_EQUAL(timeValue.hour, current_timestamp.hour);
  BOOST_CHECK_EQUAL(timeValue.minute, current_timestamp.minute);
  BOOST_CHECK_EQUAL(timeValue.second, current_timestamp.second);

  BOOST_CHECK_EQUAL(yearMonth.interval_type, SQL_IS_YEAR_TO_MONTH);
  BOOST_CHECK_EQUAL(yearMonth.intval.year_month.year, 4);
  BOOST_CHECK_EQUAL(yearMonth.intval.year_month.month, 2);

  BOOST_CHECK_EQUAL(daySecond.interval_type, SQL_IS_DAY_TO_SECOND);
  BOOST_CHECK_EQUAL(daySecond.intval.day_second.day, 6);
  BOOST_CHECK_EQUAL(daySecond.intval.day_second.hour, 4);

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestMultiLineResultUsingGetData) {
  ConnectToTS();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time from data_queries_test_db.TestScalarTypes order "
      "by device_id limit 3");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_TIMESTAMP_STRUCT timestamp;
  SQLLEN timestamp_len = 0;
  ret = SQLGetData(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("00000001", utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 10);
  BOOST_CHECK_EQUAL(timestamp.day, 20);

  // Fetch 2nd row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("00000002", utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 10);
  BOOST_CHECK_EQUAL(timestamp.day, 21);

  // Fetch 3rd row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("00000003", utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 10);
  BOOST_CHECK_EQUAL(timestamp.day, 22);

  // Fetch 4th row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSQLFetchTimeStampAsOtherTypes) {
  ConnectToTS();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select time as firstTime, time, time, time from "
      "data_queries_test_db.TestScalarTypes order by firstTime");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR timestampChar[buf_size]{};
  SQLLEN timestampChar_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, timestampChar, sizeof(timestampChar),
                   &timestampChar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR timestampWchar[buf_size]{};
  SQLLEN timestampWchar_len = 0;
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, &timestampWchar,
                   sizeof(timestampWchar), &timestampWchar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  DATE_STRUCT fieldDate{};
  SQLLEN fieldDate_len = 0;
  ret = SQLBindCol(stmt, 3, SQL_C_TYPE_DATE, &fieldDate, sizeof(fieldDate),
                   &fieldDate_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  TIME_STRUCT timeValue;
  SQLLEN timeValue_len = 0;
  ret = SQLBindCol(stmt, 4, SQL_C_TYPE_TIME, &timeValue, sizeof(timeValue),
                   &timeValue_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("2022-10-20 19:01:02.000000000",
                    utility::SqlCharToString(timestampChar, timestampChar_len));
  BOOST_CHECK_EQUAL(
      "2022-10-20 19:01:02.000000000",
      utility::SqlWcharToString(timestampWchar, timestampWchar_len));

  BOOST_CHECK_EQUAL(fieldDate.year, 2022);
  BOOST_CHECK_EQUAL(fieldDate.month, 10);
  BOOST_CHECK_EQUAL(fieldDate.day, 20);

  BOOST_CHECK_EQUAL(timeValue.hour, 19);
  BOOST_CHECK_EQUAL(timeValue.minute, 1);
  BOOST_CHECK_EQUAL(timeValue.second, 2);
}

BOOST_AUTO_TEST_CASE(TestSQLFetchTimeAsOtherTypes) {
  ConnectToTS();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select current_time, current_time, current_time, current_time");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR timestampChar[buf_size]{};
  SQLLEN timestampChar_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, timestampChar, sizeof(timestampChar),
                   &timestampChar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR timestampWchar[buf_size]{};
  SQLLEN timestampWchar_len = 0;
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, &timestampWchar,
                   sizeof(timestampWchar), &timestampWchar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_TIMESTAMP_STRUCT timestamp;
  SQLLEN timestamp_len = 0;
  ret = SQLBindCol(stmt, 3, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  TIME_STRUCT timeValue;
  SQLLEN timeValue_len = 0;
  ret = SQLBindCol(stmt, 4, SQL_C_TYPE_TIME, &timeValue, sizeof(timeValue),
                   &timeValue_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL(
      utility::SqlCharToString(timestampChar, timestampChar_len),
      utility::SqlWcharToString(timestampWchar, timestampWchar_len));

  BOOST_CHECK_EQUAL(timeValue.hour, timestamp.hour);
  BOOST_CHECK_EQUAL(timeValue.minute, timestamp.minute);
  BOOST_CHECK_EQUAL(timeValue.second, timestamp.second);
}

BOOST_AUTO_TEST_CASE(TestSQLFetchDateAsOtherTypes) {
  ConnectToTS();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select date(TIMESTAMP '2022-07-07 17:44:43.771000000'),"
      "date(TIMESTAMP '2022-07-07 17:44:43.771000000'),"
      "date(TIMESTAMP '2022-07-07 17:44:43.771000000')");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR timestampChar[buf_size]{};
  SQLLEN timestampChar_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, timestampChar, sizeof(timestampChar),
                   &timestampChar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR timestampWchar[buf_size]{};
  SQLLEN timestampWchar_len = 0;
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, &timestampWchar,
                   sizeof(timestampWchar), &timestampWchar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_TIMESTAMP_STRUCT timestamp;
  SQLLEN timestamp_len = 0;
  ret = SQLBindCol(stmt, 3, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("2022-07-07",
                    utility::SqlCharToString(timestampChar, timestampChar_len));
  BOOST_CHECK_EQUAL("2022-07-07", utility::SqlWcharToString(
                                      timestampWchar, timestampWchar_len));

  BOOST_CHECK_EQUAL(2022, timestamp.year);
  BOOST_CHECK_EQUAL(7, timestamp.month);
  BOOST_CHECK_EQUAL(7, timestamp.day);
  BOOST_CHECK_EQUAL(0, timestamp.hour);
  BOOST_CHECK_EQUAL(0, timestamp.minute);
  BOOST_CHECK_EQUAL(0, timestamp.second);
  BOOST_CHECK_EQUAL(0, timestamp.fraction);
}

BOOST_AUTO_TEST_CASE(TestSQLFetchIntervalYearMonthAsOtherTypes) {
  ConnectToTS();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT interval '3' year + interval '11' month, interval '3' year + "
      "interval '11' month");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR yearMonthChar[buf_size]{};
  SQLLEN yearMonthChar_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, &yearMonthChar, sizeof(yearMonthChar),
                   &yearMonthChar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR yearMonthWchar[buf_size]{};
  SQLLEN yearMonthWchar_len = 0;
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, &yearMonthWchar,
                   sizeof(yearMonthWchar), &yearMonthWchar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("3-11",
                    utility::SqlCharToString(yearMonthChar, yearMonthChar_len));

  BOOST_CHECK_EQUAL(
      "3-11", utility::SqlWcharToString(yearMonthWchar, yearMonthWchar_len));
}

BOOST_AUTO_TEST_CASE(TestSQLFetchIntervalDayMonthAsOtherTypes) {
  ConnectToTS();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT interval '6' day + interval '0' hour, interval '0' day + "
      "interval '4' hour");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR daySecondChar[buf_size]{};
  SQLLEN daySecondChar_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, &daySecondChar, sizeof(daySecondChar),
                   &daySecondChar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR daySecondWchar[buf_size]{};
  SQLLEN daySecondWchar_len = 0;
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, &daySecondWchar,
                   sizeof(daySecondWchar), &daySecondWchar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("6 00:00:00.000000000",
                    utility::SqlCharToString(daySecondChar, daySecondChar_len));

  BOOST_CHECK_EQUAL(
      "0 04:00:00.000000000",
      utility::SqlWcharToString(daySecondWchar, daySecondWchar_len));
}

BOOST_AUTO_TEST_CASE(TestTimeSeriesSingleResultUsingBindCol) {
  ConnectToTS();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT region, az, vpc, instance_id,"
      "CREATE_TIME_SERIES(time, measure_value::double) as cpu_utilization, "
      "CREATE_TIME_SERIES(time, measure_value::double) as cpu_utilization2 "
      "FROM data_queries_test_db.TestComplexTypes WHERE "
      "measure_name='cpu_utilization' "
      "GROUP BY region, az, vpc, instance_id order by instance_id");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR timeSeriesChar[buf_size]{};
  SQLLEN timeSeriesChar_len = 0;

  // fetch result as a string
  ret = SQLBindCol(stmt, 5, SQL_C_CHAR, timeSeriesChar, sizeof(timeSeriesChar),
                   &timeSeriesChar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR timeSeriesWchar[buf_size]{};
  SQLLEN timeSeriesWchar_len = 0;

  // fetch result as a unicode string
  ret = SQLBindCol(stmt, 6, SQL_C_WCHAR, timeSeriesWchar,
                   sizeof(timeSeriesWchar), &timeSeriesWchar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  std::string expected = "[{time: 2019-12-04 19:00:00.000000000, value: 35.2},";
  expected += "{time: 2019-12-04 19:01:00.000000000, value: 38.2},";
  expected += "{time: 2019-12-04 19:02:00.000000000, value: 45.3}]";

  BOOST_CHECK_EQUAL(
      expected, utility::SqlCharToString(timeSeriesChar, timeSeriesChar_len));

  BOOST_CHECK_EQUAL(expected, utility::SqlWcharToString(
                                  timeSeriesWchar, timeSeriesWchar_len, true));
}

BOOST_AUTO_TEST_CASE(TestArraySingleResultUsingBindCol) {
  ConnectToTS();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request =
      MakeSqlBuffer("select Array[1,2,3], Array[1,2,3], Array[], Array[]");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR arrayChar1[buf_size]{};
  SQLLEN arrayChar1_len = 0;

  // fetch result as a string
  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, arrayChar1, sizeof(arrayChar1),
                   &arrayChar1_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR arrayWchar1[buf_size]{};
  SQLLEN arrayWchar1_len = 0;

  // fetch result as a unicode string
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, arrayWchar1, sizeof(arrayWchar1),
                   &arrayWchar1_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLCHAR arrayChar2[buf_size]{};
  SQLLEN arrayChar2_len = 0;

  // fetch result as a string
  ret = SQLBindCol(stmt, 3, SQL_C_CHAR, arrayChar2, sizeof(arrayChar2),
                   &arrayChar2_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR arrayWchar2[buf_size]{};
  SQLLEN arrayWchar2_len = 0;

  // fetch result as a unicode string
  ret = SQLBindCol(stmt, 4, SQL_C_WCHAR, arrayWchar2, sizeof(arrayWchar2),
                   &arrayWchar2_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("[1,2,3]",
                    utility::SqlCharToString(arrayChar1, arrayChar1_len));
  BOOST_CHECK_EQUAL(
      "[1,2,3]", utility::SqlWcharToString(arrayWchar1, arrayWchar1_len, true));
  BOOST_CHECK_EQUAL("-", utility::SqlCharToString(arrayChar2, arrayChar2_len));
  BOOST_CHECK_EQUAL(
      "-", utility::SqlWcharToString(arrayWchar2, arrayWchar2_len, true));
}

BOOST_AUTO_TEST_CASE(TestRowSingleResultUsingBindCol) {
  ConnectToTS();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer("SELECT (1,2,3), (1,2,3)");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR rowChar[buf_size]{};
  SQLLEN rowChar_len = 0;

  // fetch result as a string
  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, rowChar, sizeof(rowChar), &rowChar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR rowWchar[buf_size]{};
  SQLLEN rowWchar_len = 0;

  // fetch result as a unicode string
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, rowWchar, sizeof(rowWchar),
                   &rowWchar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("(1,2,3)", utility::SqlCharToString(rowChar, rowChar_len));
  BOOST_CHECK_EQUAL("(1,2,3)",
                    utility::SqlWcharToString(rowWchar, rowWchar_len, true));
}

BOOST_AUTO_TEST_CASE(TestNullSingleResultUsingBindCol) {
  ConnectToTS();
  SQLRETURN ret;

  std::vector< SQLWCHAR > request = MakeSqlBuffer("select null, null");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR nullChar[buf_size]{};
  SQLLEN nullChar_len = 0;

  // fetch result as a string
  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, nullChar, sizeof(nullChar),
                   &nullChar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLWCHAR nullWchar[buf_size]{};
  SQLLEN nullWchar_len = 0;

  // fetch result as a unicode string
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, nullWchar, sizeof(nullWchar),
                   &nullWchar_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("-", utility::SqlCharToString(nullChar, nullChar_len));
  BOOST_CHECK_EQUAL("-",
                    utility::SqlWcharToString(nullWchar, nullWchar_len, true));
}

BOOST_AUTO_TEST_CASE(TestSQLCancel) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select time, index, cpu_utilization from "
      "data_queries_test_db.TestMultiMeasureBigTable order by time");
  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLCancel(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
#if defined(__linux__) || defined(__APPLE__)
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);
#else
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
#endif
}

BOOST_AUTO_TEST_CASE(TestSQLCloseCursor) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select time, index, cpu_utilization from "
      "data_queries_test_db.TestMultiMeasureBigTable order by time");
  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLCloseCursor(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  ret = SQLCloseCursor(stmt);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

#ifdef __linux__
  BOOST_REQUIRE_EQUAL("24000: [unixODBC][Driver Manager]Invalid cursor state",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#else
  BOOST_REQUIRE_EQUAL("24000: No cursor was open",
                      GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
#endif
}

BOOST_AUTO_TEST_CASE(TestSQLFetchBigTablePagination) {
  if (CheckEnvVarSetToTrue("BIG_TABLE_PAGINATION_TEST_ENABLE")) {
    // This test verifies big table resultset could be paginated
    // and the returned data is correct
    ConnectToTS();
    SQLRETURN ret;
    // data_queries_test_db.TestMultiMeasureBigTable is a big table which has
    // 20,000 records and the resultset will be paginated by default
    std::vector< SQLWCHAR > request = MakeSqlBuffer(
        "select time, index, cpu_utilization from "
        "data_queries_test_db.TestMultiMeasureBigTable order by time");
    ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
    SQL_TIMESTAMP_STRUCT timestamp;
    SQLLEN timestamp_len = 0;
    ret = SQLBindCol(stmt, 1, SQL_C_TYPE_TIMESTAMP, &timestamp,
                     sizeof(timestamp), &timestamp_len);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
    SQLBIGINT fieldLong;
    SQLLEN fieldLong_len = 0;
    ret = SQLBindCol(stmt, 2, SQL_C_SBIGINT, &fieldLong, sizeof(fieldLong),
                     &fieldLong_len);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
    double fieldDouble = 0;
    SQLLEN fieldDouble_len = 0;
    ret = SQLBindCol(stmt, 3, SQL_C_DOUBLE, &fieldDouble, sizeof(fieldDouble),
                     &fieldDouble_len);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

    // Get 1st row of current page
    ret = SQLFetch(stmt);
    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    // verify result
    BOOST_CHECK_EQUAL(1, fieldLong);
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLExecBigTablePagination) {
  if (CheckEnvVarSetToTrue("BIG_TABLE_PAGINATION_TEST_ENABLE")) {
    // This test verifies the internal asynchronous thread could be
    // terminated when testcase ends. It also verifies all rows could
    // be fetched including the last page.
    ConnectToTS();
    SQLRETURN ret;
    std::vector< SQLWCHAR > request = MakeSqlBuffer(
        "select time, index, cpu_utilization from "
        "data_queries_test_db.TestMultiMeasureBigTable order by time");
    ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

    int count = 0;
    do {
      ret = SQLFetch(stmt);
      if (ret == SQL_NO_DATA) {
        break;
      } else if (!SQL_SUCCEEDED(ret)) {
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      }
      count++;
    } while (true);

    BOOST_CHECK_EQUAL(20000, count);
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLFetchBigTablePagination1000Rows) {
  if (CheckEnvVarSetToTrue("BIG_TABLE_PAGINATION_TEST_ENABLE")) {
    // Fetch 1000 rows and verify the resultset is correct for 1001st record.
    // Each page contains only one row. There will be 1000 internal asynchronous
    // threads created to fetch 1000 pages.
    std::string dsnConnectionString;
    CreateDsnConnectionStringForAWS(dsnConnectionString);
    AddMaxRowPerPage(dsnConnectionString, "1");
    Connect(dsnConnectionString);
    SQLRETURN ret;
    std::vector< SQLWCHAR > request = MakeSqlBuffer(
        "select time, index, cpu_utilization from "
        "data_queries_test_db.TestMultiMeasureBigTable order by time");
    ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

    int count = 0;

    // These time_points could be reopened in case there is
    // a performance check need.
    // std::chrono::steady_clock::time_point time_exec_start =
    //    std::chrono::steady_clock::now();

    // fetch 1000 rows
    do {
      ret = SQLFetch(stmt);
      if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
      count++;
    } while (count < 1000);

    /*
    std::chrono::steady_clock::time_point time_exec_end =
        std::chrono::steady_clock::now();

    std::chrono::steady_clock::duration time_span =
        time_exec_end - time_exec_start;

    double nseconds = double(time_span.count())
                      * std::chrono::steady_clock::period::num
                      / std::chrono::steady_clock::period::den;
    std::cout << "Fetching 1000 rows took " << nseconds << " seconds"
              << std::endl;
    */

    SQL_TIMESTAMP_STRUCT timestamp;
    SQLLEN timestamp_len = 0;
    ret = SQLBindCol(stmt, 1, SQL_C_TYPE_TIMESTAMP, &timestamp,
                     sizeof(timestamp), &timestamp_len);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
    SQLBIGINT fieldLong;
    SQLLEN fieldLong_len = 0;
    ret = SQLBindCol(stmt, 2, SQL_C_SBIGINT, &fieldLong, sizeof(fieldLong),
                     &fieldLong_len);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
    double fieldDouble = 0;
    SQLLEN fieldDouble_len = 0;
    ret = SQLBindCol(stmt, 3, SQL_C_DOUBLE, &fieldDouble, sizeof(fieldDouble),
                     &fieldDouble_len);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

    // Get 1001st row
    ret = SQLFetch(stmt);
    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    // verify result
    BOOST_CHECK_EQUAL(1001, fieldLong);
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSmallResultPagination) {
  // This test runs a query which returns 3 rows. It sets each page
  // contains 1 row. It verifies the results are correct.
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time from data_queries_test_db.TestScalarTypes order "
      "by device_id limit 3");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Fetch 1st row
  // These time_points could be reopened in case there is
  // a performance check need.
  // std::chrono::steady_clock::time_point time_exec_start =
  //    std::chrono::steady_clock::now();
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  /*
  std::chrono::steady_clock::time_point time_exec_end =
      std::chrono::steady_clock::now();

  std::chrono::steady_clock::duration time_span =
      time_exec_end - time_exec_start;

  double nseconds = double(time_span.count())
                    * std::chrono::steady_clock::period::num
                    / std::chrono::steady_clock::period::den;
  std::cout << "Fetching 1st row took " << nseconds << " seconds" << std::endl;
  */

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_TIMESTAMP_STRUCT timestamp;
  SQLLEN timestamp_len = 0;
  ret = SQLGetData(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("00000001", utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 10);
  BOOST_CHECK_EQUAL(timestamp.day, 20);

  // Fetch 2nd row
  // time_exec_start = std::chrono::steady_clock::now();
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  /*
  time_exec_end = std::chrono::steady_clock::now();

  time_span = time_exec_end - time_exec_start;

  nseconds = double(time_span.count()) * std::chrono::steady_clock::period::num
             / std::chrono::steady_clock::period::den;
  std::cout << "Fetching 2nd row took " << nseconds << " seconds" << std::endl;
  */

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("00000002", utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 10);
  BOOST_CHECK_EQUAL(timestamp.day, 21);

  // Fetch 3rd row
  // time_exec_start = std::chrono::steady_clock::now();
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  /*
  time_exec_end = std::chrono::steady_clock::now();

  time_span = time_exec_end - time_exec_start;

  nseconds = double(time_span.count()) * std::chrono::steady_clock::period::num
             / std::chrono::steady_clock::period::den;
  std::cout << "Fetching 3rd row took " << nseconds << " seconds" << std::endl;
  */

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("00000003", utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 10);
  BOOST_CHECK_EQUAL(timestamp.day, 22);

  // Fetch 4th row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSmallResultPaginationTermination) {
  // This test runs a query which returns 3 rows. It sets each page
  // contains 1 row. It only fetches the first two rows. It could verify
  // the 3rd row asynchronous thread be terminated without a problem.
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time from data_queries_test_db.TestScalarTypes order "
      "by device_id limit 3");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQL_TIMESTAMP_STRUCT timestamp;
  SQLLEN timestamp_len = 0;
  ret = SQLGetData(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("00000001", utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 10);
  BOOST_CHECK_EQUAL(timestamp.day, 20);

  // Fetch 2nd row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 2, SQL_C_TYPE_TIMESTAMP, &timestamp, sizeof(timestamp),
                   &timestamp_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_EQUAL("00000002", utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(timestamp.year, 2022);
  BOOST_CHECK_EQUAL(timestamp.month, 10);
  BOOST_CHECK_EQUAL(timestamp.day, 21);
}

BOOST_AUTO_TEST_CASE(TestSmallResultPaginationNoFetch) {
  // This test runs a query which returns 3 rows. It sets each page
  // to contain 1 row. It does not fetch any data. It verifies that
  // the asynchronous thread could be terminated without a problem.
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time from data_queries_test_db.TestScalarTypes order "
      "by device_id limit 3");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestSQLFetchPaginationEmptyTable) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForAWS(dsnConnectionString);
  AddMaxRowPerPage(dsnConnectionString, "1");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select measure_name, time from data_queries_test_db.EmptyTable");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  BOOST_REQUIRE_NE(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt)
                       .find("01000: Query result is empty"),
                   std::string::npos);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSQLRowCountWithNoResults) {
  ConnectToTS();
  SQLRETURN ret;

  std::vector< SQLWCHAR > sql = MakeSqlBuffer(
      "select * from data_queries_test_db.TestScalarTypes limit 20");

  ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLLEN rows = 0;

  ret = SQLRowCount(stmt, &rows);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // SQLRowCount should set rows to -1 as no rows were changed
  BOOST_CHECK_EQUAL(-1, rows);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt8) {
  CheckTwoRowsInt< signed char >(SQL_C_STINYINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint8) {
  CheckTwoRowsInt< unsigned char >(SQL_C_UTINYINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt16) {
  CheckTwoRowsInt< signed short >(SQL_C_SSHORT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint16) {
  CheckTwoRowsInt< unsigned short >(SQL_C_USHORT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt32) {
  CheckTwoRowsInt< SQLINTEGER >(SQL_C_SLONG);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint32) {
  CheckTwoRowsInt< SQLUINTEGER >(SQL_C_ULONG);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt64) {
  CheckTwoRowsInt< int64_t >(SQL_C_SBIGINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint64) {
  CheckTwoRowsInt< uint64_t >(SQL_C_UBIGINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsString) {
  ConnectToTS();

  SQLRETURN ret;

  const SQLSMALLINT columnsCnt = 5;

  SQLWCHAR columns[columnsCnt][ODBC_BUFFER_SIZE];
  SQLLEN reslen;

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i],
                     ODBC_BUFFER_SIZE * sizeof(SQLWCHAR), &reslen);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, cast(video_startup_time AS int), video_startup_time, "
      "rebuffering_ratio, flag from data_queries_test_db.TestScalarTypes where "
      "video_startup_time "
      "< 3 order by device_id");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true),
                    "00000001");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[1], SQL_NTS, true), "1");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[2], SQL_NTS, true), "1");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[3], SQL_NTS, true),
                    "0.1");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[4], SQL_NTS, true), "1");

  SQLLEN columnLens[columnsCnt];

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i],
                     ODBC_BUFFER_SIZE * sizeof(SQLWCHAR), &columnLens[i]);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true),
                    "00000002");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[1], SQL_NTS, true), "2");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[2], SQL_NTS, true), "2");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[3], SQL_NTS, true),
                    "0.2");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[4], SQL_NTS, true), "0");

#ifdef __APPLE__
  SQLLEN expectedLen = 4;
  BOOST_CHECK_EQUAL(columnLens[0], 32);
#else
  SQLLEN expectedLen = 2;
  BOOST_CHECK_EQUAL(columnLens[0], 16);
#endif

  BOOST_CHECK_EQUAL(columnLens[1], expectedLen);
  BOOST_CHECK_EQUAL(columnLens[2], expectedLen);
#ifdef __APPLE__
  BOOST_CHECK_EQUAL(columnLens[3], 12);
#else
  BOOST_CHECK_EQUAL(columnLens[3], 6);
#endif
  BOOST_CHECK_EQUAL(columnLens[4], expectedLen);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestDefaultValues) {
  ConnectToTS();

  SQLRETURN ret;

  const SQLSMALLINT columnsCnt = 6;

  SQLLEN columnLens[columnsCnt];

  char strColumn[ODBC_BUFFER_SIZE];
  SQL_TIMESTAMP_STRUCT timestampColumn;
  double doubleColumn;
  double defaultDoubleColumn = 1.0;
  int64_t defaultBigintColumn = 100;
  bool defaultBoolColumn = true;

  // Binding columns.
  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, &strColumn, ODBC_BUFFER_SIZE,
                   &columnLens[0]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret =
      SQLBindCol(stmt, 2, SQL_C_TIMESTAMP, &timestampColumn, 0, &columnLens[1]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 3, SQL_C_DOUBLE, &doubleColumn, 0, &columnLens[2]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 4, SQL_C_BIT, &defaultBoolColumn, 0, &columnLens[3]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 5, SQL_C_DOUBLE, &defaultDoubleColumn, 0,
                   &columnLens[4]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 6, SQL_C_SBIGINT, &defaultBigintColumn, 0,
                   &columnLens[5]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, time, cpu_usage, flag, rebuffering_ratio,"
      "video_startup_time from data_queries_test_db.TestScalarTypes where "
      "device_id='00000005'");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Fetching the first non-null row.
  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Checking that columns are not null.
  for (SQLSMALLINT i = 0; i < 3; ++i)
    BOOST_CHECK_NE(columnLens[i], SQL_NULL_DATA);

  BOOST_CHECK_EQUAL(defaultBoolColumn, false);
  BOOST_CHECK_EQUAL(defaultDoubleColumn, 0.0);
  BOOST_CHECK_EQUAL(defaultBigintColumn, 0);
  BOOST_CHECK_EQUAL(columnLens[3], 1);
  BOOST_CHECK_EQUAL(columnLens[4], 8);
  BOOST_CHECK_EQUAL(columnLens[5], 8);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLMoreResults) {
  ConnectToTS();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select * from data_queries_test_db.TestScalarTypes limit 4");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLMoreResults(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestExecuteAfterCursorClose) {
  ConnectToTS();

  double doubleField = 0.0;
  SQLWCHAR strField[1024];
  SQLLEN strFieldLen = 0;

  // Binding columns.
  SQLRETURN ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, &strField, sizeof(strField),
                             &strFieldLen);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Binding columns.
  ret = SQLBindCol(stmt, 2, SQL_C_DOUBLE, &doubleField, 0, 0);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > selectReq = MakeSqlBuffer(
      "select device_id, cpu_usage from data_queries_test_db.TestScalarTypes "
      "where device_id='00000005'");

  ret = SQLPrepare(stmt, selectReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLExecute(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFreeStmt(stmt, SQL_CLOSE);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLExecute(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(std::abs(doubleField - 63.7) < 0.1, true);

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(strField, strFieldLen, true),
                    "00000005");

  ret = SQLFetch(stmt);

  BOOST_CHECK_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestCloseNonFullFetch) {
  ConnectToTS();

  double doubleField = 0.0;
  SQLWCHAR strField[1024];
  SQLLEN strFieldLen = 0;

  // Binding columns.
  SQLRETURN ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, &strField, sizeof(strField),
                             &strFieldLen);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Binding columns.
  ret = SQLBindCol(stmt, 2, SQL_C_DOUBLE, &doubleField, 0, 0);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > selectReq = MakeSqlBuffer(
      "select device_id, cpu_usage from data_queries_test_db.TestScalarTypes "
      "where device_id='00000005'");

  ret = SQLExecDirect(stmt, selectReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(std::abs(doubleField - 63.7) < 0.1, true);

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(strField, strFieldLen, true),
                    "00000005");

  ret = SQLFreeStmt(stmt, SQL_CLOSE);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestErrorMessage) {
  ConnectToTS();

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > selectReq = MakeSqlBuffer("SELECT A FROM B");

  SQLRETURN ret = SQLExecDirect(stmt, selectReq.data(), SQL_NTS);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  std::string error = GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt);
  std::string pattern = "Failed to execute query";

  if (error.find(pattern) == std::string::npos)
    BOOST_FAIL("'" + error + "' does not match '" + pattern + "'");
}

BOOST_AUTO_TEST_CASE(TestManyCursors) {
  ConnectToTS();

  for (int32_t i = 0; i < 100; ++i) {
    std::vector< SQLWCHAR > req =
        MakeSqlBuffer("select * from data_queries_test_db.TestScalarTypes");

    SQLRETURN ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFreeStmt(stmt, SQL_CLOSE);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestManyCursors2) {
  ConnectToTS();

  SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  for (int32_t i = 0; i < 1000; ++i) {
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    std::vector< SQLWCHAR > req = MakeSqlBuffer(
        "select video_startup_time from data_queries_test_db.TestScalarTypes "
        "where device_id='00000001'");

    ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    int32_t res = 0;
    SQLLEN resLen = 0;
    ret = SQLBindCol(stmt, 1, SQL_INTEGER, &res, 0, &resLen);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFetch(stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_REQUIRE_EQUAL(res, 1);

    ret = SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    stmt = NULL;
  }
}

BOOST_AUTO_TEST_CASE(TestSingleResultUsingGetDataWideChar) {
  ConnectToTS();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "select device_id, region from data_queries_test_db.TestScalarTypes "
      "where device_id='00000006'");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLWCHAR fieldString[1024]{};
  SQLLEN fieldString_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 2, SQL_C_WCHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL(u8"美西-5", utility::SqlWcharToString(
                                    fieldString, fieldString_len, true));

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSingleResultSelectWideCharUsingGetDataWideChar) {
  ConnectToTS();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      u8"select device_id, region from data_queries_test_db.TestScalarTypes "
      u8"where region='美西-5'");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR fieldString[buf_size]{};
  SQLLEN fieldString_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 2, SQL_C_WCHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL(u8"美西-5", utility::SqlWcharToString(
                                    fieldString, fieldString_len, true));

  // Fetch 2nd row - does not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSingleResultSelectWideCharUsingGetDataNarrowChar) {
  ConnectToTS();
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      u8"select device_id, region from data_queries_test_db.TestScalarTypes "
      u8"where region='美西-5'");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR fieldString[buf_size]{};
  SQLLEN fieldString_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 2, SQL_C_CHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  if (ANSI_STRING_ONLY) {
    BOOST_CHECK_EQUAL(8, fieldString_len);
    BOOST_CHECK_EQUAL(std::string("美西-5"), std::string((char*)fieldString));
  } else {
    BOOST_CHECK_EQUAL(4, fieldString_len);
    BOOST_CHECK_EQUAL(std::string("??-5"), std::string((char*)fieldString));
  }

  // Fetch 2nd row - does not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_SUITE_END()
