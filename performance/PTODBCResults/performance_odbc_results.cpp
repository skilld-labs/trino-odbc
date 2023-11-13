/*
 * Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// clang-format off
#include "gtest/gtest.h"
#include "performance_helper.h"
#include "chrono"
#include <fstream>
#include <boost/thread.hpp>
#include <boost/thread/detail/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <numeric>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
// clang-format on

#define BIND_SIZE 255
#define ROWSET_SIZE_5 5
#define ROWSET_SIZE_50 50
#define SINGLE_ROW 1
#define ITERATION_COUNT 10

#ifndef WIN32
typedef SQLULEN SQLROWCOUNT;
typedef SQLULEN SQLROWSETSIZE;
typedef SQLULEN SQLTRANSID;
typedef SQLLEN SQLROWOFFSET;
#endif

std::vector< SQLWCHAR > connectionString;
std::string outFileName = "performance_results_report.csv";

const testString _query =
    CREATE_STRING("SELECT * FROM ODBCTest.DevOps LIMIT 10000");

typedef struct Col {
  SQLLEN data_len;
  SQLCHAR data_dat[BIND_SIZE];
} Col;

typedef struct WCol {
  SQLLEN data_len;
  SQLWCHAR data_dat[BIND_SIZE];
} WCol;

bool queryFinished = false;

// The number of executed tests
int testNumber = 0;

// Whether to run Q21_EXPECT_2000000_ROWS, which greatly extends runtime
bool enableLargeTest = false;

void prepareOutFile() {
  std::ofstream outFile(outFileName, std::ios::trunc);
  if (!outFile.is_open()) {
    std::cout << "Setting up csv file failed\n";
    return;
  }
  // Write header
  outFile << "Test Round,test_name,query,loop_count,Average Time (ms),Max Time "
             "(ms),Min Time (ms),"
             "Median Time (ms),90th Percentile (ms),Average Memory Usage "
             "(KB),Peak Memory Usage (KB)\n";
  outFile.close();
  return;
}

// Get the average and peak memory usage, assumed to be during a query
void queryMemUsage(long long& averageMem, long long& peakMem) {
  long long memSum = 0;
  long long memCalc;
  long long i = 0;
  do {
    memCalc = currentMemUsage();
    memSum += memCalc;
    if (memCalc > peakMem) {
      peakMem = memCalc;
    }
    i++;
    // Attempt to limit the number of memory calculations
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  } while (!queryFinished);

  if (i > 0) {
    averageMem = (memSum / i);
  }
}

auto RecordBindingFetching = [](SQLHSTMT& hstmt,
                                std::vector< long long >& times,
                                const testString& query, bool is_wchar) {
  SQLSMALLINT total_columns = 0;
  int row_count = 0;

  for (size_t iter = 0; iter < ITERATION_COUNT; iter++) {
    row_count = 0;
    // Execute query
    auto start = std::chrono::steady_clock::now();

    SQLRETURN ret = SQLExecDirect(hstmt, TO_SQLTCHAR(query.c_str()), SQL_NTS);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));

    // Get column count
    SQLNumResultCols(hstmt, &total_columns);

    // Performance tests use WCHAR exclusively since the Amazon Trino ODBC
    // driver is a unicode driver and CHAR is an uncommon real-life use case
    if (is_wchar) {
      std::vector< WCol > cols(total_columns);

      // Bind and fetch
      for (size_t i = 0; i < cols.size(); i++)
        ret = SQLBindCol(hstmt, static_cast< SQLUSMALLINT >(i + 1), SQL_C_WCHAR,
                         static_cast< SQLPOINTER >(&cols[i].data_dat), 255,
                         &cols[i].data_len);
      while (SQLFetch(hstmt) == SQL_SUCCESS)
        row_count++;
      auto end = std::chrono::steady_clock::now();
      std::cout << "Total rows: " << row_count << std::endl;
      times.push_back(
          std::chrono::duration_cast< std::chrono::milliseconds >(end - start)
              .count());
      logDiagnostics(SQL_HANDLE_STMT, hstmt, ret);
      SQLCloseCursor(hstmt);
    } else {
      std::vector< Col > cols(total_columns);

      // Bind and fetch
      for (size_t i = 0; i < cols.size(); i++)
        ret = SQLBindCol(hstmt, static_cast< SQLUSMALLINT >(i + 1), SQL_C_CHAR,
                         static_cast< SQLPOINTER >(&cols[i].data_dat), 255,
                         &cols[i].data_len);
      while (SQLFetch(hstmt) == SQL_SUCCESS)
        row_count++;
      auto end = std::chrono::steady_clock::now();
      std::cout << "Total rows: " << row_count << std::endl;
      times.push_back(
          std::chrono::duration_cast< std::chrono::milliseconds >(end - start)
              .count());
      logDiagnostics(SQL_HANDLE_STMT, hstmt, ret);
      SQLCloseCursor(hstmt);
    }
  }
  queryFinished = true;
};

// Test template for Amazon queries
#define TEST_PERF_TEST(test_name, query, is_wchar)                           \
  TEST_F(TestPerformance, test_name) {                                       \
    if (strcmp(                                                              \
            ::testing::UnitTest::GetInstance()->current_test_info()->name(), \
            "Q22_EXPECT_1500000_ROWS")                                       \
            == 0                                                             \
        && !enableLargeTest) {                                               \
      GTEST_SKIP();                                                          \
    }                                                                        \
    std::vector< long long > times;                                          \
    int currentMem = currentMemUsage();                                      \
    long long averageMem = 0;                                                \
    long long peakMem = 0;                                                   \
    boost::thread queryThread([&] {                                          \
      RecordBindingFetching(_hstmt, times, testString(query), is_wchar);     \
    });                                                                      \
    boost::thread memThread([&] { queryMemUsage(averageMem, peakMem); });    \
    queryThread.join();                                                      \
    memThread.join();                                                        \
    queryFinished = false;                                                   \
    Report(#test_name, times, testString(query), averageMem, peakMem);       \
  }

class TestPerformance : public testing::Test {
 public:
  SQLHENV _env = SQL_NULL_HENV;
  SQLHDBC _conn = SQL_NULL_HDBC;
  SQLHSTMT _hstmt = SQL_NULL_HSTMT;
  void SetUp() override {
    _env = SQL_NULL_HENV;
    _conn = SQL_NULL_HDBC;
    _hstmt = SQL_NULL_HSTMT;
    SQLRETURN ret;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_env);
    if (!SQL_SUCCEEDED(ret)) {
      logDiagnostics(SQL_HANDLE_ENV, _conn, ret);
      FAIL() << "SQLAllocHandle failed for environment";
    }
    ret = SQLSetEnvAttr(_env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(ret)) {
      logDiagnostics(SQL_HANDLE_ENV, _conn, ret);
      FAIL() << "SQLSetEnvAttr failed for environment";
    }
    ret = SQLAllocHandle(SQL_HANDLE_DBC, _env, &_conn);
    if (!SQL_SUCCEEDED(ret)) {
      logDiagnostics(SQL_HANDLE_DBC, _conn, ret);
      FAIL() << "SQLAllocHandle failed for database connection";
    }
    SQLTCHAR outConnString[1024];
    SQLSMALLINT outConnStringLen;
    ret = SQLDriverConnect(_conn, NULL, &connectionString[0], SQL_NTS,
                           outConnString, HELPER_SIZEOF(outConnString),
                           &outConnStringLen, SQL_DRIVER_COMPLETE);
    if (!SQL_SUCCEEDED(ret)) {
      logDiagnostics(SQL_HANDLE_DBC, _conn, ret);
      FAIL() << "SQLDriverConnect failed";
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, _conn, &_hstmt);
    if (!SQL_SUCCEEDED(ret)) {
      logDiagnostics(SQL_HANDLE_STMT, _conn, ret);
      FAIL() << "SQLAllocHandle failed for statement";
    }
  }
  void TearDown() override {
    if (_hstmt != SQL_NULL_HSTMT) {
      SQLCloseCursor(_hstmt);
      SQLFreeHandle(SQL_HANDLE_STMT, _hstmt);
    }
    if (_conn != SQL_NULL_HDBC) {
      SQLDisconnect(_conn);
      SQLFreeHandle(SQL_HANDLE_DBC, _conn);
    }
    if (_env != SQL_NULL_HENV) {
      SQLFreeHandle(SQL_HANDLE_ENV, _env);
    }
  }
};

const std::string sync_start = "%%__PARSE__SYNC__START__%%";
const std::string sync_query = "%%__QUERY__%%";
const std::string sync_case = "%%__CASE__%%";
const std::string sync_min = "%%__MIN__%%";
const std::string sync_max = "%%__MAX__%%";
const std::string sync_mean = "%%__MEAN__%%";
const std::string sync_median = "%%__MEDIAN__%%";
const std::string sync_percentile = "%%__90TH_PERCENTILE__%%";
const std::string sync_average_memory_usage = "%%__AVERAGE_MEMORY_USAGE__%%";
const std::string sync_peak_memory_usage = "%%__PEAK_MEMORY_USAGE__%%";
const std::string sync_end = "%%__PARSE__SYNC__END__%%";

void Report(const std::string& test_case, std::vector< long long > data,
            const testString& query, long long averageMemoryUsage,
            long long peakMemoryUsage) {
  size_t size = data.size();
  ASSERT_EQ(size, (size_t)ITERATION_COUNT);

  // Get max and min
  long long time_max = *std::max_element(data.begin(), data.end());
  long long time_min = *std::min_element(data.begin(), data.end());

  // Get median
  long long time_mean =
      std::accumulate(std::begin(data), std::end(data), 0ll) / data.size();

  // Get median
  std::sort(data.begin(), data.end());
  long long time_median = (size % 2)
                              ? data[size / 2]
                              : ((data[(size / 2) - 1] + data[size / 2]) / 2);

  // Calculate ordinal rank using nearest-rank method
  int ordinalRank = static_cast< int >((90 / 100.0) * data.size());
  // Find 90th percentile
  long long percentile = 0;
  if (ordinalRank > 0) {
    percentile = data[ordinalRank - 1];
  }

  // Output results
  std::cout << sync_start << std::endl;
  std::cout << sync_query;
  std::cout << sqltcharToStr(TO_SQLTCHAR(query.c_str())) << std::endl;
  std::cout << sync_case << test_case << std::endl;
  std::cout << sync_min << time_min << " ms" << std::endl;
  std::cout << sync_max << time_max << " ms" << std::endl;
  std::cout << sync_mean << time_mean << " ms" << std::endl;
  std::cout << sync_median << time_median << " ms" << std::endl;
  std::cout << sync_percentile << percentile << " ms" << std::endl;
  std::cout << sync_average_memory_usage << averageMemoryUsage << " KB"
            << std::endl;
  std::cout << sync_peak_memory_usage << peakMemoryUsage << " KB" << std::endl;
  std::cout << sync_end << std::endl;

  std::cout << "Time dump: ";
  for (size_t i = 0; i < data.size(); i++) {
    std::cout << data[i] << " ms";
    if (i != (data.size() - 1))
      std::cout << ", ";
  }
  std::cout << std::endl;

  // Write to csv
  std::ofstream outFile(outFileName, std::ios::app);
  if (!outFile.is_open()) {
    std::cout << "Writing test result to csv failed\n";
    return;
  }
  outFile << std::to_string(++testNumber) << ","
          << "\"" << test_case << "\","
          << "\"" << sqltcharToStr(TO_SQLTCHAR(query.c_str())) << "\","
          << std::to_string(ITERATION_COUNT) << "," << time_mean << ","
          << time_max << "," << time_min << "," << time_median << ","
          << percentile << "," << averageMemoryUsage << "," << peakMemoryUsage
          << "\n";
  outFile.close();
}

// The below DISABLED_* tests are optional miscellaneous tests with
// functionality more or less covered by the TEST_PERF_TEST numbered tests.
// These tests are disabled because they muddy test results and execute simple
// queries.

TEST_F(TestPerformance, DISABLED_Time_Execute) {
  // Execute a warm up query
  std::cout << "_hstmt: " << _hstmt << std::endl;
  std::vector< long long > times;
  int currentMem = currentMemUsage();
  long long averageMem = 0;
  long long peakMem = 0;
  boost::thread memoryThread([&] { queryMemUsage(averageMem, peakMem); });
  boost::thread queryThread = boost::thread([&] {
    SQLRETURN ret = SQLExecDirect(_hstmt, TO_SQLTCHAR(_query.c_str()), SQL_NTS);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    ASSERT_TRUE(SQL_SUCCEEDED(SQLCloseCursor(_hstmt)));

    for (size_t iter = 0; iter < ITERATION_COUNT; iter++) {
      auto start = std::chrono::steady_clock::now();
      ret = SQLExecDirect(_hstmt, (SQLTCHAR*)_query.c_str(), SQL_NTS);
      auto end = std::chrono::steady_clock::now();
      logDiagnostics(SQL_HANDLE_STMT, _hstmt, ret);
      ASSERT_TRUE(SQL_SUCCEEDED(ret));
      times.push_back(
          std::chrono::duration_cast< std::chrono::milliseconds >(end - start)
              .count());
    }
    queryFinished = true;
  });
  queryThread.join();
  memoryThread.join();
  queryFinished = false;
  Report("Execute Query", times, _query, averageMem, peakMem);
}

TEST_PERF_TEST(DISABLED_Time_BindColumn_FetchSingleRow, _query, true)

TEST_F(TestPerformance, DISABLED_Time_BindColumn_Fetch5Rows) {
  std::vector< long long > times;
  int currentMem = currentMemUsage();
  long long averageMem = 0;
  long long peakMem = 0;
  boost::thread memoryThread([&] { queryMemUsage(averageMem, peakMem); });
  boost::thread queryThread = boost::thread([&] {
    SQLROWSETSIZE row_count = 0;
    SQLSMALLINT total_columns = 0;
    SQLROWSETSIZE rows_fetched = 0;
    SQLUSMALLINT row_status[ROWSET_SIZE_5];
    SQLSetStmtAttr(_hstmt, SQL_ROWSET_SIZE, (void*)ROWSET_SIZE_5, 0);

    for (size_t iter = 0; iter < ITERATION_COUNT; iter++) {
      // Execute query
      SQLRETURN ret = SQLExecDirect(_hstmt, (SQLTCHAR*)_query.c_str(), SQL_NTS);
      ASSERT_TRUE(SQL_SUCCEEDED(ret));

      // Get column count
      SQLNumResultCols(_hstmt, &total_columns);
      std::vector< std::vector< Col > > cols(total_columns);
      for (size_t i = 0; i < cols.size(); i++)
        cols[i].resize(ROWSET_SIZE_5);

      // Bind and fetch
      auto start = std::chrono::steady_clock::now();
      for (size_t i = 0; i < cols.size(); i++)
        ret = SQLBindCol(_hstmt, (SQLUSMALLINT)i + 1, SQL_C_CHAR,
                         (SQLPOINTER)&cols[i][0].data_dat[i], BIND_SIZE,
                         &cols[i][0].data_len);
      while (
          SQLExtendedFetch(_hstmt, SQL_FETCH_NEXT, 0, &rows_fetched, row_status)
          == SQL_SUCCESS) {
        row_count += rows_fetched;
        if (rows_fetched < ROWSET_SIZE_5)
          break;
      }
      auto end = std::chrono::steady_clock::now();
      ASSERT_TRUE(SQL_SUCCEEDED(SQLCloseCursor(_hstmt)));
      times.push_back(
          std::chrono::duration_cast< std::chrono::milliseconds >(end - start)
              .count());
    }
    queryFinished = true;
  });
  queryThread.join();
  memoryThread.join();
  queryFinished = false;
  Report("Bind and (5 row) Fetch", times, _query, averageMem, peakMem);
}

TEST_F(TestPerformance, DISABLED_Time_BindColumn_Fetch50Rows) {
  std::vector< long long > times;
  int currentMem = currentMemUsage();
  long long averageMem = 0;
  long long peakMem = 0;
  boost::thread memoryThread([&] { queryMemUsage(averageMem, peakMem); });
  boost::thread queryThread = boost::thread([&] {
    SQLROWSETSIZE row_count = 0;
    SQLSMALLINT total_columns = 0;
    SQLROWSETSIZE rows_fetched = 0;
    SQLUSMALLINT row_status[ROWSET_SIZE_50];
    SQLSetStmtAttr(_hstmt, SQL_ROWSET_SIZE, (void*)ROWSET_SIZE_50, 0);

    for (size_t iter = 0; iter < ITERATION_COUNT; iter++) {
      // Execute query
      SQLRETURN ret = SQLExecDirect(_hstmt, (SQLTCHAR*)_query.c_str(), SQL_NTS);
      ASSERT_TRUE(SQL_SUCCEEDED(ret));

      // Get column count
      SQLNumResultCols(_hstmt, &total_columns);
      std::vector< std::vector< Col > > cols(total_columns);
      for (size_t i = 0; i < cols.size(); i++)
        cols[i].resize(ROWSET_SIZE_50);

      // Bind and fetch
      auto start = std::chrono::steady_clock::now();
      for (size_t i = 0; i < cols.size(); i++)
        ret = SQLBindCol(_hstmt, (SQLUSMALLINT)i + 1, SQL_C_CHAR,
                         (SQLPOINTER)&cols[i][0].data_dat[i], BIND_SIZE,
                         &cols[i][0].data_len);
      while (
          SQLExtendedFetch(_hstmt, SQL_FETCH_NEXT, 0, &rows_fetched, row_status)
          == SQL_SUCCESS) {
        row_count += rows_fetched;
        if (rows_fetched < ROWSET_SIZE_50)
          break;
      }

      auto end = std::chrono::steady_clock::now();
      ASSERT_TRUE(SQL_SUCCEEDED(SQLCloseCursor(_hstmt)));
      times.push_back(
          std::chrono::duration_cast< std::chrono::milliseconds >(end - start)
              .count());
    }
    queryFinished = true;
  });
  queryThread.join();
  memoryThread.join();
  queryFinished = false;
  Report("Bind and (50 row) Fetch", times, _query, averageMem, peakMem);
}

TEST_F(TestPerformance, DISABLED_Time_Execute_FetchSingleRow) {
  std::vector< long long > times;
  int currentMem = currentMemUsage();
  long long averageMem = 0;
  long long peakMem = 0;
  boost::thread memoryThread([&] { queryMemUsage(averageMem, peakMem); });
  boost::thread queryThread = boost::thread([&] {
    SQLSMALLINT total_columns = 0;
    int row_count = 0;

    for (size_t iter = 0; iter < ITERATION_COUNT; iter++) {
      // Execute query
      auto start = std::chrono::steady_clock::now();
      SQLRETURN ret = SQLExecDirect(_hstmt, (SQLTCHAR*)_query.c_str(), SQL_NTS);
      ASSERT_TRUE(SQL_SUCCEEDED(ret));

      // Get column count
      SQLNumResultCols(_hstmt, &total_columns);
      std::vector< std::vector< Col > > cols(total_columns);
      for (size_t i = 0; i < cols.size(); i++)
        cols[i].resize(SINGLE_ROW);

      // Bind and fetch
      for (size_t i = 0; i < cols.size(); i++)
        ret = SQLBindCol(_hstmt, (SQLUSMALLINT)i + 1, SQL_C_CHAR,
                         (SQLPOINTER)&cols[i][0].data_dat[i], BIND_SIZE,
                         &cols[i][0].data_len);
      while (SQLFetch(_hstmt) == SQL_SUCCESS)
        row_count++;

      auto end = std::chrono::steady_clock::now();
      ASSERT_TRUE(SQL_SUCCEEDED(SQLCloseCursor(_hstmt)));
      times.push_back(
          std::chrono::duration_cast< std::chrono::milliseconds >(end - start)
              .count());
    }
    queryFinished = true;
  });
  queryThread.join();
  memoryThread.join();
  queryFinished = false;
  Report("Execute Query, Bind and (1 row) Fetch", times, _query, averageMem,
         peakMem);
}

TEST_PERF_TEST(
    WARMING_UP,
    CREATE_STRING("SELECT * FROM perfdb_hcltps.perftable_hcltps LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q1_EXPECT_49_ROWS,
    CREATE_STRING(
        "SELECT BIN(time, 1m) AS time_bin, AVG(measure_value::double) AS "
        "avg_cpu FROM perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() "
        "- 100y AND now() AND measure_name = 'cpu_user' AND region = "
        "'us-east-1' "
        "AND cell = 'us-east-1-cell-1' AND silo = 'us-east-1-cell-1-silo-1' "
        "AND availability_zone = 'us-east-1-1' AND microservice_name = "
        "'apollo' AND instance_type = 'r5.4xlarge' AND os_version = 'AL2' AND "
        "instance_name = 'i-AUa00Zt2-apollo-0000.amazonaws.com' GROUP BY "
        "BIN(time, 1m) ORDER BY time_bin desc LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q2_EXPECT_1_ROW,
    CREATE_STRING(
        "SELECT * FROM perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() "
        "- 100y AND now() AND measure_name = 'memory_free' AND region = "
        "'us-east-1' AND cell = 'us-east-1-cell-1' AND silo = "
        "'us-east-1-cell-1-silo-1' AND availability_zone = 'us-east-1-1' AND "
        "microservice_name = 'apollo' AND instance_name = "
        "'i-AUa00Zt2-apollo-0000.amazonaws.com' AND process_name = 'server' "
        "AND jdk_version = 'JDK_11' ORDER BY time DESC LIMIT 1"),
    true)

TEST_PERF_TEST(
    Q3_EXPECT_2_ROWS,
    CREATE_STRING(
        "SELECT BIN(time, 1h) AS hour, COUNT(*) AS num_samples, "
        "ROUND(AVG(measure_value::bigint), 2) AS avg_value, "
        "ROUND(APPROX_PERCENTILE(measure_value::bigint, 0.9), 2) AS p90_value, "
        "ROUND(APPROX_PERCENTILE(measure_value::bigint, 0.95), 2) AS "
        "p95_value, ROUND(APPROX_PERCENTILE(measure_value::bigint, 0.99), 2) "
        "AS p99_value FROM perfdb_hcltps.perftable_hcltps WHERE time BETWEEN "
        "now() - 100y AND now() AND region = "
        "'us-east-1' AND cell = 'us-east-1-cell-1' AND silo = "
        "'us-east-1-cell-1-silo-1' AND availability_zone = 'us-east-1-1' AND "
        "microservice_name = 'apollo' AND instance_type = 'r5.4xlarge' AND "
        "os_version = 'AL2' AND instance_name = "
        "'i-AUa00Zt2-apollo-0000.amazonaws.com' GROUP BY BIN(time, 1h) ORDER "
        "BY hour desc LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q4_EXPECT_1_ROWS,
    CREATE_STRING(
        "WITH gc_timeseries AS ( SELECT region, cell, silo, availability_zone, "
        "microservice_name, instance_name, process_name, jdk_version, "
        "CREATE_TIME_SERIES(time, measure_value::double) AS gc_reclaimed, "
        "MIN(time) AS min_time, MAX(time) AS max_time FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() AND measure_name = 'gc_reclaimed' AND region = 'us-east-1' AND "
        "cell = 'us-east-1-cell-1' AND silo = 'us-east-1-cell-1-silo-1' AND "
        "availability_zone = 'us-east-1-1' AND microservice_name = 'apollo' "
        "AND jdk_version = 'JDK_11' GROUP BY region, "
        "cell, silo, availability_zone, microservice_name, instance_name, "
        "process_name, jdk_version), interpolated_ts AS ( SELECT "
        "INTERPOLATE_LOCF(gc_reclaimed, SEQUENCE(min_time, max_time, 1s)) AS "
        "interpolated_gc_reclaimed FROM gc_timeseries) SELECT "
        "FILTER(interpolated_gc_reclaimed, x -> x.value > 50) AS "
        "gc_reclaimed_above_threshold, ROUND(REDUCE(interpolated_gc_reclaimed, "
        "CAST(ROW(0, 0) AS ROW(count_high BIGINT, count_total BIGINT)), (s, x) "
        "-> CAST(ROW(s.count_high + IF(x.value > 50, 1, 0), s.count_total + 1) "
        "AS ROW(count_high BIGINT, count_total BIGINT)), s -> IF(s.count_total "
        "= 0, NULL, CAST(s.count_high AS DOUBLE) / s.count_total)), 4) AS "
        "fraction_gc_reclaimed_threshold FROM interpolated_ts LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q5_EXPECT_2_ROWS,
    CREATE_STRING(
        "SELECT instance_name, BIN(time, 1h) AS time_bin, COUNT(*) AS "
        "num_samples, AVG(measure_value::double) AS avg_memory_free, "
        "ROUND(APPROX_PERCENTILE(measure_value::double, 0.9), 2) AS "
        "p90_memory_free, ROUND(APPROX_PERCENTILE(measure_value::double, "
        "0.95), 2) AS p95_memory_free, "
        "ROUND(APPROX_PERCENTILE(measure_value::double, 0.99), 2) AS "
        "p99_memory_free FROM perfdb_hcltps.perftable_hcltps WHERE time "
        "BETWEEN now() - 100y AND now() AND measure_name = 'memory_free' AND "
        "region = 'us-east-1' AND cell = 'us-east-1-cell-1' AND silo = "
        "'us-east-1-cell-1-silo-1' AND availability_zone = 'us-east-1-1' AND "
        "microservice_name = 'apollo' AND instance_name = "
        "'i-AUa00Zt2-apollo-0000.amazonaws.com' AND process_name = 'server' "
        "AND jdk_version = 'JDK_11' GROUP BY instance_name, BIN(time, 1h) "
        "LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q6_EXPECT_1_ROW,
    CREATE_STRING(
        "WITH event_interval AS ( SELECT instance_name, process_name, "
        "jdk_version, to_milliseconds(time - LAG(time, 1) OVER (ORDER BY time "
        "ASC)) AS interval FROM perfdb_hcltps.perftable_hcltps WHERE time "
        "BETWEEN now() - 100y AND now() AND measure_name = 'gc_reclaimed' AND "
        "region = 'us-east-1' AND cell = 'us-east-1-cell-1' AND silo = "
        "'us-east-1-cell-1-silo-1' AND availability_zone = 'us-east-1-1' AND "
        "microservice_name = 'apollo' "
        "AND process_name = 'server' "
        "AND jdk_version = 'JDK_11') SELECT instance_name, process_name, "
        "jdk_version, COUNT(*) AS num_events, ROUND(MIN(interval), 2) AS "
        "min_interval, ROUND(AVG(interval), 2) AS avg_interval, "
        "ROUND(MAX(interval), 2) AS max_interval, "
        "ROUND(APPROX_PERCENTILE(interval, 0.5), 2) AS p50_interval, "
        "ROUND(APPROX_PERCENTILE(interval, 0.9), 2) AS p90_interval, "
        "ROUND(APPROX_PERCENTILE(interval, 0.99), 2) AS p99_interval FROM "
        "event_interval WHERE interval IS NOT NULL GROUP BY instance_name, "
        "process_name, jdk_version LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q7_EXPECT_49_ROWS,
    CREATE_STRING(
        "SELECT BIN(time, 1m) AS time_bin, AVG(CASE WHEN measure_name = "
        "'cpu_user' THEN measure_value::double ELSE NULL END) AS avg_cpu_user, "
        "AVG(CASE WHEN measure_name = 'cpu_system' THEN measure_value::double "
        "ELSE NULL END) AS avg_cpu_system, AVG(CASE WHEN measure_name = "
        "'cpu_idle' THEN measure_value::double ELSE NULL END) AS avg_cpu_idle, "
        "AVG(CASE WHEN measure_name = 'cpu_iowait' THEN measure_value::double "
        "ELSE NULL END) AS avg_cpu_iowait, AVG(CASE WHEN measure_name = "
        "'cpu_steal' THEN measure_value::double ELSE NULL END) AS "
        "avg_cpu_steal, AVG(CASE WHEN measure_name = 'cpu_nice' THEN "
        "measure_value::double ELSE NULL END) AS avg_cpu_nice, AVG(CASE WHEN "
        "measure_name = 'cpu_si' THEN measure_value::double ELSE NULL END) AS "
        "avg_cpu_si, AVG(CASE WHEN measure_name = 'cpu_hi' THEN "
        "measure_value::double ELSE NULL END) AS avg_cpu_hi FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() AND measure_name IN ( 'cpu_user', 'cpu_system', 'cpu_idle', "
        "'cpu_iowait', 'cpu_steal', 'cpu_nice', 'cpu_si', 'cpu_hi') AND region "
        "= 'us-east-1' AND cell = 'us-east-1-cell-1' AND silo = "
        "'us-east-1-cell-1-silo-1' AND availability_zone = 'us-east-1-1' AND "
        "microservice_name = 'apollo' AND instance_type = 'r5.4xlarge' AND "
        "os_version = 'AL2' GROUP BY BIN(time, 1m) ORDER "
        "BY time_bin desc LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q8_EXPECT_49_ROWS,
    CREATE_STRING(
        "WITH cpu_user AS ( SELECT BIN(time, 1m) AS time_bin, "
        "AVG(measure_value::double) AS cpu_used FROM "
        "perfdb_hcltps.perftable_hcltps GROUP BY "
        "BIN(time, 1m)), memory_used AS ( SELECT BIN(time, 1m) AS time_bin, "
        "AVG(measure_value::double) AS mem_used FROM "
        "perfdb_hcltps.perftable_hcltps GROUP BY "
        "BIN(time, 1m)) SELECT mu.time_bin, IF(mu.mem_used > cu.cpu_used, "
        "'memory', 'cpu') AS bottleneck_resource FROM memory_used mu INNER "
        "JOIN cpu_user cu ON mu.time_bin = cu.time_bin ORDER BY mu.time_bin "
        "DESC LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q9_EXPECT_2_ROWS,
    CREATE_STRING(
        "SELECT BIN(time, 1h) AS hour, COUNT(CASE WHEN measure_name = "
        "'cpu_user' THEN measure_value::double ELSE NULL END) AS "
        "num_cpu_user_samples, ROUND(AVG(CASE WHEN measure_name = 'cpu_user' "
        "THEN measure_value::double ELSE NULL END), 2) AS avg_cpu_user, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'cpu_user' THEN "
        "measure_value::double ELSE NULL END, 0.9), 2) AS p90_cpu_user, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'cpu_user' THEN "
        "measure_value::double ELSE NULL END, 0.95), 2) AS p95_cpu_user, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'cpu_user' THEN "
        "measure_value::double ELSE NULL END, 0.99), 2) AS p99_cpu_user, "
        "COUNT(CASE WHEN measure_name = 'cpu_system' THEN "
        "measure_value::double ELSE NULL END) AS num_cpu_system_samples, "
        "ROUND(AVG(CASE WHEN measure_name = 'cpu_system' THEN "
        "measure_value::double ELSE NULL END), 2) AS avg_cpu_system, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'cpu_system' THEN "
        "measure_value::double ELSE NULL END, 0.9), 2) AS p90_cpu_system, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'cpu_system' THEN "
        "measure_value::double ELSE NULL END, 0.95), 2) AS p95_cpu_system, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'cpu_system' THEN "
        "measure_value::double ELSE NULL END, 0.99), 2) AS p99_cpu_system, "
        "COUNT(CASE WHEN measure_name = 'memory_used' THEN "
        "measure_value::double ELSE NULL END) AS num_memory_used_samples, "
        "ROUND(AVG(CASE WHEN measure_name = 'memory_used' THEN "
        "measure_value::double ELSE NULL END), 2) AS avg_memory_used, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'memory_used' THEN "
        "measure_value::double ELSE NULL END, 0.9), 2) AS p90_memory_used, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'memory_used' THEN "
        "measure_value::double ELSE NULL END, 0.95), 2) AS p95_memory_used, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'memory_used' THEN "
        "measure_value::double ELSE NULL END, 0.99), 2) AS p99_memory_used, "
        "COUNT(CASE WHEN measure_name = 'disk_io_reads' THEN "
        "measure_value::bigint ELSE NULL END) AS num_disk_io_reads_samples, "
        "ROUND(AVG(CASE WHEN measure_name = 'disk_io_reads' THEN "
        "measure_value::bigint ELSE NULL END), 2) AS avg_disk_io_reads, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'disk_io_reads' THEN "
        "measure_value::bigint ELSE NULL END, 0.9), 2) AS p90_disk_io_reads, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'disk_io_reads' THEN "
        "measure_value::bigint ELSE NULL END, 0.95), 2) AS p95_disk_io_reads, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'disk_io_reads' THEN "
        "measure_value::bigint ELSE NULL END, 0.99), 2) AS p99_disk_io_reads, "
        "COUNT(CASE WHEN measure_name = 'disk_io_writes' THEN "
        "measure_value::bigint ELSE NULL END) AS num_disk_io_writes_samples, "
        "ROUND(AVG(CASE WHEN measure_name = 'disk_io_writes' THEN "
        "measure_value::bigint ELSE NULL END), 2) AS avg_disk_io_writes, "
        "ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = 'disk_io_writes' "
        "THEN measure_value::bigint ELSE NULL END, 0.9), 2) AS "
        "p90_disk_io_writes, ROUND(APPROX_PERCENTILE(CASE WHEN measure_name = "
        "'disk_io_writes' THEN measure_value::bigint ELSE NULL END, 0.95), 2) "
        "AS p95_disk_io_writes, ROUND(APPROX_PERCENTILE(CASE WHEN measure_name "
        "= 'disk_io_writes' THEN measure_value::bigint ELSE NULL END, 0.99), "
        "2) AS p99_disk_io_writes FROM perfdb_hcltps.perftable_hcltps WHERE "
        "time BETWEEN now() - 100y AND now() AND measure_name IN ( 'cpu_user', "
        "'cpu_system', 'memory_used', 'disk_io_reads', 'disk_io_writes') AND "
        "region = 'us-east-1' AND cell = 'us-east-1-cell-1' AND silo = "
        "'us-east-1-cell-1-silo-1' AND availability_zone = 'us-east-1-1' AND "
        "microservice_name = 'apollo' AND instance_type = 'r5.4xlarge' AND "
        "os_version = 'AL2' GROUP BY BIN(time, 1h) ORDER "
        "BY hour DESC LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q10_EXPECT_2_ROWS,
    CREATE_STRING(
        "WITH cpu_user AS ( SELECT instance_name, time, measure_value::double "
        "AS cpu_user FROM perfdb_hcltps.perftable_hcltps WHERE time BETWEEN "
        "now() - 100y AND now() AND measure_name = 'cpu_user' AND region = "
        "'us-east-1' AND cell = 'us-east-1-cell-1' AND silo = "
        "'us-east-1-cell-1-silo-1' AND availability_zone = 'us-east-1-1' AND "
        "microservice_name = 'apollo' AND instance_type = 'r5.4xlarge' AND "
        "os_version = 'AL2'), cpu_system AS ( SELECT "
        "instance_name, time, measure_value::double AS cpu_system FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() AND measure_name = 'cpu_system' AND region = 'us-east-1' AND "
        "cell = 'us-east-1-cell-1' AND silo = 'us-east-1-cell-1-silo-1' AND "
        "availability_zone = 'us-east-1-1' AND microservice_name = 'apollo' "
        "AND instance_type = 'r5.4xlarge' AND os_version = 'AL2'), memory_used "
        "AS ( SELECT instance_name, time, measure_value::double AS memory_used "
        "FROM perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y "
        "AND "
        "now() AND measure_name = 'memory_used' AND region = 'us-east-1' AND "
        "cell = 'us-east-1-cell-1' AND silo = 'us-east-1-cell-1-silo-1' AND "
        "availability_zone = 'us-east-1-1' AND microservice_name = 'apollo' "
        "AND instance_type = 'r5.4xlarge' AND os_version = 'AL2'), "
        "gc_reclaimed_bins AS ( SELECT instance_name, BIN(time, 1h) AS "
        "time_bin, AVG(measure_value::double) AS gc_reclaimed FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() AND measure_name = 'gc_reclaimed' AND region = 'us-east-1' AND "
        "cell = 'us-east-1-cell-1' AND silo = 'us-east-1-cell-1-silo-1' AND "
        "availability_zone = 'us-east-1-1' AND microservice_name = 'apollo' "
        "AND process_name = 'server' AND jdk_version = 'JDK_11' GROUP BY "
        "instance_name, BIN(time, 1h)), high_utilization_bins AS ( SELECT "
        "cu.instance_name, BIN(cu.time, 1h) AS time_bin, avg(cpu_user + "
        "cpu_system) AS avg_cpu, max(cpu_user + cpu_system) AS max_cpu, "
        "avg(memory_used) AS avg_memory, max(memory_used) AS max_memory FROM "
        "cpu_user cu INNER JOIN cpu_system cs ON cu.instance_name = "
        "cs.instance_name AND cu.time = cs.time INNER JOIN memory_used mu ON "
        "mu.instance_name = cs.instance_name AND mu.time = cs.time WHERE "
        "cpu_user + cpu_system > 0 AND memory_used > 0 GROUP BY "
        "cu.instance_name, BIN(cu.time, 1h)) SELECT hu.time_bin, gc_reclaimed, "
        "avg_cpu, max_cpu, avg_memory, max_memory FROM gc_reclaimed_bins gc "
        "INNER JOIN high_utilization_bins hu ON gc.instance_name = "
        "hu.instance_name AND gc.time_bin = hu.time_bin ORDER BY hu.time_bin "
        "DESC LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q11_EXPECT_441_ROWS,
    CREATE_STRING(
        "SELECT region, cell, silo, availability_zone, microservice_name, "
        "BIN(time, 1m) AS time_bin, COUNT(DISTINCT instance_name) AS "
        "num_hosts, ROUND(AVG(measure_value::double), 2) AS avg_value, "
        "ROUND(APPROX_PERCENTILE(measure_value::double, 0.9), 2) AS p90_value, "
        "ROUND(APPROX_PERCENTILE(measure_value::double, 0.95), 2) AS "
        "p95_value, ROUND(APPROX_PERCENTILE(measure_value::double, 0.99), 2) "
        "AS p99_value FROM perfdb_hcltps.perftable_hcltps WHERE time BETWEEN "
        "now() - 100y AND now() AND measure_name = 'cpu_user' AND region = "
        "'us-east-1' AND cell = 'us-east-1-cell-1' AND microservice_name = "
        "'apollo' GROUP BY region, cell, silo, availability_zone, "
        "microservice_name, BIN(time, 1m) ORDER BY p99_value DESC LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q12_EXPECT_410_ROWS,
    CREATE_STRING(
        "SELECT region, cell, microservice_name, BIN(time, 1h) AS hour, "
        "COUNT(DISTINCT instance_name) AS num_hosts, "
        "ROUND(AVG(measure_value::double), 2) AS avg_value, "
        "ROUND(APPROX_PERCENTILE(measure_value::double, 0.9), 2) AS p90_value, "
        "ROUND(APPROX_PERCENTILE(measure_value::double, 0.95), 2) AS "
        "p95_value, ROUND(APPROX_PERCENTILE(measure_value::double, 0.99), 2) "
        "AS p99_value FROM perfdb_hcltps.perftable_hcltps WHERE time BETWEEN "
        "now() - 100y AND now() AND measure_name = 'cpu_user' GROUP BY region, "
        "cell, microservice_name, BIN(time, 1h) ORDER BY p99_value DESC LIMIT "
        "10000"),
    true)

TEST_PERF_TEST(
    Q13_EXPECT_441_ROWS,
    CREATE_STRING(
        "SELECT region, cell, silo, availability_zone, microservice_name, "
        "BIN(time, 1m) AS time_bin, COUNT(DISTINCT instance_name) AS "
        "num_hosts, ROUND(AVG(measure_value::double), 2) AS avg_value, "
        "ROUND(APPROX_PERCENTILE(measure_value::double, 0.9), 2) AS p90_value, "
        "ROUND(APPROX_PERCENTILE(measure_value::double, 0.95), 2) AS "
        "p95_value, ROUND(APPROX_PERCENTILE(measure_value::double, 0.99), 2) "
        "AS p99_value FROM perfdb_hcltps.perftable_hcltps WHERE time BETWEEN "
        "now() - 100y AND now() AND measure_name = 'cpu_user' AND region = "
        "'us-east-1' AND cell = 'us-east-1-cell-1' AND microservice_name = "
        "'apollo' GROUP BY region, cell, silo, availability_zone, "
        "microservice_name, BIN(time, 1m) ORDER BY p99_value DESC LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q14_EXPECT_10_ROWS,
    CREATE_STRING(
        "WITH per_host_timeseries AS ( SELECT region, cell, silo, "
        "availability_zone, microservice_name, instance_name, process_name, "
        "jdk_version, CREATE_TIME_SERIES(time, measure_value::double) AS "
        "memory_free, MIN(time) AS min_time, MAX(time) AS max_time FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() AND measure_name = 'memory_free' AND region = 'us-east-1' AND "
        "cell = 'us-east-1-cell-1' AND process_name IS NOT NULL GROUP BY "
        "region, cell, silo, availability_zone, microservice_name, "
        "instance_name, process_name, jdk_version), overall_min_max AS ( "
        "SELECT MAX(min_time) AS min_time, MIN(max_time) AS max_time FROM "
        "per_host_timeseries), interpolated_timeseries AS ( SELECT region, "
        "cell, microservice_name, INTERPOLATE_LINEAR(memory_free, "
        "SEQUENCE(o.min_time, o.max_time, 15s)) AS interpolated_memory_free "
        "FROM per_host_timeseries p CROSS JOIN overall_min_max o) SELECT "
        "region, cell, microservice_name, BIN(time, 1h) AS time_bin, "
        "COUNT(memory_free) AS num_samples, AVG(memory_free) AS "
        "avg_memory_free, ROUND(APPROX_PERCENTILE(memory_free, 0.9), 2) AS "
        "p90_memory_free, ROUND(APPROX_PERCENTILE(memory_free, 0.95), 2) AS "
        "p95_memory_free, ROUND(APPROX_PERCENTILE(memory_free, 0.99), 2) AS "
        "p99_memory_free FROM interpolated_timeseries CROSS JOIN "
        "UNNEST(interpolated_memory_free) AS t(time, memory_free) GROUP BY "
        "region, cell, microservice_name, BIN(time, 1h) ORDER BY "
        "p95_memory_free DESC LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q15_EXPECT_141_ROWS,
    CREATE_STRING(
        "WITH microservice_cell_avg AS ( SELECT region, cell, "
        "microservice_name, AVG(measure_value::double) AS "
        "microservice_avg_metric FROM perfdb_hcltps.perftable_hcltps WHERE "
        "time BETWEEN now() - 100y AND now() AND measure_name = 'cpu_user' AND "
        "microservice_name = 'apollo' GROUP BY region, cell, "
        "microservice_name), instance_avg AS ( SELECT region, cell, silo, "
        "availability_zone, microservice_name, instance_name, "
        "AVG(measure_value::double) AS instance_avg_metric FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() AND measure_name = 'cpu_user' AND microservice_name = 'apollo' "
        "GROUP BY region, cell, silo, availability_zone, microservice_name, "
        "instance_name) SELECT i.*, m.microservice_avg_metric FROM "
        "microservice_cell_avg m INNER JOIN instance_avg i ON i.region = "
        "m.region AND i.cell = m.cell AND i.microservice_name = "
        "m.microservice_name WHERE i.instance_avg_metric > (1 + 0) * "
        "m.microservice_avg_metric ORDER BY i.instance_avg_metric DESC LIMIT "
        "10000"),
    true)

TEST_PERF_TEST(
    Q16_EXPECT_505_ROWS,
    CREATE_STRING(
        "WITH per_instance_max_use AS ( SELECT region, cell, silo, "
        "availability_zone, microservice_name, instance_name, BIN(time, 15m) "
        "AS time_bin, MAX(CASE WHEN measure_name = 'cpu_user' THEN "
        "measure_value::double ELSE NULL END) AS max_cpu_user, MAX(CASE WHEN "
        "measure_name = 'memory_used' THEN measure_value::double ELSE NULL "
        "END) AS max_memory_used FROM perfdb_hcltps.perftable_hcltps WHERE "
        "time BETWEEN now() - 100y AND now() AND measure_name IN ('cpu_user', "
        "'memory_used') GROUP BY region, cell, silo, availability_zone, "
        "microservice_name, instance_name, BIN(time, 15m)) SELECT region, "
        "cell, silo, microservice_name, BIN(time_bin, 1d) AS day, "
        "COUNT(max_cpu_user) AS num_samples, MIN(max_cpu_user) AS min_max_cpu, "
        "AVG(max_cpu_user) AS avg_max_cpu, MAX(max_cpu_user) AS max_max_cpu, "
        "ROUND(ROUND(APPROX_PERCENTILE(max_cpu_user, 0.25), 2)) AS "
        "p25_max_cpu, ROUND(ROUND(APPROX_PERCENTILE(max_cpu_user, 0.50), 2)) "
        "AS p50_max_cpu, ROUND(ROUND(APPROX_PERCENTILE(max_cpu_user, 0.75), "
        "2)) AS p75_max_cpu, ROUND(ROUND(APPROX_PERCENTILE(max_cpu_user, "
        "0.95), 2)) AS p95_max_cpu, "
        "ROUND(ROUND(APPROX_PERCENTILE(max_cpu_user, 0.99), 2)) AS "
        "p99_max_cpu, MIN(max_memory_used) AS min_max_memory, "
        "AVG(max_memory_used) AS avg_max_memory, MAX(max_memory_used) AS "
        "max_max_memory, ROUND(ROUND(APPROX_PERCENTILE(max_memory_used, 0.25), "
        "2)) AS p25_max_memory, ROUND(ROUND(APPROX_PERCENTILE(max_memory_used, "
        "0.50), 2)) AS p50_max_memory, "
        "ROUND(ROUND(APPROX_PERCENTILE(max_memory_used, 0.75), 2)) AS "
        "p75_max_memory, ROUND(ROUND(APPROX_PERCENTILE(max_memory_used, 0.95), "
        "2)) AS p95_max_memory, ROUND(ROUND(APPROX_PERCENTILE(max_memory_used, "
        "0.99), 2)) AS p99_max_memory FROM per_instance_max_use GROUP BY "
        "region, cell, silo, microservice_name, BIN(time_bin, 1d) ORDER BY "
        "p95_max_cpu DESC LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q17_EXPECT_236_ROWS,
    CREATE_STRING(
        "WITH per_instance_memory_used AS ( SELECT region, cell, silo, "
        "availability_zone, microservice_name, instance_name, BIN(time, 5m) "
        "AS time_bin, MAX(measure_value::double) AS max_memory FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() AND measure_name = 'memory_used' GROUP BY region, cell, silo, "
        "availability_zone, microservice_name, instance_name, BIN(time, 5m)), "
        "per_microservice_memory AS ( SELECT region, cell, silo, "
        "microservice_name, APPROX_PERCENTILE(max_memory, 0.95) AS "
        "p95_max_memory FROM per_instance_memory_used GROUP BY region, cell, "
        "silo, microservice_name), per_silo_ranked AS ( SELECT region, cell, "
        "silo, microservice_name, DENSE_RANK() OVER (PARTITION BY region, "
        "cell, silo ORDER BY p95_max_memory DESC) AS rank FROM "
        "per_microservice_memory), instances_with_high_memory AS ( SELECT "
        "r.region, r.cell, r.silo, r.microservice_name, m.instance_name, "
        "APPROX_PERCENTILE(max_memory, 0.95) AS p95_max_memory FROM "
        "per_silo_ranked r INNER JOIN per_instance_memory_used m ON r.region "
        "= m.region AND r.cell = m.cell AND r.silo = m.silo AND "
        "r.microservice_name = m.microservice_name WHERE r.rank = 1 GROUP BY "
        "r.region, r.cell, r.silo, r.microservice_name, m.instance_name), "
        "ranked_instances AS ( SELECT region, cell, silo, microservice_name, "
        "instance_name, DENSE_RANK() OVER (PARTITION BY region, cell, silo, "
        "microservice_name ORDER BY p95_max_memory DESC) AS rank FROM "
        "instances_with_high_memory) SELECT t.region, t.cell, t.silo, "
        "t.microservice_name, t.instance_name, t.process_name, t.jdk_version, "
        "COUNT(*) AS num_samples, MIN(measure_value::double) AS min_gc_pause, "
        "ROUND(AVG(measure_value::double), 2) AS avg_gc_pause, "
        "ROUND(STDDEV(measure_value::double), 2) AS stddev_gc_pause, "
        "ROUND(APPROX_PERCENTILE(measure_value::double, 0.5), 2) AS "
        "p50_gc_pause, ROUND(APPROX_PERCENTILE(measure_value::double, 0.9), "
        "2) AS p90_gc_pause, ROUND(APPROX_PERCENTILE(measure_value::double, "
        "0.99), 2) AS p99_gc_pause FROM ranked_instances r INNER JOIN "
        "perfdb_hcltps.perftable_hcltps t ON r.region = t.region AND r.cell = "
        "t.cell AND r.silo = t.silo AND r.microservice_name = "
        "t.microservice_name AND r.instance_name = t.instance_name WHERE time "
        "BETWEEN now() - 100y AND now() AND measure_name = 'gc_pause' AND rank "
        "<= 10 GROUP BY t.region, t.cell, t.silo, t.microservice_name, "
        "t.instance_name, t.process_name, t.jdk_version LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q18_EXPECT_410_ROWS,
    CREATE_STRING(
        "WITH per_instance_cpu_used AS ( SELECT region, cell, silo, "
        "availability_zone, microservice_name, instance_name, BIN(time, 5m) AS "
        "time_bin, AVG(measure_value::double) AS avg_cpu FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() AND measure_name = 'cpu_user' GROUP BY region, cell, silo, "
        "availability_zone, microservice_name, instance_name, BIN(time, 5m)), "
        "per_microservice_cpu AS ( SELECT region, cell, microservice_name, "
        "BIN(time_bin, 1h) AS hour, APPROX_PERCENTILE(avg_cpu, 0.95) AS "
        "p95_avg_cpu FROM per_instance_cpu_used GROUP BY region, cell, "
        "microservice_name, BIN(time_bin, 1h)), per_microservice_ranked AS ( "
        "SELECT region, cell, microservice_name, hour, p95_avg_cpu, "
        "DENSE_RANK() OVER (PARTITION BY region, cell, microservice_name ORDER "
        "BY p95_avg_cpu DESC) AS rank FROM per_microservice_cpu) SELECT "
        "region, cell, microservice_name, hour AS hour, p95_avg_cpu FROM "
        "per_microservice_ranked WHERE rank <= 5 ORDER BY region, cell, "
        "microservice_name, rank ASC LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q19_EXPECT_3027_ROWS,
    CREATE_STRING(
        "WITH task_completed AS ( SELECT region, cell, silo, "
        "availability_zone, microservice_name, instance_name, process_name, "
        "jdk_version, time, measure_value::bigint AS task_completed FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() AND measure_name = 'task_completed'), task_end_states AS ( "
        "SELECT region, cell, silo, availability_zone, microservice_name, "
        "instance_name, process_name, jdk_version, time, "
        "measure_value::varchar AS task_end_state FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() AND measure_name = 'task_end_state') SELECT tc.region, "
        "tc.cell, tc.silo, tc.microservice_name, tes.task_end_state, "
        "COUNT(task_completed) AS num_tasks, MIN(task_completed) AS "
        "min_task_completed, ROUND(AVG(task_completed), 2) AS "
        "avg_task_completed, MAX(task_completed) AS max_task_completed, "
        "ROUND(APPROX_PERCENTILE(task_completed, 0.5), 2) AS "
        "p50_task_completed, ROUND(APPROX_PERCENTILE(task_completed, 0.9), 2) "
        "AS p90_task_completed, ROUND(APPROX_PERCENTILE(task_completed, "
        "0.99), 2) AS p99_task_completed FROM task_completed tc INNER JOIN "
        "task_end_states tes ON tc.region = tes.region AND tc.cell = tes.cell "
        "AND tc.silo = tes.silo AND tc.availability_zone = "
        "tes.availability_zone AND tc.microservice_name = "
        "tes.microservice_name AND tc.instance_name = tes.instance_name AND "
        "tc.process_name = tes.process_name AND tc.jdk_version = "
        "tes.jdk_version AND tc.time = tes.time GROUP BY tc.region, tc.cell, "
        "tc.silo, tc.microservice_name, tes.task_end_state ORDER BY "
        "tc.region, tc.cell, tc.silo, tc.microservice_name, "
        "tes.task_end_state LIMIT 10000"),
    true)

TEST_PERF_TEST(
    Q20_EXPECT_141_ROWS,
    CREATE_STRING(
        "WITH microservice_cell_avg AS ( SELECT region, cell, "
        "microservice_name, AVG(measure_value::double) AS "
        "microservice_avg_metric FROM perfdb_hcltps.perftable_hcltps WHERE "
        "time BETWEEN now() - 100y AND now() AND measure_name = 'cpu_user' AND "
        "microservice_name = 'apollo' GROUP BY region, cell, "
        "microservice_name), instance_avg AS ( SELECT region, cell, silo, "
        "availability_zone, microservice_name, instance_name, "
        "AVG(measure_value::double) AS instance_avg_metric FROM "
        "perfdb_hcltps.perftable_hcltps WHERE time BETWEEN now() - 100y AND "
        "now() "
        "AND "
        "measure_name = 'cpu_user' AND microservice_name = 'apollo' GROUP BY "
        "region, cell, silo, availability_zone, microservice_name, "
        "instance_name) SELECT i.*, m.microservice_avg_metric FROM "
        "microservice_cell_avg m INNER JOIN instance_avg i ON i.region = "
        "m.region AND i.cell = m.cell AND i.microservice_name = "
        "m.microservice_name WHERE i.instance_avg_metric > (1 + 0) * "
        "m.microservice_avg_metric ORDER BY i.instance_avg_metric DESC LIMIT "
        "10000"),
    true)

TEST_PERF_TEST(
    Q21_EXPECT_15000_ROWS,
    CREATE_STRING("SELECT * FROM perfdb_hcltps.perftable_hcltps LIMIT 15000"),
    true)

TEST_PERF_TEST(
    Q22_EXPECT_1500000_ROWS,
    CREATE_STRING("SELECT * FROM perfdb_hcltps.perftable_hcltps LIMIT 1500000"),
    true)

int main(int argc, char** argv) {
#ifdef WIN32
  // Enable CRT for detecting memory leaks
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#ifdef __APPLE__
  // Enable malloc logging for detecting memory leaks.
  system("export MallocStackLogging=1");
#endif
  std::string secretKey; /*$*/
  std::string region = "us-west-2";
  std::string defaultConnStr = "DSN=trino-iam;";
  std::string specialConnStr;

  // Handle passed in arguments
  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "--large-test") == 0) {
        enableLargeTest = true;
      } else if (strcmp(argv[i], "--secret-key") == 0 && i + 1 < argc) {
        secretKey = std::string(argv[i + 1]); /*$*/
        i++;
      } else if (strcmp(argv[i], "--region") == 0 && i + 1 < argc) {
        region = std::string(argv[i + 1]);
        i++;
      } else {
        std::cout
            << "Invalid argument: " << std::string(argv[i]) << std::endl
            << "Valid arguments are:\n"
               "--large-test\t\t\t\t\t\t\tEnable the test that returns "
               "1,500,000 rows and extends the run time to ~11 hours.\n"
               "--region <region>\t\t\t\t\t\tThe region to use for testing. "
               "Optional, but if provided then the access key ID and secret "
               "key must also be provided. Defaults to us-west-2.\n"
               "--access-key-id <access key id> --secret-key <secret key>\tThe "
               "AWS access key ID and AWS secret key to use for tests. If "
               "either are passed as arguments both must be provided.\n";
        std::cout << "Exiting . . .\n";
        return 1;
      }
    }
  }

/*$*/
  // if (!accessKeyId.empty() && !secretKey.empty()) {
  // } else {
  //   connectionString =
  //       std::vector< SQLWCHAR >(defaultConnStr.begin(), defaultConnStr.end());
  // }

  prepareOutFile();

  testing::internal::CaptureStdout();
  ::testing::InitGoogleTest(&argc, argv);

  int failures = RUN_ALL_TESTS();

  std::string output = testing::internal::GetCapturedStdout();
  std::cout << output << std::endl;
  std::cout << (failures ? "Not all tests passed." : "All tests passed")
            << std::endl;

#ifdef __APPLE__
  // Disable malloc logging and report memory leaks
  system("unset MallocStackLogging");
  system("leaks performance_results > leaks_performance_results");
#endif
  return failures;
}
