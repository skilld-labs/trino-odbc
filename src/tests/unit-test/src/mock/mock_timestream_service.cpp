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

#include <aws/core/Aws.h>
#include <aws/timestream-query/model/QueryResult.h>
#include <aws/timestream-query/TimestreamQueryErrors.h>
#include <aws/core/utils/Outcome.h>
#include <aws/timestream-query/model/Row.h>
#include <aws/timestream-query/model/Datum.h>
#include <aws/timestream-query/model/ColumnInfo.h>

#include <mock/mock_timestream_service.h>

namespace timestream {
namespace odbc {

std::mutex MockTimestreamService::mutex_;
MockTimestreamService* MockTimestreamService::instance_ = nullptr;
int MockTimestreamService::token = 0;
int MockTimestreamService::errorToken = 0;

void MockTimestreamService::CreateMockTimestreamService() {
  if (!instance_) {
    std::lock_guard< std::mutex > lock(mutex_);
    if (!instance_) {
      instance_ = new MockTimestreamService;
    }
  }
}

void MockTimestreamService::DestoryMockTimestreamService() {
  if (instance_) {
    std::lock_guard< std::mutex > lock(mutex_);
    if (instance_) {
      delete instance_;
      instance_ = nullptr;
    }
  }
}

MockTimestreamService ::~MockTimestreamService() {
  // No-op
}

void MockTimestreamService::AddCredential(const Aws::String& keyId,
                                          const Aws::String& secretKey) {
  credMap_[keyId] = secretKey;
}

void MockTimestreamService::RemoveCredential(const Aws::String& keyId) {
  credMap_.erase(keyId);
}

bool MockTimestreamService::Authenticate(const Aws::String& keyId,
                                         const Aws::String& secretKey) {
  auto itr = credMap_.find(keyId);
  if (itr == credMap_.end() || itr->second != secretKey) {
    return false;
  }
  return true;
}

// Setup QueryResult for mockTables
void MockTimestreamService::SetupResultForMockTable(
    Aws::TimestreamQuery::Model::QueryResult& result) {
  Aws::TimestreamQuery::Model::ColumnInfo firstColumn;
  firstColumn.SetName("measure");
  Aws::TimestreamQuery::Model::Type stringType;
  stringType.SetScalarType(Aws::TimestreamQuery::Model::ScalarType::VARCHAR);
  firstColumn.SetType(stringType);

  Aws::TimestreamQuery::Model::ColumnInfo secondColumn;
  secondColumn.SetName("time");
  Aws::TimestreamQuery::Model::Type timeType;
  timeType.SetScalarType(Aws::TimestreamQuery::Model::ScalarType::TIMESTAMP);
  secondColumn.SetType(timeType);
  result.AddColumnInfo(firstColumn);
  result.AddColumnInfo(secondColumn);

  Aws::TimestreamQuery::Model::Datum measure;
  measure.SetScalarValue("cpu_usage");
  Aws::TimestreamQuery::Model::Datum time1;
  time1.SetScalarValue("2022-11-09 23:52:51.554000000");
  Aws::TimestreamQuery::Model::Datum time2;
  time2.SetScalarValue("2022-11-10 23:53:51.554000000");
  Aws::TimestreamQuery::Model::Datum time3;
  time3.SetScalarValue("2022-11-11 23:54:51.554000000");

  Aws::TimestreamQuery::Model::Row row1;
  row1.AddData(measure);
  row1.AddData(time1);

  Aws::TimestreamQuery::Model::Row row2;
  row2.AddData(measure);
  row2.AddData(time2);

  Aws::TimestreamQuery::Model::Row row3;
  row3.AddData(measure);
  row3.AddData(time3);

  result.AddRows(row1);
  result.AddRows(row2);
  result.AddRows(row3);
}

// This function simulates AWS Timestream service. It provides
// simple result without the need of parsing the query. Update
// this function if new query needs to be handled.
Aws::TimestreamQuery::Model::QueryOutcome MockTimestreamService::HandleQueryReq(
    const Aws::TimestreamQuery::Model::QueryRequest& request) {
  if (request.GetQueryString() == "SELECT 1") {
    // set up QueryResult
    Aws::TimestreamQuery::Model::QueryResult result;
    Aws::TimestreamQuery::Model::Datum datum;
    datum.SetScalarValue("1");

    Aws::TimestreamQuery::Model::Row row;
    row.AddData(datum);

    result.AddRows(row);
    return Aws::TimestreamQuery::Model::QueryOutcome(result);
  } else if (request.GetQueryString()
             == "select measure, time from mockDB.mockTable") {
    Aws::TimestreamQuery::Model::QueryResult result;
    SetupResultForMockTable(result);
    return Aws::TimestreamQuery::Model::QueryOutcome(result);
  } else if (request.GetQueryString()
             == "select measure, time from mockDB.mockTable10000") {
    Aws::TimestreamQuery::Model::QueryResult result;
    SetupResultForMockTable(result);

    // for pagination test
    result.SetNextToken(std::to_string(++token));
    return Aws::TimestreamQuery::Model::QueryOutcome(result);
  } else if (request.GetQueryString()
             == "select measure, time from mockDB.mockTable10Error") {
    Aws::TimestreamQuery::Model::QueryResult result;
    SetupResultForMockTable(result);

    // for pagination test
    if (errorToken < 3) {
      result.SetNextToken(std::to_string(++errorToken));
      return Aws::TimestreamQuery::Model::QueryOutcome(result);
    } else {
      Aws::TimestreamQuery::TimestreamQueryError error(
          Aws::Client::AWSError< Aws::Client::CoreErrors >(
              Aws::Client::CoreErrors::UNKNOWN, false));

      return Aws::TimestreamQuery::Model::QueryOutcome(error);
    }
  } else {
    Aws::TimestreamQuery::TimestreamQueryError error(
        Aws::Client::AWSError< Aws::Client::CoreErrors >(
            Aws::Client::CoreErrors::UNKNOWN, false));

    return Aws::TimestreamQuery::Model::QueryOutcome(error);
  }
}
}  // namespace odbc
}  // namespace timestream
