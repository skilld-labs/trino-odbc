/*
 * Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

#ifndef _IGNITE_ODBC_MULTI_MEASURE_METADATA_CREATER
#define _IGNITE_ODBC_MULTI_MEASURE_METADATA_CREATER

/*@*/
#include <aws/core/Aws.h>
#include <aws/timestream-write/model/Dimension.h>
#include <aws/timestream-write/model/Record.h>
#include <aws/timestream-write/model/MeasureValue.h>

using Aws::TimestreamWrite::Model::Dimension;
using Aws::TimestreamWrite::Model::MeasureValue;
using Aws::TimestreamWrite::Model::Record;

namespace timestream {
namespace odbc {

class MeasureMetadataCreater {
 public:
  MeasureMetadataCreater() = default;

  virtual ~MeasureMetadataCreater() = default;

  virtual void CreateDimensions(Aws::Vector< Dimension >& dimensions) = 0;
  virtual void CreateRecords(Aws::Vector< Dimension >& dimensions,
                             Aws::Vector< Record >& values) = 0;
  virtual void CreateMeasureValues(Aws::Vector< MeasureValue >& values) = 0;
  virtual const char* GetMetricName() = 0;

  typedef void (*RecordValueAssignFunPtr)(Aws::Vector< Record >& values,
                                          int index);
  virtual RecordValueAssignFunPtr GetRecordValueAssignFunPtr() = 0;

  typedef void (*MeasureValueAssignFunPtr)(Aws::Vector< MeasureValue >& values,
                                           int index);
  virtual MeasureValueAssignFunPtr GetMeasureValueAssignFunPtr() = 0;
};
}  // namespace odbc
}  // namespace timestream

#endif  //_IGNITE_ODBC_MULTI_MEASURE_METADATA_CREATER
