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

#include "timestream_writer.h"

#include <aws/timestream-write/model/Record.h>
#include <aws/timestream-write/model/WriteRecordsRequest.h>
#include <aws/timestream-write/model/MeasureValueType.h>
#include <aws/timestream-write/model/RejectedRecordsException.h>
#include <aws/timestream-write/model/RejectedRecord.h>

#include "metadata-creator/computer_table_creater.h"

#include <time.h>

#if defined(_WIN32)
#include <winsock.h>
#else
#include <sys/time.h>
#endif
#include <iostream>

using Aws::TimestreamWrite::Model::MeasureValueType;
using Aws::TimestreamWrite::Model::Record;
using Aws::TimestreamWrite::Model::RejectedRecord;
using Aws::TimestreamWrite::Model::RejectedRecordsException;
using Aws::TimestreamWrite::Model::WriteRecordsRequest;

// needed to avoid misreplace
#ifdef GetMessage
#undef GetMessage
#endif

namespace timestream {
namespace odbc {

#if defined(_WIN32)
#include <chrono>

int gettimeofday(struct timeval* tp, struct timezone* tzp) {
  namespace sc = std::chrono;
  sc::system_clock::duration d = sc::system_clock::now().time_since_epoch();
  sc::seconds s = sc::duration_cast< sc::seconds >(d);
  tp->tv_sec = s.count();
  tp->tv_usec = sc::duration_cast< sc::microseconds >(d - s).count();

  return 0;
}
#endif  // _WIN32

std::shared_ptr< MeasureMetadataCreater >
TimestreamWriter::CreateMetadataCreater(Aws::String tableType) {
  std::transform(tableType.begin(), tableType.end(), tableType.begin(),
                 toupper);
  if (tableType == "COMPUTER") {
    return std::make_shared< ComputerTableCreater >();
  } else {
    std::cerr << "No table metadata creator is found for " << tableType
              << std::endl;
    return nullptr;
  }
}

bool TimestreamWriter::WriteSingleValueRecords(const Aws::String& tableType,
                                               const Aws::String& database,
                                               const Aws::String& table,
                                               int loopNum) {
  std::shared_ptr< MeasureMetadataCreater > creater =
      CreateMetadataCreater(tableType);
  if (!creater) {
    std::cerr << "Table metadata creater could not be found" << std::endl;
    return false;
  }

  // create dimensions
  Aws::Vector< Dimension > dimensions;
  creater->CreateDimensions(dimensions);

  // create records
  Aws::Vector< Record > records;
  creater->CreateRecords(dimensions, records);

  for (int i = 0; i < loopNum; i++) {
    // get current time
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long long time =
        (static_cast< long long >(tp.tv_sec) * 1000L) + (tp.tv_usec / 1000);

    // fill timestamp for each record
    for (auto& itr : records) {
      itr.WithTime(std::to_string(time));
    }

    // fill record values
    (creater->GetRecordValueAssignFunPtr())(records, i + 1);

    // create a write request
    WriteRecordsRequest writeRecordsRequest;
    writeRecordsRequest.WithDatabaseName(database).WithTableName(table);

    // set records in this request
    writeRecordsRequest.WithRecords(records);

    try {
      Aws::TimestreamWrite::Model::WriteRecordsOutcome outcome =
          client_->WriteRecords(writeRecordsRequest);
      if (!outcome.IsSuccess()) {
        std::cout << "Error msg is " << outcome.GetError().GetMessage()
                  << std::endl;
        return false;
      }
    } catch (RejectedRecordsException e) {
      for (RejectedRecord rejectedRecord : e.GetRejectedRecords()) {
        std::cout << "Rejected Index " << rejectedRecord.GetRecordIndex()
                  << ": " << rejectedRecord.GetReason() << std::endl;
      }
      return false;
    }
  }
  return true;
}

bool TimestreamWriter::WriteMultiValueRecords(const Aws::String& tableType,
                                              const Aws::String& database,
                                              const Aws::String& table,
                                              int loopNum) {
  std::shared_ptr< MeasureMetadataCreater > creater =
      CreateMetadataCreater(tableType);
  if (!creater) {
    std::cerr << "Table metadata creater could not be found" << std::endl;
    return false;
  }

  // create dimensions
  Aws::Vector< Dimension > dimensions;
  creater->CreateDimensions(dimensions);

  // create measure values
  Aws::Vector< MeasureValue > values;
  creater->CreateMeasureValues(values);

  // create common attributes for all records
  Record commonAttributes;
  commonAttributes.WithDimensions(dimensions);

  // create records with measure names
  Record multiMeasure;
  multiMeasure.WithMeasureName(creater->GetMetricName())
      .WithMeasureValueType(MeasureValueType::MULTI);

  for (int i = 0; i < loopNum; i++) {
    // fill timestamp to common attributes
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long long time =
        static_cast< long long >(tp.tv_sec) * 1000L + tp.tv_usec / 1000;
    commonAttributes.WithTime(std::to_string(time));

    // fill record values
    (creater->GetMeasureValueAssignFunPtr())(values, i + 1);
    multiMeasure.WithMeasureValues(values);

    // create a write request
    WriteRecordsRequest writeRecordsRequest;
    writeRecordsRequest.WithDatabaseName(database)
        .WithTableName(table)
        .WithCommonAttributes(commonAttributes);

    // set records in this request
    writeRecordsRequest.AddRecords(multiMeasure);

    try {
      Aws::TimestreamWrite::Model::WriteRecordsOutcome outcome =
          client_->WriteRecords(writeRecordsRequest);
      if (!outcome.IsSuccess()) {
        std::cout << "Error msg is " << outcome.GetError().GetMessage()
                  << std::endl;
        return false;
      }
    } catch (RejectedRecordsException e) {
      for (RejectedRecord rejectedRecord : e.GetRejectedRecords()) {
        std::cout << "Rejected Index " << rejectedRecord.GetRecordIndex()
                  << ": " << rejectedRecord.GetReason() << std::endl;
      }
      return false;
    }
  }
  return true;
}
}  // namespace odbc
}  // namespace timestream
