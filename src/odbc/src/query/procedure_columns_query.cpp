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

#include "timestream/odbc/query/procedure_columns_query.h"

#include "timestream/odbc/connection.h"
#include "timestream/odbc/type_traits.h"

namespace timestream {
namespace odbc {
namespace query {
ProcedureColumnsQuery::ProcedureColumnsQuery(
    diagnostic::DiagnosableAdapter& diag)
    : Query(diag, QueryType::PROCEDURE_COLUMNS), columnsMeta() {
  using namespace timestream::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(19);

  const std::string sch("");
  const std::string tbl("");

  columnsMeta.push_back(ColumnMeta(sch, tbl, "PROCEDURE_CAT",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PROCEDURE_SCHEM",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PROCEDURE_NAME",
                                   ScalarType::VARCHAR, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_NAME", ScalarType::VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_TYPE", ScalarType::INTEGER,
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
}

ProcedureColumnsQuery::~ProcedureColumnsQuery() {
  // No-op.
}

SqlResult::Type ProcedureColumnsQuery::Execute() {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLProcedureColumns is not supported. Return empty result set.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_SUCCESS_WITH_INFO;
}

SqlResult::Type ProcedureColumnsQuery::Cancel() {
  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* ProcedureColumnsQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type ProcedureColumnsQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLProcedureColumns is not supported. No data is returned.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type ProcedureColumnsQuery::GetColumn(
    uint16_t columnIdx, app::ApplicationDataBuffer& buffer) {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLProcedureColumns is not supported. No data is returned.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type ProcedureColumnsQuery::Close() {
  return SqlResult::AI_SUCCESS;
}

bool ProcedureColumnsQuery::DataAvailable() const {
  return false;
}

int64_t ProcedureColumnsQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type ProcedureColumnsQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}
}  // namespace query
}  // namespace odbc
}  // namespace timestream
