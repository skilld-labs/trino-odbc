/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modifications Copyright Amazon.com, Inc. or its affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timestream/odbc/environment.h"

#include <cstdlib>

#include "timestream/odbc/connection.h"
#include "timestream/odbc/system/odbc_constants.h"

namespace timestream {
namespace odbc {
Environment::Environment()
    : connections(), odbcVersion(SQL_OV_ODBC3), odbcNts(SQL_TRUE) {
}

Environment::~Environment() {
  // No-op.
}

Connection* Environment::CreateConnection() {
  Connection* connection;

  IGNITE_ODBC_API_CALL(InternalCreateConnection(connection));

  return connection;
}

void Environment::DeregisterConnection(Connection* conn) {
  connections.erase(conn);
}

SqlResult::Type Environment::InternalCreateConnection(Connection*& connection) {
  LOG_DEBUG_MSG("InternalCreateConnection is called");
  connection = new Connection(this);

  if (!connection) {
    AddStatusRecord(SqlState::SHY001_MEMORY_ALLOCATION, "Not enough memory.");

    return SqlResult::AI_ERROR;
  }

  connections.insert(connection);

  return SqlResult::AI_SUCCESS;
}

void Environment::SetAttribute(int32_t attr, void* value, int32_t len) {
  IGNITE_ODBC_API_CALL(InternalSetAttribute(attr, value, len));
}

SqlResult::Type Environment::InternalSetAttribute(int32_t attr, void* value,
                                                  int32_t len) {
  LOG_DEBUG_MSG("InternalSetAttribute is called with attr is " << attr);
  IGNITE_UNUSED(len);

  EnvironmentAttribute::Type attribute = EnvironmentAttributeToInternal(attr);

  switch (attribute) {
    case EnvironmentAttribute::ODBC_VERSION: {
      int32_t version =
          static_cast< int32_t >(reinterpret_cast< intptr_t >(value));

      if (version == SQL_OV_ODBC2 || version == SQL_OV_ODBC3) {
        odbcVersion = version;
        LOG_INFO_MSG("ODBC version has been set to ODBC " << version);
      } else {
        std::stringstream ss;
        ss << "The value of ODBC version(" << version
           << ") is not supported and the default value(" << odbcVersion
           << ") will be used.";

        AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED, ss.str().data(),
                        timestream::odbc::LogLevel::Type::WARNING_LEVEL);

        return SqlResult::AI_SUCCESS_WITH_INFO;
      }

      return SqlResult::AI_SUCCESS;
    }

    case EnvironmentAttribute::OUTPUT_NTS: {
      int32_t nts = static_cast< int32_t >(reinterpret_cast< intptr_t >(value));

      if (nts != odbcNts) {
        AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                        "Only null-termination of strings is supported.",
                        timestream::odbc::LogLevel::Type::WARNING_LEVEL);

        return SqlResult::AI_SUCCESS_WITH_INFO;
      }

      return SqlResult::AI_SUCCESS;
    }

    case EnvironmentAttribute::UNKNOWN:
    default:
      break;
  }

  AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                  "Attribute is not supported.");

  return SqlResult::AI_ERROR;
}

void Environment::GetAttribute(int32_t attr,
                               app::ApplicationDataBuffer& buffer) {
  IGNITE_ODBC_API_CALL(InternalGetAttribute(attr, buffer));
}

SqlResult::Type Environment::InternalGetAttribute(
    int32_t attr, app::ApplicationDataBuffer& buffer) {
  LOG_DEBUG_MSG("InternalGetAttribute is called with attr is " << attr);
  EnvironmentAttribute::Type attribute = EnvironmentAttributeToInternal(attr);

  switch (attribute) {
    case EnvironmentAttribute::ODBC_VERSION: {
      buffer.PutInt32(odbcVersion);

      return SqlResult::AI_SUCCESS;
    }

    case EnvironmentAttribute::OUTPUT_NTS: {
      buffer.PutInt32(odbcNts);

      return SqlResult::AI_SUCCESS;
    }

    case EnvironmentAttribute::UNKNOWN:
    default:
      break;
  }

  AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                  "Attribute is not supported.");

  return SqlResult::AI_ERROR;
}
}  // namespace odbc
}  // namespace timestream
