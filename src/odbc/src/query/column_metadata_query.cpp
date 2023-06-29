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

#include "timestream/odbc/query/column_metadata_query.h"

#include <vector>

#include "timestream/odbc/connection.h"
#include "timestream/odbc/ignite_error.h"
#include "timestream/odbc/system/odbc_constants.h"
#include "timestream/odbc/log.h"
#include "ignite/odbc/odbc_error.h"
#include "timestream/odbc/type_traits.h"

using timestream::odbc::IgniteError;

namespace {
struct ResultColumn {
  enum Type {
    /** Catalog name. NULL if not applicable to the data source. */
    TABLE_CAT = 1,

    /** Schema name. NULL if not applicable to the data source. */
    TABLE_SCHEM,

    /** Table name. */
    TABLE_NAME,

    /** Column name. */
    COLUMN_NAME,

    /** SQL data type. */
    DATA_TYPE,

    /** Data source-dependent data type name. */
    TYPE_NAME,

    /** Column size. */
    COLUMN_SIZE,

    /** The length in bytes of data transferred on fetch. */
    BUFFER_LENGTH,

    /** The total number of significant digits to the right of the decimal
       point. */
    DECIMAL_DIGITS,

    /** Precision. */
    NUM_PREC_RADIX,

    /** Nullability of the data in column (int). */
    NULLABLE,

    /** A description of the column. */
    REMARKS,

    /** Default value for the column. May be null. */
    COLUMN_DEF,

    /** SQL data type. */
    SQL_DATA_TYPE,

    /** Subtype code for datetime and interval data types. */
    SQL_DATETIME_SUB,

    /** Maximum length in bytes of a character or binary data type column.
       NULL for other data types. */
    CHAR_OCTET_LENGTH,

    /** Index of column in table (starting at 1) */
    ORDINAL_POSITION,

    /** Nullability of data in column (String). */
    IS_NULLABLE
  };
};
}  // namespace
namespace timestream {
namespace odbc {
namespace query {
ColumnMetadataQuery::ColumnMetadataQuery(
    diagnostic::DiagnosableAdapter& diag, Connection& connection,
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schema,
    const boost::optional< std::string >& table,
    const boost::optional< std::string >& column)
    : Query(diag, timestream::odbc::query::QueryType::COLUMN_METADATA),
      connection(connection),
      catalog(catalog),
      schema(schema),
      table(table),
      column(column),
      executed(false),
      fetched(false),
      meta(),
      columnsMeta() {
  LOG_DEBUG_MSG("ColumnMetadataQuery is called");
  using namespace timestream::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(18);

  const std::string sch("");
  const std::string tbl("");

  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_CAT", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_SCHEM", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_NAME", ScalarType::VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_NAME", ScalarType::VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DATA_TYPE", ScalarType::INTEGER,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TYPE_NAME", ScalarType::VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_SIZE", ScalarType::INTEGER,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "BUFFER_LENGTH",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DECIMAL_DIGITS",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NUM_PREC_RADIX",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NULLABLE", ScalarType::INTEGER,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "REMARKS", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_DEF", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "SQL_DATA_TYPE",
                                   ScalarType::INTEGER, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "SQL_DATETIME_SUB",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "CHAR_OCTET_LENGTH",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "ORDINAL_POSITION",
                                   ScalarType::INTEGER, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "IS_NULLABLE", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));

