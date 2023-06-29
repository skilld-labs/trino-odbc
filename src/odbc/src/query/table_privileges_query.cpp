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

#include "timestream/odbc/query/table_privileges_query.h"

#include "timestream/odbc/connection.h"
#include "timestream/odbc/type_traits.h"

namespace timestream {
namespace odbc {
namespace query {
TablePrivilegesQuery::TablePrivilegesQuery(diagnostic::DiagnosableAdapter& diag)
    : Query(diag, QueryType::TABLE_PRIVILEGES), columnsMeta() {
  using namespace timestream::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(7);

  const std::string sch("");
  const std::string tbl("");

  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_CAT", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_SCHEM", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_NAME", ScalarType::VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "GRANTOR", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "GRANTEE", ScalarType::VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PRIVILEGE", ScalarType::VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "IS_GRANTABLE",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
}

TablePrivilegesQuery::~TablePrivilegesQuery() {
  // No-op.
}

SqlResult::Type TablePrivilegesQuery::Execute() {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLTablePrivileges is not supported. Return empty result set.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_SUCCESS_WITH_INFO;
}

SqlResult::Type TablePrivilegesQuery::Cancel() {
  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* TablePrivilegesQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type TablePrivilegesQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLTablePrivileges is not supported. No data is returned.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type TablePrivilegesQuery::GetColumn(
    uint16_t columnIdx, app::ApplicationDataBuffer& buffer) {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLTablePrivileges is not supported. No data is returned.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type TablePrivilegesQuery::Close() {
  return SqlResult::AI_SUCCESS;
}

bool TablePrivilegesQuery::DataAvailable() const {
  return false;
}

int64_t TablePrivilegesQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type TablePrivilegesQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}
}  // namespace query
}  // namespace odbc
}  // namespace timestream
