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

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

#include "trino/odbc/config/connection_info.h"
#include "trino/odbc/system/odbc_constants.h"
#include "trino/odbc/utility.h"
#include "odbc_test_suite.h"
#include "test_utils.h"

using namespace trino;
using namespace trino_test;
using namespace trino::odbc;
using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct SqlGetInfoTestSuiteFixture : odbc::OdbcTestSuite {
  void CheckStrInfo(SQLSMALLINT type, const std::string& expectedValue) {
    SQLSMALLINT valLen = 0;

    // Get required length.
    std::string typeStr = odbc::config::ConnectionInfo::InfoTypeToString(type);
    SQLRETURN ret = SQLGetInfo(dbc, type, nullptr, 0, &valLen);
    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);

    // Note: length is in bytes, not characters.
    std::vector< SQLWCHAR > val((valLen / sizeof(SQLWCHAR)) + 1);
    ret = SQLGetInfo(dbc, type, val.data(), val.size() * sizeof(SQLWCHAR),
                     &valLen);

    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);

    std::string actualValue = utility::SqlWcharToString(val.data());
    BOOST_REQUIRE_MESSAGE(actualValue == expectedValue,
                          "\"" + actualValue + "\" != \"" + expectedValue
                              + "\". SQLGetInfo Type: " + typeStr);
  }

  void CheckIntInfo(SQLSMALLINT type, SQLUINTEGER expectedValue) {
    SQLUINTEGER val;
    SQLRETURN ret = SQLGetInfo(dbc, type, &val, 0, 0);

    std::string typeStr = odbc::config::ConnectionInfo::InfoTypeToString(type);
    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);
    BOOST_REQUIRE_MESSAGE(val == expectedValue,
                          std::to_string(val)
                              + " != " + std::to_string(expectedValue)
                              + ". SQLGetInfo Type: " + typeStr);
  }

  void CheckShortInfo(SQLSMALLINT type, SQLUSMALLINT expectedValue) {
    SQLUSMALLINT val;
    SQLRETURN ret = SQLGetInfo(dbc, type, &val, 0, 0);

    std::string typeStr = odbc::config::ConnectionInfo::InfoTypeToString(type);
    ODBC_FAIL_ON_ERROR1(ret, SQL_HANDLE_DBC, dbc, typeStr);
    BOOST_REQUIRE_MESSAGE(val == expectedValue,
                          std::to_string(val)
                              + " != " + std::to_string(expectedValue)
                              + ". SQLGetInfo Type: " + typeStr);
  }

  /**
   * Constructor.
   */
  SqlGetInfoTestSuiteFixture() {
  }

  /**
   * Destructor.
   */
  ~SqlGetInfoTestSuiteFixture() {
  }

  /**
   * Connect to the Trino server with the database name
   *
   * @param databaseName Database Name
   */
  void connectToTSServer() {
    std::string dsnConnectionString;
    CreateDsnConnectionStringForAWS(dsnConnectionString);
    Connect(dsnConnectionString);
  }
};

