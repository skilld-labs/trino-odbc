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

#ifndef _TIMESTREAM_ODBC_QUERY_TYPE_INFO_QUERY
#define _TIMESTREAM_ODBC_QUERY_TYPE_INFO_QUERY

#include "timestream/odbc/query/query.h"

namespace timestream {
namespace odbc {
namespace query {
/**
 * Type info query.
 */
class TypeInfoQuery : public timestream::odbc::query::Query {
 public:
  /**
   * Constructor.
   *
   * @param diag Diagnostics collector.
   * @param sqlType SQL type.
   */
  TypeInfoQuery(diagnostic::DiagnosableAdapter& diag, int16_t sqlType);

  /**
   * Destructor.
   */
  virtual ~TypeInfoQuery();

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
  IGNITE_NO_COPY_ASSIGNMENT(TypeInfoQuery);

  /** Columns metadata. */
  meta::ColumnMetaVector columnsMeta;

  /** Executed flag. */
  bool executed;

  /** Fetched flag. */
  bool fetched;

  /** Requested types. */
  std::vector< int8_t > types;

  /** Query cursor. */
  std::vector< int8_t >::const_iterator cursor;
};
}  // namespace query
}  // namespace odbc
}  // namespace timestream

#endif  //_TIMESTREAM_ODBC_QUERY_TYPE_INFO_QUERY
