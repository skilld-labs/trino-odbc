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

#include "trino/odbc/query/procedures_query.h"

#include "trino/odbc/connection.h"
#include "trino/odbc/type_traits.h"

namespace trino {
namespace odbc {
namespace query {
ProceduresQuery::ProceduresQuery(diagnostic::DiagnosableAdapter& diag)
    : Query(diag, QueryType::PROCEDURES), columnsMeta() {
  using namespace trino::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(8);

  const std::string sch("");
  const std::string tbl("");

  columnsMeta.push_back(ColumnMeta(sch, tbl, "PROCEDURE_CAT",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PROCEDURE_SCHEM",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PROCEDURE_NAME",
                                   ScalarType::VARCHAR, Nullability::NO_NULL));
  // NUM_INPUT_PARAMS, NUM_OUTPUT_PARAMS, and NUM_RESULT_SETS intentionally set
  // to "NOT_SET", as they are reserved for future use according to Microsoft
  // ODBC Documentation. Internally, "NOT_SET" is treated same as VARCHAR data
  // type.
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NUM_INPUT_PARAMS",
                                   ScalarType::NOT_SET, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NUM_OUTPUT_PARAMS",
                                   ScalarType::NOT_SET, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NUM_RESULT_SETS",
                                   ScalarType::NOT_SET, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "REMARKS", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PROCEDURE_TYPE",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
}

ProceduresQuery::~ProceduresQuery() {
  // No-op.
}

SqlResult::Type ProceduresQuery::Execute() {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLProcedures is not supported. Return empty result set.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_SUCCESS_WITH_INFO;
}

SqlResult::Type ProceduresQuery::Cancel() {
  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* ProceduresQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type ProceduresQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                       "SQLProcedures is not supported. No data is returned.",
                       LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type ProceduresQuery::GetColumn(uint16_t columnIdx,
                                           app::ApplicationDataBuffer& buffer) {
  diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                       "SQLProcedures is not supported. No data is returned.",
                       LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type ProceduresQuery::Close() {
  return SqlResult::AI_SUCCESS;
}

bool ProceduresQuery::DataAvailable() const {
  return false;
}

int64_t ProceduresQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type ProceduresQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}
}  // namespace query
}  // namespace odbc
}  // namespace trino
