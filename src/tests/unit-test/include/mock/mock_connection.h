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

#ifndef _IGNITE_ODBC_MOCK_CONNECTION
#define _IGNITE_ODBC_MOCK_CONNECTION

#include "timestream/odbc/connection.h"
#include "mock/mock_statement.h"

namespace timestream {
namespace odbc {
/**
 * Mock Connection so function level test could be done against Connection.
 */
class MockConnection : public Connection {
 public:
  /**
   * Constructor.
   */
  MockConnection(Environment* env);

  /**
   * Destructor.
   */
  ~MockConnection();

  MockStatement* CreateStatement();

 private:
  /**
   * Create statement associated with the connection.
   * Internal call.
   *
   * @param statement Pointer to valid instance on success or NULL on failure.
   * @return Operation result.
   */
  virtual SqlResult::Type InternalCreateStatement(MockStatement*& statement);

  /**
   * Create MockTimestreamQueryClient object.
   *
   * @param credentials Aws IAM credentials.
   * @param clientCfg Aws client configuration.
   * @param cfg connection configuration.
   * @return a shared_ptr to created MockTimestreamQueryClient object.
   */
  virtual std::shared_ptr< Aws::TimestreamQuery::TimestreamQueryClient >
  CreateTSQueryClient(const Aws::Auth::AWSCredentials& credentials,
                      const Aws::Client::ClientConfiguration& clientCfg);

  /**
   * Create MockHttpClient object.
   *
   * @return a shared_ptr to created MockHttpClient object.
   */
  virtual std::shared_ptr< Aws::Http::HttpClient > GetHttpClient();

  /**
   * Create MockSTSClient object.
   *
   * @return a shared_ptr to created MockSTSClient object.
   */
  virtual std::shared_ptr< Aws::STS::STSClient > GetStsClient();
};
}  // namespace odbc
}  // namespace timestream

#endif  //_IGNITE_ODBC_MOCK_CONNECTION
