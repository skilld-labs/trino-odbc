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

#ifndef _IGNITE_ODBC_MOCK_ENVIRONMENT
#define _IGNITE_ODBC_MOCK_ENVIRONMENT

#include "trino/odbc/environment.h"

namespace trino {
namespace odbc {
/**
 * Mock Environment so function level test could be done against Environment.
 */
class MockEnvironment : public Environment {
 public:
  /**
   * Constructor.
   */
  MockEnvironment();

  /**
   * Destructor.
   */
  ~MockEnvironment();

  /**
   * Get the number of connections created.
   */
  size_t ConnectionsNum() {
    return connections.size();
  }

 private:
  /**
   * Create connection associated with the environment.
   * Internal call.
   *
   * @return Pointer to valid instance on success or NULL on failure.
   * @return Operation result.
   */
  virtual SqlResult::Type InternalCreateConnection(Connection*& connection);
};
}  // namespace odbc
}  // namespace trino

#endif  //_IGNITE_ODBC_MOCK_ENVIRONMENT