  tableMetadataQuery_ = std::make_shared< TableMetadataQuery >(
      diag, connection, catalog, schema, table, boost::none);
}

ColumnMetadataQuery::~ColumnMetadataQuery() {
  // No-op.
}

SqlResult::Type ColumnMetadataQuery::Execute() {
  LOG_DEBUG_MSG("Execute is called");
  if (executed)
    Close();

  if (DATABASE_AS_SCHEMA && catalog && !catalog->empty()
      && *catalog != SQL_ALL_CATALOGS) {
    // catalog has been provided with a non-empty value that isn't
    // SQL_ALL_CATALOGS. Return empty result set by default since
    // Timestream does not have catalogs.
    diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                         "Empty result set is returned as catalog is set to \""
                             + *catalog
                             + "\" and Timestream does not have catalogs");
    return SqlResult::AI_SUCCESS_WITH_INFO;
  } else if (!DATABASE_AS_SCHEMA && schema && !schema->empty()
             && *schema != SQL_ALL_SCHEMAS) {
    // schema has been provided with a non-empty value that isn't
    // SQL_ALL_SCHEMAS. Return empty result set by default since
    // Timestream does not have schemas.
    diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                         "Empty result set is returned as schema is set to \""
                             + *schema
                             + "\" and Timestream does not have schemas");
    return SqlResult::AI_SUCCESS_WITH_INFO;
  }

  if (connection.GetMetadataID()) {
    if ((DATABASE_AS_SCHEMA && (schema == boost::none || table == boost::none))
        || (!DATABASE_AS_SCHEMA
            && (catalog == boost::none || table == boost::none))
        || column == boost::none) {
      if (DATABASE_AS_SCHEMA) {
        diag.AddStatusRecord(
            SqlState::SHY009_INVALID_USE_OF_NULL_POINTER,
            "SQL_ATTR_METADATA_ID statement attribute was set to SQL_TRUE, and "
            "the SchemaName, TableName, or ColumnName argument was a null "
            "pointer.");
      } else {
        diag.AddStatusRecord(
            SqlState::SHY009_INVALID_USE_OF_NULL_POINTER,
            "SQL_ATTR_METADATA_ID statement attribute was set to SQL_TRUE, and "
            "the CatalogName, TableName, or ColumnName argument was a null "
            "pointer.");
      }
      return SqlResult::AI_ERROR;
    }
  } else {
    if (catalog == boost::none) {
      catalog = std::string("%");
    }
    if (schema == boost::none) {
      schema = std::string("%");
    }
    if (table == boost::none) {
      table = std::string("%");
    }
    if (column == boost::none) {
      column = std::string("%");
    }
  }

  if (DATABASE_AS_SCHEMA) {
    if (schema->empty() || table->empty()) {
      std::string warnMsg = "Schema and table name should not be empty.";
      diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING, warnMsg,
                           LogLevel::Type::WARNING_LEVEL);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }
  } else {
    if (catalog->empty() || table->empty()) {
      std::string warnMsg = "Catalog and table name should not be empty.";

      diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING, warnMsg,
                           LogLevel::Type::WARNING_LEVEL);
      return SqlResult::AI_SUCCESS_WITH_INFO;
    }
  }

  SqlResult::Type result = MakeRequestGetColumnsMeta();

  if (result == SqlResult::AI_SUCCESS) {
    executed = true;
    fetched = false;

    cursor = meta.begin();
  }

  LOG_DEBUG_MSG("Execute exiting with " << result);
  return result;
}

