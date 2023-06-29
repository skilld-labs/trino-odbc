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

#include "timestream/odbc/query/statistics_query.h"

#include "timestream/odbc/connection.h"
#include "timestream/odbc/log.h"
#include "timestream/odbc/type_traits.h"

namespace timestream {
namespace odbc {
namespace query {
StatisticsQuery::StatisticsQuery(diagnostic::DiagnosableAdapter& diag,
                                 int32_t odbcVer)
    : Query(diag, timestream::odbc::query::QueryType::STATISTICS),
      columnsMeta() {
  LOG_DEBUG_MSG("StatisticsQuery is called");
  using namespace timestream::odbc::type_traits;

  using meta::ColumnMeta;
  using meta::Nullability;

  columnsMeta.reserve(13);

  const std::string sch("");
  const std::string tbl("");

  std::string catalog_meta_name = "TABLE_CAT";
  std::string schema_meta_name = "TABLE_SCHEM";
  std::string ordinal_pos_name = "ORDINAL_POSITION";
  std::string sort_order_name = "ASC_OR_DESC";

  if (odbcVer == SQL_OV_ODBC2) {
    // For backwards compatibility with ODBC 2.0
    catalog_meta_name = "TABLE_QUALIFIER";
    schema_meta_name = "TABLE_OWNER";
    ordinal_pos_name = "SEQ_IN_INDEX";
    sort_order_name = "COLLATION";
  }

  columnsMeta.push_back(ColumnMeta(sch, tbl, catalog_meta_name,
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, schema_meta_name,
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_NAME", ScalarType::VARCHAR,
                                   Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NON_UNIQUE", ScalarType::INTEGER,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "INDEX_QUALIFIER",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "INDEX_NAME", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(
      ColumnMeta(sch, tbl, "TYPE", ScalarType::INTEGER, Nullability::NO_NULL));
  columnsMeta.push_back(ColumnMeta(sch, tbl, ordinal_pos_name,
                                   ScalarType::INTEGER, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_NAME", ScalarType::VARCHAR,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, sort_order_name,
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "CARDINALITY", ScalarType::INTEGER,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "PAGES", ScalarType::INTEGER,
                                   Nullability::NULLABLE));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "FILTER_CONDITION",
                                   ScalarType::VARCHAR, Nullability::NULLABLE));
}

StatisticsQuery::~StatisticsQuery() {
  // No-op.
}

SqlResult::Type StatisticsQuery::Execute() {
  diag.AddStatusRecord(
      SqlState::S01000_GENERAL_WARNING,
      "SQLStatistics is not supported. Return empty result set.",
      LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_SUCCESS_WITH_INFO;
}

SqlResult::Type StatisticsQuery::Cancel() {
  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* StatisticsQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type StatisticsQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                       "SQLStatistics is not supported. No data is returned.",
                       LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type StatisticsQuery::GetColumn(uint16_t columnIdx,
                                           app::ApplicationDataBuffer& buffer) {
  diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                       "SQLStatistics is not supported. No data is returned.",
                       LogLevel::Type::WARNING_LEVEL);

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type StatisticsQuery::Close() {
  return SqlResult::AI_SUCCESS;
}

bool StatisticsQuery::DataAvailable() const {
  return false;
}
int64_t StatisticsQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type StatisticsQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}
}  // namespace query
}  // namespace odbc
}  // namespace timestream
