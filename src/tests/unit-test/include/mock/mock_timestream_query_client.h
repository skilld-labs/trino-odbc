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

#ifndef _MOCK_TRINO_QUERY_CLIENT
#define _MOCK_TRINO_QUERY_CLIENT

/*@*/
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/trino-query/TrinoQueryClient.h>
#include <aws/trino-query/model/QueryRequest.h>

namespace trino {
namespace odbc {
/**
 * Mock TrinoQueryClient so its behavior could be controlled by us.
 * All interfaces should be kept same as TrinoQueryClient.
 */
class MockTrinoQueryClient
    : public Aws::TrinoQuery::TrinoQueryClient {
 public:
  /**
   * Constructor.
   */
  MockTrinoQueryClient(
      const Aws::Auth::AWSCredentials &credentials,
      const Aws::Client::ClientConfiguration &clientConfiguration =
          Aws::Client::ClientConfiguration())
      : Aws::TrinoQuery::TrinoQueryClient(credentials,
                                                    clientConfiguration),
        credentials_(credentials),
        clientConfiguration_(clientConfiguration) {
  }

  /**
   * Destructor.
   */
  ~MockTrinoQueryClient() {
  }

  /**
   * Run a query.
   *
   * @param request Aws QueryResult .
   * @return Operation outcome.
   */
  virtual Aws::TrinoQuery::Model::QueryOutcome Query(
      const Aws::TrinoQuery::Model::QueryRequest &request) const;

 private:
  Aws::Auth::AWSCredentials credentials_;
  Aws::Client::ClientConfiguration clientConfiguration_;
};
}  // namespace odbc
}  // namespace trino

#endif