BOOST_FIXTURE_TEST_SUITE(SqlGetInfoTestSuite, SqlGetInfoTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestValues) {
  connectToTSServer();

#if defined(WIN32)
  CheckStrInfo(SQL_DRIVER_NAME, "trino.odbc.dll");
#elif defined(__APPLE__)
  CheckStrInfo(SQL_DRIVER_NAME, "libtrino-odbc.dylib");
#elif defined(__linux__)
  CheckStrInfo(SQL_DRIVER_NAME, "libtrino-odbc.so");
#endif
  CheckStrInfo(SQL_DBMS_NAME, "Amazon Trino");
  CheckStrInfo(SQL_DRIVER_ODBC_VER, "03.00");
  CheckStrInfo(SQL_DRIVER_VER, utility::GetFormatedDriverVersion());
  CheckStrInfo(SQL_ACCESSIBLE_TABLES, "N");
  CheckStrInfo(SQL_COLUMN_ALIAS, "Y");
  CheckStrInfo(SQL_IDENTIFIER_QUOTE_CHAR, "\"");
  CheckStrInfo(SQL_CATALOG_NAME_SEPARATOR, ".");
  CheckStrInfo(SQL_SPECIAL_CHARACTERS, "_");
  if (DATABASE_AS_SCHEMA) {
    CheckStrInfo(SQL_CATALOG_TERM, "");
    CheckStrInfo(SQL_CATALOG_NAME, "N");
  } else {
    CheckStrInfo(SQL_CATALOG_TERM, "database");
    CheckStrInfo(SQL_CATALOG_NAME, "Y");
  }
  CheckStrInfo(SQL_TABLE_TERM, "table");
  if (DATABASE_AS_SCHEMA) {
    CheckStrInfo(SQL_SCHEMA_TERM, "schema");
  } else {
    CheckStrInfo(SQL_SCHEMA_TERM, "");
  }
  CheckStrInfo(SQL_NEED_LONG_DATA_LEN, "N");
  CheckStrInfo(SQL_ACCESSIBLE_PROCEDURES, "N");
  CheckStrInfo(SQL_ACCESSIBLE_TABLES, "N");
  CheckStrInfo(SQL_COLLATION_SEQ, "");
#if defined(__linux) || defined(__linux__) || defined(linux)
  // As we are connecting using SQLDriverConnect, it seems the driver is
  // removing the DSN setting
  CheckStrInfo(SQL_DATA_SOURCE_NAME, "");
#else
  CheckStrInfo(SQL_DATA_SOURCE_NAME, "Trino DSN");
#endif
  CheckStrInfo(SQL_DATA_SOURCE_READ_ONLY, "Y");
  CheckStrInfo(SQL_DATABASE_NAME, "");
  CheckStrInfo(SQL_DESCRIBE_PARAMETER, "N");
  CheckStrInfo(SQL_EXPRESSIONS_IN_ORDERBY, "Y");
  CheckStrInfo(SQL_INTEGRITY, "N");
  CheckStrInfo(SQL_KEYWORDS, "");
  CheckStrInfo(SQL_LIKE_ESCAPE_CLAUSE, "Y");
  CheckStrInfo(SQL_MAX_ROW_SIZE_INCLUDES_LONG, "Y");
  CheckStrInfo(SQL_MULT_RESULT_SETS, "N");
  CheckStrInfo(SQL_MULTIPLE_ACTIVE_TXN, "Y");
  CheckStrInfo(SQL_ORDER_BY_COLUMNS_IN_SELECT, "Y");
  CheckStrInfo(SQL_PROCEDURE_TERM, "");
  CheckStrInfo(SQL_PROCEDURES, "N");
  CheckStrInfo(SQL_ROW_UPDATES, "N");
  CheckStrInfo(SQL_SEARCH_PATTERN_ESCAPE, "");
  CheckStrInfo(SQL_SERVER_NAME, "AWS Trino");
  std::string expectedUserName =
      ignite::odbc::common::GetEnv("AWS_ACCESS_KEY_ID");
  CheckStrInfo(SQL_USER_NAME, expectedUserName);

  CheckIntInfo(SQL_ASYNC_MODE, SQL_AM_NONE);
  CheckIntInfo(SQL_BATCH_ROW_COUNT, 0);
  CheckIntInfo(SQL_BATCH_SUPPORT, 0);
  CheckIntInfo(SQL_BOOKMARK_PERSISTENCE, 0);
  if (DATABASE_AS_SCHEMA) {
    CheckIntInfo(SQL_CATALOG_LOCATION, 0);
    CheckIntInfo(SQL_CATALOG_USAGE, 0);
  } else {
    CheckIntInfo(SQL_CATALOG_LOCATION, SQL_CL_START);
    CheckIntInfo(SQL_CATALOG_USAGE, SQL_CU_DML_STATEMENTS);
  }
  CheckIntInfo(SQL_GETDATA_EXTENSIONS, SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER
                                           | SQL_GD_BOUND | SQL_GD_BLOCK);
  CheckIntInfo(SQL_ODBC_INTERFACE_CONFORMANCE, SQL_OIC_CORE);
  CheckIntInfo(SQL_SQL_CONFORMANCE, SQL_SC_SQL92_ENTRY);
  CheckIntInfo(SQL_TIMEDATE_ADD_INTERVALS,
               SQL_FN_TSI_FRAC_SECOND | SQL_FN_TSI_SECOND | SQL_FN_TSI_MINUTE
                   | SQL_FN_TSI_HOUR | SQL_FN_TSI_DAY | SQL_FN_TSI_WEEK
                   | SQL_FN_TSI_MONTH | SQL_FN_TSI_QUARTER | SQL_FN_TSI_YEAR);
  CheckIntInfo(SQL_TIMEDATE_DIFF_INTERVALS,
               SQL_FN_TSI_FRAC_SECOND | SQL_FN_TSI_SECOND | SQL_FN_TSI_MINUTE
                   | SQL_FN_TSI_HOUR | SQL_FN_TSI_DAY | SQL_FN_TSI_WEEK
                   | SQL_FN_TSI_MONTH | SQL_FN_TSI_QUARTER | SQL_FN_TSI_YEAR);
  CheckIntInfo(SQL_DATETIME_LITERALS, 0);
  CheckIntInfo(SQL_SYSTEM_FUNCTIONS, SQL_FN_SYS_IFNULL);
  CheckIntInfo(SQL_CONVERT_FUNCTIONS, SQL_FN_CVT_CAST);
  CheckIntInfo(SQL_OJ_CAPABILITIES, SQL_OJ_LEFT | SQL_OJ_NOT_ORDERED
                                        | SQL_OJ_RIGHT
                                        | SQL_OJ_ALL_COMPARISON_OPS);
  CheckIntInfo(SQL_POS_OPERATIONS, 0);
  CheckIntInfo(SQL_SQL92_DATETIME_FUNCTIONS, SQL_SDF_CURRENT_DATE
                                                 | SQL_SDF_CURRENT_TIME
                                                 | SQL_SDF_CURRENT_TIMESTAMP);
  CheckIntInfo(SQL_SQL92_VALUE_EXPRESSIONS, SQL_SVE_CASE | SQL_SVE_CAST);
  CheckIntInfo(SQL_STATIC_CURSOR_ATTRIBUTES1,
               SQL_CA1_NEXT | SQL_CA1_ABSOLUTE | SQL_CA1_RELATIVE
                   | SQL_CA1_BOOKMARK | SQL_CA1_LOCK_NO_CHANGE
                   | SQL_CA1_POS_POSITION | SQL_CA1_POS_REFRESH);
  CheckIntInfo(SQL_STATIC_CURSOR_ATTRIBUTES2,
               SQL_CA2_READ_ONLY_CONCURRENCY | SQL_CA2_CRC_EXACT);
  CheckIntInfo(SQL_PARAM_ARRAY_ROW_COUNTS, SQL_PARC_BATCH);
  CheckIntInfo(SQL_PARAM_ARRAY_SELECTS, SQL_PAS_NO_BATCH);
  CheckIntInfo(SQL_SCROLL_OPTIONS, SQL_SO_FORWARD_ONLY | SQL_SO_STATIC);
  CheckIntInfo(SQL_ALTER_DOMAIN, 0);
  CheckIntInfo(SQL_ALTER_TABLE, 0);
  CheckIntInfo(SQL_CREATE_ASSERTION, 0);
  CheckIntInfo(SQL_CREATE_CHARACTER_SET, 0);
  CheckIntInfo(SQL_CREATE_COLLATION, 0);
  CheckIntInfo(SQL_CREATE_DOMAIN, 0);
  CheckIntInfo(SQL_CREATE_SCHEMA, 0);
  CheckIntInfo(SQL_CREATE_TABLE, 0);
  CheckIntInfo(SQL_CREATE_TRANSLATION, 0);
  CheckIntInfo(SQL_CREATE_VIEW, 0);
#ifndef _WIN32
  CheckIntInfo(SQL_CURSOR_COMMIT_BEHAVIOR, SQL_CB_CLOSE);
  CheckIntInfo(SQL_CURSOR_ROLLBACK_BEHAVIOR, SQL_CB_PRESERVE);
#endif  // ifndef _WIN32
  CheckIntInfo(SQL_CURSOR_SENSITIVITY, SQL_INSENSITIVE);
  CheckIntInfo(SQL_DDL_INDEX, SQL_DI_CREATE_INDEX | SQL_DI_DROP_INDEX);
  CheckIntInfo(SQL_DEFAULT_TXN_ISOLATION, 0);
  CheckIntInfo(SQL_DROP_ASSERTION, 0);
  CheckIntInfo(SQL_DROP_CHARACTER_SET, 0);
  CheckIntInfo(SQL_DROP_COLLATION, 0);
  CheckIntInfo(SQL_DROP_DOMAIN, 0);
  CheckIntInfo(SQL_DROP_SCHEMA, 0);
  CheckIntInfo(SQL_DROP_TABLE, 0);
  CheckIntInfo(SQL_DROP_TRANSLATION, 0);
  CheckIntInfo(SQL_DROP_VIEW, 0);
  CheckIntInfo(SQL_DYNAMIC_CURSOR_ATTRIBUTES1, 0);
  CheckIntInfo(SQL_DYNAMIC_CURSOR_ATTRIBUTES2, 0);
  CheckIntInfo(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT);
  CheckIntInfo(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2,
               SQL_CA2_READ_ONLY_CONCURRENCY | SQL_CA2_CRC_EXACT);
  CheckIntInfo(SQL_INDEX_KEYWORDS, SQL_IK_NONE);
  CheckIntInfo(SQL_INFO_SCHEMA_VIEWS, 0);
  CheckIntInfo(SQL_INSERT_STATEMENT, 0);
  CheckIntInfo(SQL_KEYSET_CURSOR_ATTRIBUTES1, 0);
  CheckIntInfo(SQL_KEYSET_CURSOR_ATTRIBUTES2, 0);
  CheckIntInfo(SQL_MAX_ASYNC_CONCURRENT_STATEMENTS, 0);
  CheckIntInfo(SQL_MAX_BINARY_LITERAL_LEN, 0);
  CheckIntInfo(SQL_MAX_CATALOG_NAME_LEN, 0);
  CheckIntInfo(SQL_MAX_CHAR_LITERAL_LEN, 0);
  CheckIntInfo(SQL_MAX_INDEX_SIZE, 0);
  CheckIntInfo(SQL_MAX_ROW_SIZE, 0);
  CheckIntInfo(SQL_MAX_STATEMENT_LEN, 0);
  CheckIntInfo(SQL_SQL92_FOREIGN_KEY_DELETE_RULE, 0);
  CheckIntInfo(SQL_SQL92_FOREIGN_KEY_UPDATE_RULE, 0);
  CheckIntInfo(SQL_SQL92_GRANT, 0);
  CheckIntInfo(SQL_SQL92_REVOKE, 0);
  CheckIntInfo(SQL_STANDARD_CLI_CONFORMANCE, 0);
  CheckIntInfo(SQL_TXN_ISOLATION_OPTION, 0);
  CheckIntInfo(SQL_UNION, SQL_U_UNION | SQL_U_UNION_ALL);

  if (DATABASE_AS_SCHEMA) {
    CheckIntInfo(SQL_SCHEMA_USAGE,
                 SQL_SU_DML_STATEMENTS | SQL_SU_TABLE_DEFINITION
                     | SQL_SU_PRIVILEGE_DEFINITION | SQL_SU_INDEX_DEFINITION);
  } else {
    CheckIntInfo(SQL_SCHEMA_USAGE, 0);
  }

  CheckIntInfo(SQL_AGGREGATE_FUNCTIONS, SQL_AF_ALL);

  CheckIntInfo(SQL_NUMERIC_FUNCTIONS,
               SQL_FN_NUM_ABS | SQL_FN_NUM_ATAN | SQL_FN_NUM_ATAN2
                   | SQL_FN_NUM_COS | SQL_FN_NUM_COT | SQL_FN_NUM_DEGREES
                   | SQL_FN_NUM_FLOOR | SQL_FN_NUM_LOG | SQL_FN_NUM_LOG10
                   | SQL_FN_NUM_PI | SQL_FN_NUM_POWER | SQL_FN_NUM_RADIANS
                   | SQL_FN_NUM_ROUND | SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN
                   | SQL_FN_NUM_SQRT | SQL_FN_NUM_TAN);

  CheckIntInfo(SQL_STRING_FUNCTIONS, SQL_FN_STR_ASCII | SQL_FN_STR_LENGTH
                                         | SQL_FN_STR_LTRIM | SQL_FN_STR_REPLACE
                                         | SQL_FN_STR_RTRIM
                                         | SQL_FN_STR_SUBSTRING);

  CheckIntInfo(SQL_TIMEDATE_FUNCTIONS,
               SQL_FN_TD_CURDATE | SQL_FN_TD_DAYOFMONTH | SQL_FN_TD_MONTH
                   | SQL_FN_TD_MONTHNAME | SQL_FN_TD_NOW | SQL_FN_TD_YEAR);

  CheckIntInfo(SQL_SQL92_NUMERIC_VALUE_FUNCTIONS, 0);

  CheckIntInfo(SQL_SQL92_STRING_FUNCTIONS,
               SQL_SSF_CONVERT | SQL_SSF_LOWER | SQL_SSF_UPPER
                   | SQL_SSF_SUBSTRING | SQL_SSF_TRANSLATE | SQL_SSF_TRIM_BOTH
                   | SQL_SSF_TRIM_LEADING | SQL_SSF_TRIM_TRAILING);

  CheckIntInfo(SQL_SQL92_PREDICATES, SQL_SP_BETWEEN | SQL_SP_COMPARISON
                                         | SQL_SP_IN | SQL_SP_ISNULL
                                         | SQL_SP_LIKE);

  CheckIntInfo(SQL_SQL92_RELATIONAL_JOIN_OPERATORS,
               SQL_SRJO_CROSS_JOIN | SQL_SRJO_INNER_JOIN
                   | SQL_SRJO_LEFT_OUTER_JOIN | SQL_SRJO_RIGHT_OUTER_JOIN);

  CheckIntInfo(SQL_CONVERT_BIGINT, SQL_CVT_BIGINT | SQL_CVT_DOUBLE);
  CheckIntInfo(SQL_CONVERT_BINARY, 0);
  CheckIntInfo(SQL_CONVERT_BIT, 0);
  CheckIntInfo(SQL_CONVERT_CHAR, 0);
  CheckIntInfo(SQL_CONVERT_VARCHAR, SQL_CVT_VARCHAR);
  CheckIntInfo(SQL_CONVERT_LONGVARCHAR, 0);
  CheckIntInfo(SQL_CONVERT_WCHAR, 0);
  CheckIntInfo(SQL_CONVERT_WVARCHAR, 0);
  CheckIntInfo(SQL_CONVERT_WLONGVARCHAR, 0);
  CheckIntInfo(SQL_CONVERT_GUID, 0);
  CheckIntInfo(SQL_CONVERT_DATE, SQL_CVT_DATE);
  CheckIntInfo(SQL_CONVERT_DECIMAL, 0);
  CheckIntInfo(SQL_CONVERT_DOUBLE,
               SQL_CVT_INTEGER | SQL_CVT_BIGINT | SQL_CVT_DOUBLE);
  CheckIntInfo(SQL_CONVERT_FLOAT, 0);
  CheckIntInfo(SQL_CONVERT_REAL, 0);
  CheckIntInfo(SQL_CONVERT_INTEGER,
               SQL_CVT_INTEGER | SQL_CVT_BIGINT | SQL_CVT_DOUBLE);
  CheckIntInfo(SQL_CONVERT_NUMERIC, 0);
  CheckIntInfo(SQL_CONVERT_SMALLINT, 0);
  CheckIntInfo(SQL_CONVERT_TINYINT, 0);
  CheckIntInfo(SQL_CONVERT_TIME, SQL_CVT_TIME);
  CheckIntInfo(SQL_CONVERT_TIMESTAMP, SQL_CVT_TIMESTAMP);
  CheckIntInfo(SQL_CONVERT_INTERVAL_DAY_TIME, 0);
  CheckIntInfo(SQL_CONVERT_INTERVAL_YEAR_MONTH, 0);
  CheckIntInfo(SQL_CONVERT_VARBINARY, 0);
  CheckIntInfo(SQL_CONVERT_LONGVARBINARY, 0);

  CheckIntInfo(SQL_SQL92_ROW_VALUE_CONSTRUCTOR,
               SQL_SRVC_VALUE_EXPRESSION | SQL_SRVC_NULL);

  CheckIntInfo(SQL_SUBQUERIES, SQL_SQ_QUANTIFIED | SQL_SQ_IN | SQL_SQ_EXISTS
                                   | SQL_SQ_COMPARISON);

  CheckIntInfo(SQL_FETCH_DIRECTION,
               SQL_FD_FETCH_NEXT | SQL_FD_FETCH_FIRST | SQL_FD_FETCH_LAST
                   | SQL_FD_FETCH_PRIOR | SQL_FD_FETCH_ABSOLUTE
                   | SQL_FD_FETCH_RELATIVE | SQL_FD_FETCH_BOOKMARK);

  CheckShortInfo(SQL_MAX_CONCURRENT_ACTIVITIES, 0);
  CheckShortInfo(SQL_QUOTED_IDENTIFIER_CASE, SQL_IC_SENSITIVE);
  CheckShortInfo(SQL_ACTIVE_ENVIRONMENTS, 0);
  CheckShortInfo(SQL_CONCAT_NULL_BEHAVIOR, SQL_CB_NULL);
  CheckShortInfo(SQL_CORRELATION_NAME, SQL_CN_ANY);
  CheckShortInfo(SQL_FILE_USAGE, SQL_FILE_NOT_SUPPORTED);
  CheckShortInfo(SQL_GROUP_BY, SQL_GB_GROUP_BY_CONTAINS_SELECT);
  CheckShortInfo(SQL_IDENTIFIER_CASE, SQL_IC_SENSITIVE);
  CheckShortInfo(SQL_MAX_COLUMN_NAME_LEN, NAMEDATALEN);
  CheckShortInfo(SQL_MAX_COLUMNS_IN_GROUP_BY, 0);
  CheckShortInfo(SQL_MAX_COLUMNS_IN_INDEX, 0);
  CheckShortInfo(SQL_MAX_COLUMNS_IN_ORDER_BY, 0);
  CheckShortInfo(SQL_MAX_COLUMNS_IN_SELECT, 0);
  CheckShortInfo(SQL_MAX_COLUMNS_IN_TABLE, 0);
  CheckShortInfo(SQL_MAX_CURSOR_NAME_LEN, MAX_CURSOR_LEN);
  CheckShortInfo(SQL_MAX_DRIVER_CONNECTIONS, 0);
  CheckShortInfo(SQL_MAX_IDENTIFIER_LEN, NAMEDATALEN);
  CheckShortInfo(SQL_MAX_PROCEDURE_NAME_LEN, 0);
  CheckShortInfo(SQL_MAX_SCHEMA_NAME_LEN, NAMEDATALEN);
  CheckShortInfo(SQL_MAX_TABLE_NAME_LEN, NAMEDATALEN);
  CheckShortInfo(SQL_MAX_TABLES_IN_SELECT, 0);
  CheckShortInfo(SQL_MAX_USER_NAME_LEN, 0);
  CheckShortInfo(SQL_NON_NULLABLE_COLUMNS, SQL_NNC_NON_NULL);
  CheckShortInfo(SQL_NULL_COLLATION, SQL_NC_HIGH);
  CheckShortInfo(SQL_TXN_CAPABLE, SQL_TC_NONE);
}

BOOST_AUTO_TEST_SUITE_END()
