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

#ifndef _TRINO_ODBC_QUERY_COLUMN_METADATA_QUERY
#define _TRINO_ODBC_QUERY_COLUMN_METADATA_QUERY

#include "trino/odbc/query/query.h"
#include "trino/odbc/query/data_query.h"
#include "trino/odbc/query/table_metadata_query.h"

namespace trino {
namespace odbc {
/** Connection forward-declaration. */
class Connection;

namespace query {
/**
 * Query.
 */
class ColumnMetadataQuery : public trino::odbc::query::Query {
 public:
  /**
   * Constructor.
   *
   * @param diag Diagnostics collector.
   * @param connection Associated connection.
   * @param schema Schema search pattern.
   * @param table Table search pattern.
   * @param column Column search pattern.
   */
  ColumnMetadataQuery(diagnostic::DiagnosableAdapter& diag,
                      Connection& connection,
                      const boost::optional< std::string >& catalog,
                      const boost::optional< std::string >& schema,
                      const boost::optional< std::string >& table,
                      const boost::optional< std::string >& column);

  /**
   * Destructor.
   */
  virtual ~ColumnMetadataQuery();

  /**
   * Execute query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Execute();

  /**
   * Cancel query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Cancel();

  /**
   * Get column metadata.
   *
   * @return Column metadata.
   */
  virtual const meta::ColumnMetaVector* GetMeta();

  /**
   * Fetch next result row to application buffers.
   *
   * @return Operation result.
   */
  virtual SqlResult::Type FetchNextRow(app::ColumnBindingMap& columnBindings);

  /**
   * Get data of the specified column in the result set.
   *
   * @param columnIdx Column index.
   * @param buffer Buffer to put column data to.
   * @return Operation result.
   */
  virtual SqlResult::Type GetColumn(uint16_t columnIdx,
                                    app::ApplicationDataBuffer& buffer);

  /**
   * Close query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Close();

  /**
   * Check if data is available.
   *
   * @return True if data is available.
   */
  virtual bool DataAvailable() const;

  /**
   * Get number of rows affected by the statement.
   *
   * @return Number of rows affected by the statement.
   */
  virtual int64_t AffectedRows() const;

  /**
   * Get row number of the row that the cursor points at.
   * Row number starts at 1.
   *
   * @return Row number of the row that the cursor points at.
   */
  virtual int64_t RowNumber() const;

  /**
   * Move to the next result set.
   *
   * @return Operation result.
   */
  virtual SqlResult::Type NextResultSet();

 private:
  IGNITE_NO_COPY_ASSIGNMENT(ColumnMetadataQuery);

  /**
   * Get columns metadata with database search pattern
   *
   * @param databasePattern Database pattern
   * @param databaseType ResultColumn Type.
   *
   * @return Operation result.
   */
  SqlResult::Type GetColumnsWithDatabaseSearchPattern(
      const boost::optional< std::string >& databasePattern,
      TableMetadataQuery::ResultColumn::Type databaseType);

  /**
   * Make get columns metadata requets and use response to set internal state.
   *
   * @return Operation result.
   */
  SqlResult::Type MakeRequestGetColumnsMeta();

  /**
   * Make get columns metadata requets and use response to set internal state
   * for a table.
   *
   * @param databaseName Database name
   * @param tableName Table name
   *
   * @return Operation result.
   */
  SqlResult::Type MakeRequestGetColumnsMetaPerTable(
      const std::string& databaseName, const std::string& tableName);

  /** Connection associated with the statement. */
  Connection& connection;

  /** Catalog search pattern. */
  boost::optional< std::string > catalog;

  /** Schema search pattern. */
  boost::optional< std::string > schema;

  /** Table search pattern. */
  boost::optional< std::string > table;

  /** Column search pattern. */
  boost::optional< std::string > column;

  /** Query executed. */
  bool executed;

  /** Fetched flag. */
  bool fetched;

  /** Fetched metadata. */
  meta::ColumnMetaVector meta;

  /** Metadata cursor. */
  meta::ColumnMetaVector::iterator cursor;

  /** Columns metadata. */
  meta::ColumnMetaVector columnsMeta;

  /** DataQuery pointer for "describe" command to run **/
  std::shared_ptr< DataQuery > dataQuery_;

  /** TableMetadataQuery pointer for fetching table **/
  std::shared_ptr< TableMetadataQuery > tableMetadataQuery_;
};
}  // namespace query
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_QUERY_COLUMN_METADATA_QUERY
