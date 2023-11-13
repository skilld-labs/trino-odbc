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

#ifdef _WIN32
#include <windows.h>
#endif

#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <utility>

#include "trino/odbc/meta/column_meta.h"
#include "trino/odbc/type_traits.h"
#include "odbc_test_suite.h"

using trino::odbc::OdbcTestSuite;
using trino::odbc::meta::ColumnMeta;
using trino::odbc::meta::Nullability;
using namespace boost::unit_test;

BOOST_AUTO_TEST_CASE(TestGetAttribute) {
  // Only SQL_DESC_* fields are tested in this test.
  // This is because those are the fields that would be passed to
  // SQLColAttribute function.
  using namespace trino::odbc::type_traits;

  std::string database("database");
  std::string table("table");
  std::string column("column");

  ColumnMeta columnMeta(database, table, column,
                        static_cast< int16_t >(ScalarType::VARCHAR),
                        Nullability::NULLABLE);

  SQLLEN intVal;
  std::string resVal;
  bool found;

  // test retrieving std::string value

  // test SQL_DESC_LABEL
  found = columnMeta.GetAttribute(SQL_DESC_LABEL, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, column);

  // test SQL_DESC_BASE_COLUMN_NAME
  found = columnMeta.GetAttribute(SQL_DESC_BASE_COLUMN_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, column);

  // test SQL_DESC_NAME
  found = columnMeta.GetAttribute(SQL_DESC_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, column);

  // test SQL_DESC_TABLE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_TABLE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, table);

  // test SQL_DESC_BASE_TABLE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_BASE_TABLE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, table);

  // test SQL_DESC_SCHEMA_NAME
  found = columnMeta.GetAttribute(SQL_DESC_SCHEMA_NAME, resVal);
  BOOST_CHECK(found);
  if (DATABASE_AS_SCHEMA)
    BOOST_CHECK_EQUAL(resVal, database);
  else
    BOOST_CHECK_EQUAL(resVal, "");

  // test SQL_DESC_CATALOG_NAME
  found = columnMeta.GetAttribute(SQL_DESC_CATALOG_NAME, resVal);
  BOOST_CHECK(found);
  if (DATABASE_AS_SCHEMA)
    BOOST_CHECK_EQUAL(resVal, "");
  else
    BOOST_CHECK_EQUAL(resVal, database);

  // test SQL_DESC_LITERAL_PREFIX
  found = columnMeta.GetAttribute(SQL_DESC_LITERAL_PREFIX, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, "'");

  // test SQL_DESC_LITERAL_SUFFIX
  found = columnMeta.GetAttribute(SQL_DESC_LITERAL_SUFFIX, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, "'");

  // test SQL_DESC_TYPE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_TYPE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, SqlTypeName::VARCHAR);

  // test SQL_DESC_LOCAL_TYPE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_LOCAL_TYPE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, SqlTypeName::VARCHAR);

  // fields SQL_COLUMN_PRECISION and SQL_DESC_SCALE are not tested
  // for retrieving string values

  // test retrieving SQLLEN value

  // test SQL_DESC_FIXED_PREC_SCALE
  found = columnMeta.GetAttribute(SQL_DESC_FIXED_PREC_SCALE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_FALSE);

  // test SQL_DESC_AUTO_UNIQUE_VALUE
  found = columnMeta.GetAttribute(SQL_DESC_AUTO_UNIQUE_VALUE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_FALSE);

  // test SQL_DESC_CASE_SENSITIVE
  found = columnMeta.GetAttribute(SQL_DESC_CASE_SENSITIVE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_TRUE);

  // test SQL_DESC_CONCISE_TYPE
  found = columnMeta.GetAttribute(SQL_DESC_CONCISE_TYPE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_VARCHAR);

  // test SQL_DESC_TYPE
  found = columnMeta.GetAttribute(SQL_DESC_TYPE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_VARCHAR);

  // test SQL_DESC_DISPLAY_SIZE
  found = columnMeta.GetAttribute(SQL_DESC_DISPLAY_SIZE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, TRINO_SQL_MAX_LENGTH);

  // test SQL_DESC_LENGTH
  found = columnMeta.GetAttribute(SQL_DESC_LENGTH, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, TRINO_SQL_MAX_LENGTH);

  // test SQL_DESC_OCTET_LENGTH
  found = columnMeta.GetAttribute(SQL_DESC_OCTET_LENGTH, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, TRINO_SQL_MAX_LENGTH);

  // test SQL_DESC_NULLABLE
  found = columnMeta.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NULLABLE);

  // test SQL_DESC_NUM_PREC_RADIX
  found = columnMeta.GetAttribute(SQL_DESC_NUM_PREC_RADIX, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, 0);

  // test SQL_DESC_PRECISION
  found = columnMeta.GetAttribute(SQL_DESC_PRECISION, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, TRINO_SQL_MAX_LENGTH);

  // test SQL_DESC_SCALE
  found = columnMeta.GetAttribute(SQL_DESC_SCALE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, -1);

  // test SQL_DESC_SEARCHABLE
  found = columnMeta.GetAttribute(SQL_DESC_SEARCHABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_PRED_BASIC);

  // test SQL_DESC_UNNAMED
  found = columnMeta.GetAttribute(SQL_DESC_UNNAMED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NAMED);

  // test SQL_DESC_UNSIGNED
  found = columnMeta.GetAttribute(SQL_DESC_UNSIGNED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_TRUE);

  // test SQL_DESC_UPDATABLE
  found = columnMeta.GetAttribute(SQL_DESC_UPDATABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_ATTR_READWRITE_UNKNOWN);
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLiteralPrefix) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, std::string > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     std::string("'")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     std::string("")),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          std::string("")),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          std::string("")),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          std::string("")),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     std::string(""))};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving std::string value

    // test SQL_DESC_LITERAL_PREFIX
    found = columnMeta.GetAttribute(SQL_DESC_LITERAL_PREFIX, resVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(resVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLiteralSuffix) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, std::string > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     std::string("'")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     std::string("")),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          std::string("")),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          std::string("")),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          std::string("")),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     std::string("")),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     std::string(""))};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving std::string value

    // test SQL_DESC_LITERAL_SUFFIX
    found = columnMeta.GetAttribute(SQL_DESC_LITERAL_SUFFIX, resVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(resVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLocalTypeName) {
  using namespace trino::odbc::type_traits;

  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, std::string > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     trino::odbc::type_traits::SqlTypeName::VARCHAR),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     trino::odbc::type_traits::SqlTypeName::BIT),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     trino::odbc::type_traits::SqlTypeName::BIGINT),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     trino::odbc::type_traits::SqlTypeName::DOUBLE),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     trino::odbc::type_traits::SqlTypeName::TIMESTAMP),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          trino::odbc::type_traits::SqlTypeName::DATE),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          trino::odbc::type_traits::SqlTypeName::TIME),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          trino::odbc::type_traits::SqlTypeName::INTERVAL_DAY_TO_SECOND),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          trino::odbc::type_traits::SqlTypeName::INTERVAL_YEAR_TO_MONTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     trino::odbc::type_traits::SqlTypeName::INTEGER),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     trino::odbc::type_traits::SqlTypeName::NOT_SET),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     trino::odbc::type_traits::SqlTypeName::UNKNOWN)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving std::string value

    // test SQL_DESC_LOCAL_TYPE_NAME
    found = columnMeta.GetAttribute(SQL_DESC_LOCAL_TYPE_NAME, resVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(resVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeCaseSensitive) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     true),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     false),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          false),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          false),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          false),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          false),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     false)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_CASE_SENSITIVE
    found = columnMeta.GetAttribute(SQL_DESC_CASE_SENSITIVE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeConciseTypeAndType) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     SQL_VARCHAR),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     SQL_BIT),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     SQL_BIGINT),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     SQL_DOUBLE),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     SQL_TYPE_TIMESTAMP),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          SQL_TYPE_DATE),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          SQL_TYPE_TIME),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          SQL_INTERVAL_DAY_TO_SECOND),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          SQL_INTERVAL_YEAR_TO_MONTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     SQL_INTEGER),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     SQL_VARCHAR),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     SQL_VARCHAR)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_CONCISE_TYPE
    found = columnMeta.GetAttribute(SQL_DESC_CONCISE_TYPE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);

    // test SQL_DESC_TYPE
    found = columnMeta.GetAttribute(SQL_DESC_TYPE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeDisplaySize) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     TRINO_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     1),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     20),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     24),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     20),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          10),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          8),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          25),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          12),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     11),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     TRINO_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     TRINO_SQL_MAX_LENGTH)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_DISPLAY_SIZE
    found = columnMeta.GetAttribute(SQL_DESC_DISPLAY_SIZE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLength) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     TRINO_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     1),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     20),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     24),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     20),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          10),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          8),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          25),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          12),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     11),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     TRINO_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     TRINO_SQL_MAX_LENGTH)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value
    // test SQL_DESC_LENGTH
    found = columnMeta.GetAttribute(SQL_DESC_LENGTH, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeOctetLength) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;
  size_t size_of_char = sizeof(char);

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     TRINO_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     1),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     8),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     8),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     16),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          6),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          6),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          34),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          34),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     4),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     TRINO_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     TRINO_SQL_MAX_LENGTH)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_OCTET_LENGTH
    found = columnMeta.GetAttribute(SQL_DESC_OCTET_LENGTH, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeNullable) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;
  ColumnMeta columnMetaNullable(database, table, column, ScalarType::UNKNOWN,
                                Nullability::NULLABLE);

  // test SQL_DESC_NULLABLE
  found = columnMetaNullable.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NULLABLE);

  ColumnMeta columnMetaNoNulls(database, table, column, ScalarType::UNKNOWN,
                               Nullability::NO_NULL);

  // test SQL_DESC_NULLABLE
  found = columnMetaNoNulls.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NO_NULLS);

  ColumnMeta columnMetaUnknown(database, table, column, ScalarType::UNKNOWN,
                               Nullability::NULLABILITY_UNKNOWN);

  // test SQL_DESC_NULLABLE
  found = columnMetaUnknown.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NULLABLE_UNKNOWN);
}

