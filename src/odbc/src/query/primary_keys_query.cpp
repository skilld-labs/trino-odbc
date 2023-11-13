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

#include "trino/odbc/query/primary_keys_query.h"

#include "trino/odbc/connection.h"
#include "trino/odbc/type_traits.h"

namespace trino {
namespace odbc {
namespace query {
PrimaryKeysQuery::PrimaryKeysQuery(diagnostic::DiagnosableAdapter& diag)
    : Query(diag, QueryType::PRIMARY_KEYS), columnsMeta() {
  using namespace trino::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(6);

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
  columnsMeta.push_back(ColumnMeta(sch, tbl, "KEY_SEQ", ScalarType::INTEGER,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PK_NAME", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
}

PrimaryKeysQuery::~PrimaryKeysQuery() {
  // No-op.
}

SqlResult::Type PrimaryKeysQuery::Execute() {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLPrimaryKeys is not supported. Return empty result set.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_SUCCESS_WITH_INFO;
}

SqlResult::Type PrimaryKeysQuery::Cancel() {
  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* PrimaryKeysQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type PrimaryKeysQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                       "SQLPrimaryKeys is not supported. No data is returned.",
                       LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type PrimaryKeysQuery::GetColumn(
    uint16_t columnIdx, app::ApplicationDataBuffer& buffer) {
  diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                       "SQLPrimaryKeys is not supported. No data is returned.",
                       LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type PrimaryKeysQuery::Close() {
  return SqlResult::AI_SUCCESS;
}

bool PrimaryKeysQuery::DataAvailable() const {
  return false;
}

int64_t PrimaryKeysQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type PrimaryKeysQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}
}  // namespace query
}  // namespace odbc
}  // namespace trino
