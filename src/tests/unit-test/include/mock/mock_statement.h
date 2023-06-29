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

#ifndef _IGNITE_ODBC_MOCK_STATEMENT
#define _IGNITE_ODBC_MOCK_STATEMENT

#include "timestream/odbc/statement.h"

namespace timestream {
namespace odbc {
/**
 * Mock Statement so function level test could be done against Statement.
 */
class MockStatement : public Statement {
 public:
  /**
   * Constructor.
   *
   * @param parent Connection associated with the statement.
   */
  MockStatement(Connection& parent);

  /**
   * Destructor.
   */
  ~MockStatement();
};
}  // namespace odbc
}  // namespace timestream

#endif  //_IGNITE_ODBC_MOCK_STATEMENT
