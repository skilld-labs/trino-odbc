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

#ifndef _TIMESTREAM_ODBC_QUERY_DATA_QUERY
#define _TIMESTREAM_ODBC_QUERY_DATA_QUERY

#include "timestream/odbc/timestream_cursor.h"
#include "timestream/odbc/query/query.h"
#include "timestream/odbc/connection.h"

#include <aws/timestream-query/model/QueryRequest.h>
#include <aws/timestream-query/model/QueryResult.h>
#include <aws/timestream-query/model/ColumnInfo.h>

#include <queue>
#include <mutex>
#include <condition_variable>

using Aws::TimestreamQuery::Model::ColumnInfo;
using Aws::TimestreamQuery::Model::QueryRequest;
using Aws::TimestreamQuery::Model::QueryResult;

namespace timestream {
namespace odbc {
/** Connection forward-declaration. */
class Connection;

namespace query {
/**
 * Context for asynchronous fetching data query result.
 */
class IGNITE_IMPORT_EXPORT DataQueryContext {
 public:
  DataQueryContext() : isClosing_(false) {
  }

  ~DataQueryContext() = default;

  /** mutex */
  std::mutex mutex_;

  /** condition variable to synchronize threads */
  std::condition_variable cv_;

  /** queue to save query execution outcome objects. */
  std::queue< Aws::TimestreamQuery::Model::QueryOutcome > queue_;

  /** Flag to indicate if the main thread is exiting or not. */
  bool isClosing_;
};

/**
 * Query.
 */
class IGNITE_IMPORT_EXPORT DataQuery : public timestream::odbc::query::Query {
 public:
  /**
   * Constructor.
   *
   * @param diag Diagnostics collector.
   * @param connection Associated connection.
   * @param sql SQL query string.
   */
  DataQuery(diagnostic::DiagnosableAdapter& diag, Connection& connection,
            const std::string& sql);

  /**
   * Destructor.
   */
  virtual ~DataQuery();

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
   * @param columnBindings Application buffers to put data to.
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
   * @return Result.
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
   * @return Operaion result.
   */
  virtual SqlResult::Type NextResultSet();

  /**
   * Get SQL query string.
   *
   * @return SQL query string.
   */
  const std::string& GetSql() const {
    return sql_;
  }

 private:
  IGNITE_NO_COPY_ASSIGNMENT(DataQuery);

  /**
   * Make query prepare request and use response to set internal
   * state.
   *
   * @return Result.
   */
  SqlResult::Type MakeRequestPrepare();

  /**
   * Make query execute request and use response to set internal
   * state.
   *
   * @return Result.
   */
  SqlResult::Type MakeRequestExecute();

  /**
   * Make data fetch request and use response to set internal state.
   *
   * @return Result.
   */
  SqlResult::Type MakeRequestFetch();

  /**
   * Make result set metadata request.
   *
   * @return Result.
   */
  SqlResult::Type MakeRequestResultsetMeta();

  /**
   * Fetch one page resultset.
   * @param isFirst Flag indicating if the page to be fetched is the first page
   * or not.
   * @return Result.
   */
  SqlResult::Type FetchOnePage(bool isFirst);

  /**
   * Set result set meta by reading AWS Timestream column metadata vector.
   *
   * @param tsVector Aws::TimestreamQuery::Model::ColumnInfo vector.
   */
  void ReadColumnMetadataVector(const Aws::Vector< ColumnInfo >& tsVector);

  /**
   * Process column conversion operation result.
   *
   * @param convRes Conversion result.
   * @param rowIdx Row index.
   * @param columnIdx Column index.
   * @return General SQL result.
   */
  SqlResult::Type ProcessConversionResult(app::ConversionResult::Type convRes,
                                          int32_t rowIdx, int32_t columnIdx);

  /**
   * Set result set meta.
   *
   * @param value Metadata value.
   */
  void SetResultsetMeta(const meta::ColumnMetaVector& value);

  /**
   * Close query.
   *
   * @return Result.
   */
  SqlResult::Type InternalClose();

  /**
   * Switch cursor to hold next resultset page data.
   *
   * @return Result.
   */
  SqlResult::Type SwitchCursor();

  /**
   * Record the thread so they could be waited before the main thread ends.
   * @param thread Thread to be saved.
   *
   * @return void.
   */
  void addThreads(std::thread& thread) {
    threads_.push(std::move(thread));
  }

  /** Connection associated with the statement. */
  Connection& connection_;

  /** SQL Query. */
  std::string sql_;

  /** Result set metadata is available */
  bool resultMetaAvailable_;

  /** Result set metadata. */
  meta::ColumnMetaVector resultMeta_;

  /** Current TS Query Request. */
  QueryRequest request_;

  /** Current TS Query Result. */
  std::shared_ptr< QueryResult > result_;

  /** Cursor. */
  std::unique_ptr< TimestreamCursor > cursor_;

  /** Timestream query client. */
  std::shared_ptr< Aws::TimestreamQuery::TimestreamQueryClient > queryClient_;

  /** Context for asynchornous result fetching. */
  DataQueryContext context_;

  /** Queue for threads. */
  std::queue< std::thread > threads_;

  /** Flag indicating asynchronous fetch is started. */
  bool hasAsyncFetch;

  /** Row counter for how many rows has been fetched */
  int rowCounter;
};
}  // namespace query
}  // namespace odbc
}  // namespace timestream

#endif  //_TIMESTREAM_ODBC_QUERY_DATA_QUERY
