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

#include "trino/odbc/query/special_columns_query.h"

#include "trino/odbc/type_traits.h"

namespace trino {
namespace odbc {
namespace query {
SpecialColumnsQuery::SpecialColumnsQuery(diagnostic::DiagnosableAdapter& diag)
    : Query(diag, QueryType::SPECIAL_COLUMNS), columnsMeta() {
  using namespace trino::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(8);

  const std::string sch("");
  const std::string tbl("");

  columnsMeta.push_back(ColumnMeta(sch, tbl, "SCOPE", ScalarType::INTEGER,
                                   Nullability::NULLABLE));
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
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PSEUDO_COLUMN",
                                   ScalarType::INTEGER, Nullability::NULLABLE));
}

SpecialColumnsQuery::~SpecialColumnsQuery() {
  // No-op.
}

SqlResult::Type SpecialColumnsQuery::Execute() {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLSpecialColumns is not supported. Return empty result set.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_SUCCESS_WITH_INFO;
}

SqlResult::Type SpecialColumnsQuery::Cancel() {
  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* SpecialColumnsQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type SpecialColumnsQuery::FetchNextRow(app::ColumnBindingMap&) {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLSpecialColumns is not supported. No data is returned.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type SpecialColumnsQuery::GetColumn(uint16_t,
                                               app::ApplicationDataBuffer&) {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLSpecialColumns is not supported. No data is returned.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type SpecialColumnsQuery::Close() {
  return SqlResult::AI_SUCCESS;
}

bool SpecialColumnsQuery::DataAvailable() const {
  return false;
}

int64_t SpecialColumnsQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type SpecialColumnsQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}
}  // namespace query
}  // namespace odbc
}  // namespace trino
