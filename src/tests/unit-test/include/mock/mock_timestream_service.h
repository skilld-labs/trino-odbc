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

#ifndef _MOCK_TRINO_SERVICE
#define _MOCK_TRINO_SERVICE

/*@*/
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/trino-query/TrinoQueryClient.h>
#include <aws/trino-query/model/QueryRequest.h>

namespace trino {
namespace odbc {
/**
 * Mock Trino service for unit test
 */
class MockTrinoService {
 public:
  /**
   * Create the singleton object.
   */
  static void CreateMockTrinoService();

  /**
   * Destory the singleton object.
   */
  static void DestoryMockTrinoService();

  /**
   * Get the singleton object pointer.
   */
  static MockTrinoService* GetInstance() {
    return instance_;
  }

  /**
   * Destructor.
   */
  ~MockTrinoService();

  /**
   * Add credentials configured by user before testcase starts
   *
   * @param keyId Access key Id
   * @param secretKey Secret access key
   */
  void AddCredential(const Aws::String& keyId, const Aws::String& secretKey);

  /**
   * Remove credentials configured by user
   *
   * @param keyId Access key Id
   */
  void RemoveCredential(const Aws::String& keyId);

  /**
   * Get credential map
   *
   * @return The credential map
   */
  std::map< Aws::String, Aws::String > GetCredentialMap() {
    return credMap_;
  }

  /**
   * Verify credentials provided by user
   *
   * @param keyId Access key Id
   * @param secretKey Secret access key
   */
  bool Authenticate(const Aws::String& keyId, const Aws::String& secretKey);

  /**
   * Handle query request from query client
   *
   * @param request Query request
   */
  Aws::TrinoQuery::Model::QueryOutcome HandleQueryReq(
      const Aws::TrinoQuery::Model::QueryRequest& request);

 private:
  /**
   * Constructor.
   */
  MockTrinoService() {
  }

  void SetupResultForMockTable(
      Aws::TrinoQuery::Model::QueryResult& result);

  static std::mutex mutex_;
  static MockTrinoService* instance_;
  std::map< Aws::String, Aws::String >
      credMap_;  // credentials configured by user
  static int token;
  static int errorToken;
};
}  // namespace odbc
}  // namespace trino

#endif
