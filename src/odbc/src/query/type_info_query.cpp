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

#include "timestream/odbc/query/type_info_query.h"

#include "timestream/odbc/system/odbc_constants.h"
#include "timestream/odbc/type_traits.h"

namespace {
struct ResultColumn {
  enum Type {
    /** Data source-dependent data-type name. */
    TYPE_NAME = 1,

    /** SQL data type. */
    DATA_TYPE,

    /** The maximum column size that the server supports for this data type. */
    COLUMN_SIZE,

    /** Character or characters used to prefix a literal. */
    LITERAL_PREFIX,

    /** Character or characters used to terminate a literal. */
    LITERAL_SUFFIX,

    /**
     * A list of keywords, separated by commas, corresponding to each
     * parameter that the application may specify in parentheses when using
     * the name that is returned in the TYPE_NAME field.
     */
    CREATE_PARAMS,

    /** Whether the data type accepts a NULL value. */
    NULLABLE,

    /**
     * Whether a character data type is case-sensitive in collations and
     * comparisons.
     */
    CASE_SENSITIVE,

    /** How the data type is used in a WHERE clause. */
    SEARCHABLE,

    /** Whether the data type is unsigned. */
    UNSIGNED_ATTRIBUTE,

    /** Whether the data type has predefined fixed precision and scale. */
    FIXED_PREC_SCALE,

    /** Whether the data type is auto-incrementing. */
    AUTO_UNIQUE_VALUE,

    /**
     * Localized version of the data source–dependent name of the data
     * type.
     */
    LOCAL_TYPE_NAME,

    /** The minimum scale of the data type on the data source. */
    MINIMUM_SCALE,

    /** The maximum scale of the data type on the data source. */
    MAXIMUM_SCALE,

    /**
     * The value of the SQL data type as it appears in the SQL_DESC_TYPE
     * field of the descriptor.
     */
    SQL_DATA_TYPE,

    /**
     * When the value of SQL_DATA_TYPE is SQL_DATETIME or SQL_INTERVAL,
     * this column contains the datetime/interval sub-code.
     */
    SQL_DATETIME_SUB,

    /**
     * If the data type is an approximate numeric type, this column
     * contains the value 2 to indicate that COLUMN_SIZE specifies a number
     * of bits.
     */
    NUM_PREC_RADIX,

    /**
     * If the data type is an interval data type, then this column contains
     * the value of the interval leading precision.
     */
    INTERVAL_PRECISION
  };
};
}  // namespace