SqlResult::Type ColumnMetadataQuery::Cancel() {
  LOG_DEBUG_MSG("Cancel is called");
  if (dataQuery_) {
    dataQuery_->Cancel();
  }

  Close();

  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* ColumnMetadataQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type ColumnMetadataQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  LOG_DEBUG_MSG("FetchNextRow is called");
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (!fetched)
    fetched = true;
  else if (cursor != meta.end())
    ++cursor;
  if (cursor == meta.end()) {
    LOG_DEBUG_MSG("cursor reaches meta end");
    return SqlResult::AI_NO_DATA;
  }

  app::ColumnBindingMap::iterator it;

  for (it = columnBindings.begin(); it != columnBindings.end(); ++it)
    GetColumn(it->first, it->second);

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type ColumnMetadataQuery::GetColumn(
    uint16_t columnIdx, app::ApplicationDataBuffer& buffer) {
  LOG_DEBUG_MSG("GetColumn is called with columnIdx " << columnIdx);
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (cursor == meta.end()) {
    std::string errMsg = "Cursor has reached end of the result set.";
    diag.AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE, errMsg);
    return SqlResult::AI_ERROR;
  }

  const meta::ColumnMeta& currentColumn = *cursor;
  boost::optional< int16_t > columnType = currentColumn.GetDataType();
  LOG_DEBUG_MSG("columnType is " << columnType.get_value_or(-1));

  switch (columnIdx) {
    case ResultColumn::TABLE_CAT: {
      buffer.PutString(currentColumn.GetCatalogName());
      break;
    }

    case ResultColumn::TABLE_SCHEM: {
      buffer.PutString(currentColumn.GetSchemaName());
      break;
    }

    case ResultColumn::TABLE_NAME: {
      buffer.PutString(currentColumn.GetTableName());
      break;
    }

    case ResultColumn::COLUMN_NAME: {
      buffer.PutString(currentColumn.GetColumnName());
      break;
    }

    case ResultColumn::DATA_TYPE: {
      buffer.PutInt16(type_traits::BinaryToSqlType(columnType));
      break;
    }

    case ResultColumn::TYPE_NAME: {
      buffer.PutString(type_traits::BinaryTypeToSqlTypeName(columnType));
      break;
    }

    case ResultColumn::COLUMN_SIZE: {
      buffer.PutInt32(type_traits::BinaryTypeColumnSize(columnType));
      break;
    }

    case ResultColumn::BUFFER_LENGTH: {
      buffer.PutInt32(type_traits::BinaryTypeTransferLength(columnType));
      break;
    }

    case ResultColumn::DECIMAL_DIGITS: {
      boost::optional< int16_t > decDigits =
          type_traits::BinaryTypeDecimalDigits(columnType);
      if (!decDigits || *decDigits < 0)
        buffer.PutNull();
      else
        buffer.PutInt16(*decDigits);
      break;
    }

    case ResultColumn::NUM_PREC_RADIX: {
      boost::optional< int32_t > numPrecRadix =
          type_traits::BinaryTypeNumPrecRadix(columnType);
      if (!numPrecRadix || numPrecRadix < 0)
        buffer.PutNull();
      else
        buffer.PutInt16(static_cast< int16_t >(*numPrecRadix));
      break;
    }

    case ResultColumn::NULLABLE: {
      buffer.PutInt32(currentColumn.GetNullability());
      break;
    }

    case ResultColumn::REMARKS: {
      buffer.PutString(currentColumn.GetRemarks());
      break;
    }

    case ResultColumn::COLUMN_DEF: {
      buffer.PutString(currentColumn.GetColumnDef());
      break;
    }

    case ResultColumn::SQL_DATA_TYPE: {
      buffer.PutInt16(type_traits::BinaryToSqlType(columnType));
      break;
    }

    case ResultColumn::SQL_DATETIME_SUB: {
      buffer.PutNull();
      break;
    }

    case ResultColumn::CHAR_OCTET_LENGTH: {
      buffer.PutInt32(type_traits::BinaryTypeCharOctetLength(columnType));
      break;
    }

    case ResultColumn::ORDINAL_POSITION: {
      buffer.PutInt32(currentColumn.GetOrdinalPosition());
      break;
    }

    case ResultColumn::IS_NULLABLE: {
      buffer.PutString(
          type_traits::NullabilityToIsNullable(currentColumn.GetNullability()));
      break;
    }

    default: {
      diag.AddStatusRecord(SqlState::S07009_INVALID_DESCRIPTOR_INDEX,
                           "Invalid index.");
      return SqlResult::AI_ERROR;
    }
  }

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type ColumnMetadataQuery::Close() {
  meta.clear();
  cursor = meta.end();

  executed = false;

  return SqlResult::AI_SUCCESS;
}

bool ColumnMetadataQuery::DataAvailable() const {
  return executed && !meta.empty() && cursor != meta.end();
}

int64_t ColumnMetadataQuery::AffectedRows() const {
  return 0;
}

int64_t ColumnMetadataQuery::RowNumber() const {
  if (!executed || cursor == meta.end()) {
    diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                         "Cursor does not point to any data.",
                         timestream::odbc::LogLevel::Type::WARNING_LEVEL);

    LOG_DEBUG_MSG("Row number returned is 0.");

    return 0;
  }

  int64_t rowNumber = cursor - meta.begin() + 1;
  LOG_DEBUG_MSG("Row number returned: " << rowNumber);

  return rowNumber;
}

SqlResult::Type ColumnMetadataQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}

SqlResult::Type ColumnMetadataQuery::GetColumnsWithDatabaseSearchPattern(
    const boost::optional< std::string >& databasePattern,
    TableMetadataQuery::ResultColumn::Type databaseType) {
  LOG_DEBUG_MSG(
      "GetColumnsWithDatabaseSearchPattern is called with databasePattern "
      << databasePattern.get_value_or(""));
  if (!connection.GetMetadataID()) {
    // database name and table name are treated as search patterns
    SqlResult::Type result = tableMetadataQuery_->Execute();
    if (result != SqlResult::AI_SUCCESS) {
      std::string warnMsg = "Failed to get table metadata for "
                            + databasePattern.get_value_or("") + "."
                            + table.get_value_or("");
      diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING, warnMsg,
                           LogLevel::Type::WARNING_LEVEL);
      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    app::ColumnBindingMap columnBindings;
    SqlLen buflen = STRING_BUFFER_SIZE;
    // According to Timestream, database name could only contain
    // letters, digits, dashes, periods or underscores. It could
    // not be a unicode string.
    char databaseName[STRING_BUFFER_SIZE]{};
    ApplicationDataBuffer buf1(
        timestream::odbc::type_traits::OdbcNativeType::Type::AI_CHAR,
        databaseName, buflen, nullptr);
    columnBindings[databaseType] = buf1;

    // According to Timestream, table name could only contain
    // letters, digits, dashes, periods or underscores. It could
    // not be a unicode string.
    char tableName[STRING_BUFFER_SIZE]{};
    ApplicationDataBuffer buf2(
        timestream::odbc::type_traits::OdbcNativeType::Type::AI_CHAR,
        &tableName, buflen, nullptr);
    columnBindings[TableMetadataQuery::ResultColumn::TABLE_NAME] = buf2;

    LOG_ERROR_MSG("table is " << databaseName << "." << tableName);
    while (tableMetadataQuery_->FetchNextRow(columnBindings)
           == SqlResult::AI_SUCCESS) {
      result = MakeRequestGetColumnsMetaPerTable(std::string(databaseName),
                                                 std::string(tableName));
      if (result != SqlResult::AI_SUCCESS
          && result != SqlResult::AI_SUCCESS_WITH_INFO) {
        LOG_ERROR_MSG("Failed to get columns for " << databaseName << "."
                                                   << tableName);
        break;
      }
    }
    return result;
  } else {
    // database name and table name are treated as case insensitive identifiers
    return MakeRequestGetColumnsMetaPerTable(databasePattern.get_value_or(""),
                                             table.get_value_or(""));
  }
}

