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

#ifndef _TRINO_ODBC_IGNITE_COLUMN
#define _TRINO_ODBC_IGNITE_COLUMN

#include <stdint.h>
#include <trino/odbc/app/application_data_buffer.h>
#include "trino/odbc/meta/column_meta.h"
/*@*/
#include <aws/trino-query/model/Row.h>

using Aws::TrinoQuery::Model::Datum;
using Aws::TrinoQuery::Model::Row;
using trino::odbc::app::ApplicationDataBuffer;
using trino::odbc::app::ConversionResult;

namespace trino {
namespace odbc {
/**
 * Result set column.
 */
class TrinoColumn {
 public:
  /**
   * Constructor.
   *
   * @param columnIdx The column index, start from 0.
   * @param columnMetadata The column metadata.
   */
  TrinoColumn(
                   uint32_t columnIdx, const meta::ColumnMeta& columnMetadata);

  /**
   * Copy constructor.
   *
   * @param other Another instance.
   */
  TrinoColumn(const TrinoColumn& other) = default;

  /**
   * Copy operator.
   *
   * @param other Another instance.
   * @return This.
   */
  TrinoColumn& operator=(const TrinoColumn& other) = delete;

  /**
   * Destructor.
   */
  ~TrinoColumn() = default;

  /**
   * Read column data and store it in application data buffer.
   *
   * @param datum Aws datum which contains the result data.
   * @param dataBuf Application data buffer.
   * @return Operation result.
   */
  ConversionResult::Type ReadToBuffer(const Datum& datum,
                                      ApplicationDataBuffer& dataBuf) const;

 private:
  /**
   * Parse Aws Datum data and save result to dataBuf
   *
   * @param datum Aws datum which contains the result data
   * @param dataBuf Application data buffer.
   * @return Operation result.
   */
  ConversionResult::Type ParseDatum(const Datum& datum,
                                    ApplicationDataBuffer& dataBuf) const;

  /**
   * Parse scalar data type in datum and save result to dataBuf.
   *
   * @param datum Aws datum which contains the result data
   * @param dataBuf Application data buffer.
   * @return Operation result.
   */
  ConversionResult::Type ParseScalarType(const Datum& datum,
                                         ApplicationDataBuffer& dataBuf) const;

  /**
   * Parse TimeSeries data type in datum and save result to dataBuf.
   *
   * @param datum Aws datum which contains the result data
   * @param dataBuf Application data buffer.
   * @return Operation result.
   */
  ConversionResult::Type ParseTimeSeriesType(
      const Datum& datum, ApplicationDataBuffer& dataBuf) const;

  /**
   * Parse Array data type in datum and save result to dataBuf.
   *
   * @param datum Aws datum which contains the result data
   * @param dataBuf Application data buffer.
   * @return Operation result.
   */
  ConversionResult::Type ParseArrayType(const Datum& datum,
                                        ApplicationDataBuffer& dataBuf) const;

  /**
   * Parse Row data type in datum and save result to dataBuf.
   *
   * @param datum Aws datum which contains the result data
   * @param dataBuf Application data buffer.
   * @return Operation result.
   */
  ConversionResult::Type ParseRowType(const Datum& datum,
                                      ApplicationDataBuffer& dataBuf) const;

  /** The column index */
  uint32_t columnIdx_;

  /** The column metadata */
  const meta::ColumnMeta& columnMeta_;
};
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_IGNITE_COLUMN