namespace timestream {
namespace odbc {
namespace query {
TypeInfoQuery::TypeInfoQuery(diagnostic::DiagnosableAdapter& diag,
                             int16_t sqlType)
    : timestream::odbc::query::Query(
        diag, timestream::odbc::query::QueryType::TYPE_INFO),
      columnsMeta(),
      executed(false),
      fetched(false),
      types(),
      cursor(types.end()) {
  LOG_DEBUG_MSG("TypeInfoQuery constructor is called");

  using namespace timestream::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(19);

  const std::string sch;
  const std::string tbl;

  columnsMeta.push_back(ColumnMeta(sch, tbl, "TYPE_NAME", ScalarType::VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DATA_TYPE", ScalarType::INTEGER,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_SIZE", ScalarType::INTEGER,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "LITERAL_PREFIX",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "LITERAL_SUFFIX",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "CREATE_PARAMS",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NULLABLE", ScalarType::INTEGER,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "CASE_SENSITIVE",
                                   ScalarType::INTEGER, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "SEARCHABLE", ScalarType::INTEGER,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "UNSIGNED_ATTRIBUTE",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FIXED_PREC_SCALE",
                                   ScalarType::INTEGER, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "AUTO_UNIQUE_VALUE",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "LOCAL_TYPE_NAME",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "MINIMUM_SCALE",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "MAXIMUM_SCALE",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "SQL_DATA_TYPE",
                                   ScalarType::INTEGER, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "SQL_DATETIME_SUB",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NUM_PREC_RADIX",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "INTERVAL_PRECISION",
                                   ScalarType::INTEGER, Nullability::NULLABLE));

  assert(IsSqlTypeSupported(sqlType) || sqlType == SQL_ALL_TYPES);

  if (sqlType == SQL_ALL_TYPES) {
    types.push_back(static_cast< int8_t >(ScalarType::VARCHAR));
    types.push_back(static_cast< int8_t >(ScalarType::BOOLEAN));
    types.push_back(static_cast< int8_t >(ScalarType::BIGINT));
    types.push_back(static_cast< int8_t >(ScalarType::DOUBLE));
    types.push_back(static_cast< int8_t >(ScalarType::TIMESTAMP));
    types.push_back(static_cast< int8_t >(ScalarType::DATE));
    types.push_back(static_cast< int8_t >(ScalarType::TIME));
    types.push_back(static_cast< int8_t >(ScalarType::INTERVAL_DAY_TO_SECOND));
    types.push_back(static_cast< int8_t >(ScalarType::INTERVAL_YEAR_TO_MONTH));
    types.push_back(static_cast< int8_t >(ScalarType::INTEGER));
    types.push_back(static_cast< int8_t >(ScalarType::NOT_SET));
    types.push_back(static_cast< int8_t >(ScalarType::UNKNOWN));
  } else {
    types.push_back(static_cast< int8_t >(SqlTypeToBinary(sqlType)));
  }
}

TypeInfoQuery::~TypeInfoQuery() {
  // No-op.
}

SqlResult::Type TypeInfoQuery::Execute() {
  LOG_DEBUG_MSG("Execute is called");
  cursor = types.begin();

  executed = true;
  fetched = false;

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type TypeInfoQuery::Cancel() {
  LOG_DEBUG_MSG("Cancel is called");

  Close();

  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* TypeInfoQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type TypeInfoQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  LOG_DEBUG_MSG("FetchNextRow is called with columnBindings size "
                << columnBindings.size());
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (!fetched)
    fetched = true;
  else if (cursor != types.end())
    ++cursor;
  if (cursor == types.end()) {
    LOG_DEBUG_MSG("cursor reaches the end of types");
    return SqlResult::AI_NO_DATA;
  }

  app::ColumnBindingMap::iterator it;

  for (it = columnBindings.begin(); it != columnBindings.end(); ++it)
    GetColumn(it->first, it->second);

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type TypeInfoQuery::GetColumn(uint16_t columnIdx,
                                         app::ApplicationDataBuffer& buffer) {
  LOG_DEBUG_MSG("GetColumn is called with columnIdx " << columnIdx);

  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (cursor == types.end()) {
    std::string errMsg = "Cursor has reached end of the result set.";
    diag.AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE, errMsg);
    return SqlResult::AI_ERROR;
  }

  int8_t currentType = *cursor;
  LOG_DEBUG_MSG("currentType is " << currentType);

  switch (columnIdx) {
    case ResultColumn::TYPE_NAME: {
      buffer.PutString(type_traits::BinaryTypeToSqlTypeName(currentType));

      break;
    }

    case ResultColumn::DATA_TYPE:
    case ResultColumn::SQL_DATA_TYPE: {
      buffer.PutInt16(type_traits::BinaryToSqlType(currentType));

      break;
    }

    case ResultColumn::COLUMN_SIZE: {
      buffer.PutInt32(type_traits::BinaryTypeColumnSize(currentType));

      break;
    }

    case ResultColumn::LITERAL_PREFIX: {
      if (currentType == static_cast< int8_t >(ScalarType::VARCHAR))
        buffer.PutString("'");
      else
        buffer.PutNull();
      break;
    }

    case ResultColumn::LITERAL_SUFFIX: {
      if (currentType == static_cast< int8_t >(ScalarType::VARCHAR))
        buffer.PutString("'");
      else
        buffer.PutNull();
      break;
    }

    case ResultColumn::CREATE_PARAMS: {
      buffer.PutNull();

      break;
    }

    case ResultColumn::NULLABLE: {
      buffer.PutInt32(type_traits::BinaryTypeNullability(currentType));

      break;
    }

    case ResultColumn::CASE_SENSITIVE: {
      if (currentType == static_cast< int8_t >(ScalarType::VARCHAR))
        buffer.PutInt16(SQL_TRUE);
      else
        buffer.PutInt16(SQL_FALSE);
      break;
    }

    case ResultColumn::SEARCHABLE: {
      buffer.PutInt16(SQL_SEARCHABLE);

      break;
    }

    case ResultColumn::UNSIGNED_ATTRIBUTE: {
      buffer.PutInt16(type_traits::BinaryTypeUnsigned(currentType));

      break;
    }

    case ResultColumn::FIXED_PREC_SCALE:
    case ResultColumn::AUTO_UNIQUE_VALUE: {
      buffer.PutInt16(SQL_FALSE);

      break;
    }

    case ResultColumn::LOCAL_TYPE_NAME: {
      buffer.PutNull();

      break;
    }

    case ResultColumn::MINIMUM_SCALE:
    case ResultColumn::MAXIMUM_SCALE: {
      buffer.PutInt16(type_traits::BinaryTypeDecimalDigits(currentType));

      break;
    }

    case ResultColumn::SQL_DATETIME_SUB: {
      buffer.PutNull();

      break;
    }

    case ResultColumn::NUM_PREC_RADIX: {
      buffer.PutInt32(type_traits::BinaryTypeNumPrecRadix(currentType));

      break;
    }

    case ResultColumn::INTERVAL_PRECISION: {
      buffer.PutNull();

      break;
    }

    default:
      break;
  }

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type TypeInfoQuery::Close() {
  cursor = types.end();

  executed = false;

  return SqlResult::AI_SUCCESS;
}

bool TypeInfoQuery::DataAvailable() const {
  return executed && cursor != types.end();
}

int64_t TypeInfoQuery::AffectedRows() const {
  return 0;
}

int64_t TypeInfoQuery::RowNumber() const {
  if (!executed || cursor == types.end()) {
    diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                         "Cursor does not point to any data.",
                         timestream::odbc::LogLevel::Type::WARNING_LEVEL);

    LOG_DEBUG_MSG("Row number returned is 0.");

    return 0;
  }

  int64_t rowNumber = cursor - types.begin() + 1;
  LOG_DEBUG_MSG("Row number returned: " << rowNumber);

  return rowNumber;
}

SqlResult::Type TypeInfoQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}
}  // namespace query
}  // namespace odbc
}  // namespace timestream
