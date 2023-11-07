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

/*@*/
#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h>
#include <aws/timestream-query/model/QueryResult.h>
#include <aws/timestream-query/model/Row.h>
#include <aws/timestream-query/model/Datum.h>
#include <aws/timestream-query/TimestreamQueryErrors.h>

#include <mock/mock_timestream_query_client.h>
#include <mock/mock_timestream_service.h>

namespace timestream {
namespace odbc {
// All the working logic is done by the singleton MockTimestreamService object.
Aws::TimestreamQuery::Model::QueryOutcome MockTimestreamQueryClient::Query(
    const Aws::TimestreamQuery::Model::QueryRequest &request) const {
  // authenticate first
  if (!MockTimestreamService::GetInstance()->Authenticate(
          credentials_.GetAWSAccessKeyId(), credentials_.GetAWSSecretKey())) {
    Aws::TimestreamQuery::TimestreamQueryError error(
        Aws::Client::AWSError< Aws::Client::CoreErrors >(
            Aws::Client::CoreErrors::INVALID_ACCESS_KEY_ID, false));

    return Aws::TimestreamQuery::Model::QueryOutcome(error);
  }

  return MockTimestreamService::GetInstance()->HandleQueryReq(request);
}

}  // namespace odbc
}  // namespace timestream
