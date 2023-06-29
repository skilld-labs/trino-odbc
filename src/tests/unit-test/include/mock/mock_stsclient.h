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

#ifndef _IGNITE_ODBC_MOCK_STSCLIENT
#define _IGNITE_ODBC_MOCK_STSCLIENT

#include "aws/sts/STSClient.h"

using namespace Aws::STS;

namespace timestream {
namespace odbc {
/**
 * Mock Connection so function level test could be done against Connection.
 */
class MockSTSClient : public STSClient {
 public:
  /**
   * Constructor.
   */
  MockSTSClient() = default;

  /**
   * Destructor.
   */
  ~MockSTSClient() = default;

  /**
   * Handle AssumeRoleWithSAMLRequest to generate a mocked outcome with
   * credentials
   *
   * @param request The AssumeRoleWithSAMLRequest.
   * @return Mocked outcome.
   */
  virtual Model::AssumeRoleWithSAMLOutcome AssumeRoleWithSAML(
      const Model::AssumeRoleWithSAMLRequest &request) const;
};
}  // namespace odbc
}  // namespace timestream

#endif  //_IGNITE_ODBC_MOCK_STSCLIENT
