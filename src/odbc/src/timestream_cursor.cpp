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

#include "timestream/odbc/timestream_cursor.h"

namespace timestream {
namespace odbc {
TimestreamCursor::TimestreamCursor(
    const Aws::Vector< Row > rowVec,
    const meta::ColumnMetaVector& columnMetadataVec)
    : rowVec_(rowVec),
      iterator_(rowVec_.begin()),
      columnMetadataVec_(columnMetadataVec),
      curPos_(0) {
  // No-op.
}

TimestreamCursor::~TimestreamCursor() {
  // No-op.
}

// After Increment, the "curPos_"th iterator is being handled
bool TimestreamCursor::Increment() {
  LOG_DEBUG_MSG("Increment is called");

  if (curPos_ > 0) {
    ++iterator_;
  }
  curPos_++;
  return curPos_ <= rowVec_.size();
}

bool TimestreamCursor::HasData() const {
  return curPos_ <= rowVec_.size();
}

app::ConversionResult::Type TimestreamCursor::ReadColumnToBuffer(
    uint32_t columnIdx, app::ApplicationDataBuffer& dataBuf) {
  LOG_DEBUG_MSG("ReadColumnToBuffer is called");
  if (!EnsureColumnDiscovered(columnIdx)) {
    LOG_ERROR_MSG("columnIdx could not be discovered for index " << columnIdx);
    return app::ConversionResult::Type::AI_FAILURE;
  }

  TimestreamColumn& column = GetColumn(columnIdx);
  const Datum& datum = iterator_->GetData()[columnIdx-1];
  return column.ReadToBuffer(datum, dataBuf);
}

bool TimestreamCursor::EnsureColumnDiscovered(uint32_t columnIdx) {
  LOG_DEBUG_MSG("EnsureColumnDiscovered is called for column " << columnIdx);
  if (columnIdx > columnMetadataVec_.size() || columnIdx < 1) {
    LOG_ERROR_MSG("columnIdx out of range for index " << columnIdx);
    return false;
  }

  LOG_DEBUG_MSG("columns_ size is " << columns_.size()
                                    << ", columnMetadataVec_ size is "
                                    << columnMetadataVec_.size());
  if (columns_.size() == columnMetadataVec_.size()) {
    return true;
  }

  uint32_t index = columns_.size();
  while (index < columnIdx) {
    TimestreamColumn newColumn(index, columnMetadataVec_[index]);

    columns_.push_back(newColumn);
    index++;
  }

  return true;
}
}  // namespace odbc
}  // namespace timestream
