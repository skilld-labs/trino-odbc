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

#ifndef _TIMESTREAM_ODBC_IGNITE_CURSOR
#define _TIMESTREAM_ODBC_IGNITE_CURSOR

#include <stdint.h>

#include <map>
#include <memory>

#include "timestream/odbc/common_types.h"
#include "timestream/odbc/timestream_column.h"
#include "timestream/odbc/meta/column_meta.h"

/*@*/
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/timestream-query/model/Row.h>

using Aws::TimestreamQuery::Model::Row;

namespace timestream {
namespace odbc {
/**
 * Query result cursor.
 */
class TimestreamCursor {
 public:
  /**
   * Constructor.
   * @param rowVec Aws Row vector.
   * @param columnMetadataVec Column metadata vector.
   */
  TimestreamCursor(const Aws::Vector< Row > rowVec,
                   const meta::ColumnMetaVector& columnMetadataVec);

  /**
   * Destructor.
   */
  ~TimestreamCursor();

  /**
   * Move cursor to the next result row.
   *
   * @return False if data update required or no more data.
   */
  bool Increment();

  /**
   * Check if the iterator has data.
   *
   * @return True if the iterator has data.
   */
  bool HasData() const;

  /**
   * Get column number in a row.
   *
   * @return number of columns.
   */
  int32_t GetColumnSize() const {
    return columnMetadataVec_.size();
  }

  /**
   * Read column data and store it in application data buffer.
   *
   * @param columnIdx Column index.
   * @param dataBuf Application data buffer.
   * @return Conversion result.
   */
  app::ConversionResult::Type ReadColumnToBuffer(
      uint32_t columnIdx, app::ApplicationDataBuffer& dataBuf);

 private:
  IGNITE_NO_COPY_ASSIGNMENT(TimestreamCursor);

  /**
   * Get columns by its index.
   *
   * Column indexing starts at 1.
   *
   * @note This operation is private because it's unsafe to use:
   *       It is neccessary to ensure that column is discovered prior
   *       to calling this method using EnsureColumnDiscovered().
   *
   * @param columnIdx Column index.
   * @return Reference to specified column.
   */
  TimestreamColumn& GetColumn(uint32_t columnIdx) {
    return columns_[columnIdx - 1];
  }

  /**
   * Ensure that column data is discovered.
   *
   * @param columnIdx Column index.
   * @return True if the column is discovered and false if it can not
   * be discovered.
   */
  bool EnsureColumnDiscovered(uint32_t columnIdx);

  /** Resultset rows */
  const Aws::Vector< Row > rowVec_;

  /** The iterator to beginning of cursor */
  Aws::Vector< Row >::const_iterator iterator_;

  /** The column metadata vector*/
  const meta::ColumnMetaVector& columnMetadataVec_;

  /** Columns. */
  std::vector< TimestreamColumn > columns_;

  /* current iterator position, start from 1 when used */
  int curPos_;
};
}  // namespace odbc
}  // namespace timestream

#endif  //_TIMESTREAM_ODBC_IGNITE_CURSOR
