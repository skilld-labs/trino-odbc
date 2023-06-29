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

#include <mock/mock_environment.h>
#include <mock/mock_connection.h>

namespace timestream {
namespace odbc {
MockEnvironment::MockEnvironment() : Environment() {
  // No-op.
}

MockEnvironment::~MockEnvironment() {
  // No-op.
}

SqlResult::Type MockEnvironment::InternalCreateConnection(
    Connection*& connection) {
  connection = new MockConnection(this);

  if (!connection) {
    AddStatusRecord(SqlState::SHY001_MEMORY_ALLOCATION, "Not enough memory.");

    return SqlResult::AI_ERROR;
  }

  connections.insert(connection);
  return SqlResult::AI_SUCCESS;
}
}  // namespace odbc
}  // namespace timestream
