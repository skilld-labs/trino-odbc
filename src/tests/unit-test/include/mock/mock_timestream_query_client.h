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

#ifndef _MOCK_TIMESTREAM_QUERY_CLIENT
#define _MOCK_TIMESTREAM_QUERY_CLIENT

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/timestream-query/TimestreamQueryClient.h>
#include <aws/timestream-query/model/QueryRequest.h>

namespace timestream {
namespace odbc {
/**
 * Mock TimestreamQueryClient so its behavior could be controlled by us.
 * All interfaces should be kept same as TimestreamQueryClient.
 */
class MockTimestreamQueryClient
    : public Aws::TimestreamQuery::TimestreamQueryClient {
 public:
  /**
   * Constructor.
   */
  MockTimestreamQueryClient(
      const Aws::Auth::AWSCredentials &credentials,
      const Aws::Client::ClientConfiguration &clientConfiguration =
          Aws::Client::ClientConfiguration())
      : Aws::TimestreamQuery::TimestreamQueryClient(credentials,
                                                    clientConfiguration),
        credentials_(credentials),
        clientConfiguration_(clientConfiguration) {
  }

  /**
   * Destructor.
   */
  ~MockTimestreamQueryClient() {
  }

  /**
   * Run a query.
   *
   * @param request Aws QueryResult .
   * @return Operation outcome.
   */
  virtual Aws::TimestreamQuery::Model::QueryOutcome Query(
      const Aws::TimestreamQuery::Model::QueryRequest &request) const;

 private:
  Aws::Auth::AWSCredentials credentials_;
  Aws::Client::ClientConfiguration clientConfiguration_;
};
}  // namespace odbc
}  // namespace timestream

#endif
