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

#include "timestream/odbc/diagnostic/diagnosable_adapter.h"

#include "timestream/odbc/connection.h"
#include "timestream/odbc/log.h"
#include "ignite/odbc/odbc_error.h"

namespace timestream {
namespace odbc {
namespace diagnostic {
void DiagnosableAdapter::AddStatusRecord(
    SqlState::Type sqlState, const std::string& message,
    timestream::odbc::LogLevel::Type logLevel, int32_t rowNum,
    int32_t columnNum) {
  WRITE_LOG_MSG("Adding new record: " << message << ", rowNum: " << rowNum
                                      << ", columnNum: " << columnNum,
                logLevel);

  if (connection) {
    diagnosticRecords.AddStatusRecord(
        connection->CreateStatusRecord(sqlState, message, rowNum, columnNum));
  } else {
    diagnosticRecords.AddStatusRecord(
        DiagnosticRecord(sqlState, message, "", "", rowNum, columnNum));
  }
}

void DiagnosableAdapter::AddStatusRecord(
    SqlState::Type sqlState, const std::string& message,
    timestream::odbc::LogLevel::Type logLevel) {
  AddStatusRecord(sqlState, message, logLevel, 0, 0);
}

void DiagnosableAdapter::AddStatusRecord(const std::string& message) {
  AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, message);
}

void DiagnosableAdapter::AddStatusRecord(const ignite::odbc::OdbcError& err) {
  AddStatusRecord(err.GetStatus(), err.GetErrorMessage(),
                  timestream::odbc::LogLevel::Type::ERROR_LEVEL, 0, 0);
}

void DiagnosableAdapter::AddStatusRecord(const DiagnosticRecord& rec) {
  diagnosticRecords.AddStatusRecord(rec);
}
}  // namespace diagnostic
}  // namespace odbc
}  // namespace timestream
