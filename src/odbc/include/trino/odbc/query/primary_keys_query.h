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

#ifndef _TRINO_ODBC_QUERY_PRIMARY_KEYS_QUERY
#define _TRINO_ODBC_QUERY_PRIMARY_KEYS_QUERY

#include "trino/odbc/query/query.h"

namespace trino {
namespace odbc {
namespace query {
/**
 * Primary keys query.
 */
class PrimaryKeysQuery : public Query {
 public:
  /**
   * Constructor.
   *
   * @param diag Diagnostics collector.
   */
  PrimaryKeysQuery(diagnostic::DiagnosableAdapter& diag);

  /**
   * Destructor.
   */
  virtual ~PrimaryKeysQuery();

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
   * Move to the next result set.
   *
   * @return Operation result.
   */
  virtual SqlResult::Type NextResultSet();

 private:
  IGNITE_NO_COPY_ASSIGNMENT(PrimaryKeysQuery);

  /** Columns metadata. */
  meta::ColumnMetaVector columnsMeta;
};
}  // namespace query
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_QUERY_PRIMARY_KEYS_QUERY