BOOST_AUTO_TEST_CASE(TestGetAttributeNumPrecRadix) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     0),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     10),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     10),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     2),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     0),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          0),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          0),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          0),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          0),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     10),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     0),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     0)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_NUM_PREC_RADIX
    found = columnMeta.GetAttribute(SQL_DESC_NUM_PREC_RADIX, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributePrecision) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     TRINO_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     1),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     19),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     15),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     19),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          10),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          8),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          25),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          12),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     10),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     TRINO_SQL_MAX_LENGTH),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     TRINO_SQL_MAX_LENGTH)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_PRECISION
    found = columnMeta.GetAttribute(SQL_DESC_PRECISION, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeScale) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     -1),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     -1),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     0),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     15),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     -1),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          -1),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          -1),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          -1),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          -1),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     0),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     -1),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     -1)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_SCALE
    found = columnMeta.GetAttribute(SQL_DESC_SCALE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeUnnamed) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;
  ColumnMeta columnMetaUnnamed(database, table, std::string(""),
                               ScalarType::UNKNOWN, Nullability::NULLABLE);

  // test SQL_DESC_UNNAMED
  found = columnMetaUnnamed.GetAttribute(SQL_DESC_UNNAMED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_UNNAMED);

  ColumnMeta columnMetaNamed(database, table, column, ScalarType::UNKNOWN,
                             Nullability::NULLABLE);

  // test SQL_DESC_UNNAMED
  found = columnMetaNamed.GetAttribute(SQL_DESC_UNNAMED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NAMED);
}

BOOST_AUTO_TEST_CASE(TestGetAttributeUnsigned) {
  std::string database("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::vector< std::pair< int16_t, SQLLEN > > tests = {
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::VARCHAR),
                     true),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BOOLEAN),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::BIGINT),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::DOUBLE),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::TIMESTAMP),
                     true),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::DATE),
          true),
      std::make_pair(
          static_cast< int16_t >(Aws::TrinoQuery::Model::ScalarType::TIME),
          true),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND),
          true),
      std::make_pair(
          static_cast< int16_t >(
              Aws::TrinoQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH),
          true),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::INTEGER),
                     false),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::NOT_SET),
                     true),
      std::make_pair(static_cast< int16_t >(
                         Aws::TrinoQuery::Model::ScalarType::UNKNOWN),
                     true)};

  for (int i = 0; i < tests.size(); i++) {
    ColumnMeta columnMeta(database, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_UNSIGNED
    found = columnMeta.GetAttribute(SQL_DESC_UNSIGNED, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}