SqlResult::Type ColumnMetadataQuery::MakeRequestGetColumnsMeta() {
  LOG_DEBUG_MSG("MakeRequestGetColumnsMeta is called");
  // meta should only be cleared here as MakeRequestGetColumnsMetaPerTable()
  // could be called multiple times
  meta.clear();

  if (DATABASE_AS_SCHEMA) {
    // databases are reported as schemas
    return GetColumnsWithDatabaseSearchPattern(
        schema, TableMetadataQuery::ResultColumn::TABLE_SCHEM);
  } else {
    // databases are reported as catalogs
    return GetColumnsWithDatabaseSearchPattern(
        catalog, TableMetadataQuery::ResultColumn::TABLE_CAT);
  }
}

SqlResult::Type ColumnMetadataQuery::MakeRequestGetColumnsMetaPerTable(
    const std::string& databaseName, const std::string& tableName) {
  LOG_DEBUG_MSG(
      "MakeRequestGetColumnsMetaPerTable is called with databaseName: "
      << databaseName << ", tableName: " << tableName);
  std::string sql = "describe \"";
  sql += databaseName;
  sql += "\".\"";
  sql += tableName + "\"";
  LOG_DEBUG_MSG("sql is " << sql);

  dataQuery_ = std::make_shared< DataQuery >(diag, connection, sql);
  SqlResult::Type result = dataQuery_->Execute();
  if (result != SqlResult::AI_SUCCESS) {
    LOG_DEBUG_MSG("Sql execution result is " << result);
    return SqlResult::AI_NO_DATA;
  }

  app::ColumnBindingMap columnBindings;
  SqlLen buflen = STRING_BUFFER_SIZE;
  // column name could be a unicode string
  SQLWCHAR columnName[STRING_BUFFER_SIZE];
  ApplicationDataBuffer buf1(
      timestream::odbc::type_traits::OdbcNativeType::Type::AI_WCHAR, columnName,
      buflen, nullptr);
  columnBindings[1] = buf1;

  char dataType[64];
  ApplicationDataBuffer buf2(
      timestream::odbc::type_traits::OdbcNativeType::Type::AI_CHAR, &dataType,
      buflen, nullptr);
  columnBindings[2] = buf2;

  char remarks[64];
  ApplicationDataBuffer buf3(
      timestream::odbc::type_traits::OdbcNativeType::Type::AI_CHAR, &remarks,
      buflen, nullptr);
  columnBindings[3] = buf3;

  LOG_DEBUG_MSG("column is " << columnName << ", dataType is " << dataType
                             << ", remarks is " << remarks);

  int32_t prevPosition = 0;
  while (dataQuery_->FetchNextRow(columnBindings) == SqlResult::AI_SUCCESS) {
    if (column.get_value_or("") == "%"
        || column.get_value_or("")
               == utility::SqlWcharToString(columnName, STRING_BUFFER_SIZE)) {
      meta.emplace_back(meta::ColumnMeta(databaseName, tableName));
      meta.back().Read(columnBindings, ++prevPosition);
    }
  }

  LOG_DEBUG_MSG("meta size is " << meta.size());

  if (meta.empty()) {
    diag.AddStatusRecord(
        SqlState::S01000_GENERAL_WARNING,
        "No columns with name \'" + column.get_value_or("") + "\' found",
        LogLevel::Type::WARNING_LEVEL);
    result = SqlResult::AI_SUCCESS_WITH_INFO;
  }

  return result;
}
}  // namespace query
}  // namespace odbc
}  // namespace timestream
