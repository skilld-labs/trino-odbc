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

#ifndef _TRINO_ODBC_DIAGNOSTIC_DIAGNOSABLE_ADAPTER
#define _TRINO_ODBC_DIAGNOSTIC_DIAGNOSABLE_ADAPTER

#include "ignite/odbc/diagnostic/diagnosable.h"
#include "ignite/odbc/odbc_error.h"

#define IGNITE_ODBC_API_CALL(...)         \
  diagnosticRecords.Reset();              \
  SqlResult::Type result = (__VA_ARGS__); \
  diagnosticRecords.SetHeaderRecord(result)

#define IGNITE_ODBC_API_CALL_ALWAYS_SUCCESS \
  diagnosticRecords.Reset();                \
  diagnosticRecords.SetHeaderRecord(SqlResult::AI_SUCCESS)

namespace trino {
namespace odbc {
class OdbcError;
class Connection;

namespace diagnostic {
/**
 * Diagnosable interface.
 */
class IGNITE_IMPORT_EXPORT DiagnosableAdapter
    : public ignite::odbc::diagnostic::Diagnosable {
 public:
  /**
   * Constructor.
   *
   * @param connection Pointer to connection. Used to create
   *     diagnostic records with connection info.
   */
  DiagnosableAdapter(const Connection* connection = 0)
      : connection(connection) {
    // No-op.
  }

  /**
   * Destructor.
   */
  virtual ~DiagnosableAdapter() {
    // No-op.
  }

  /**
   * Get diagnostic record.
   *
   * @return Diagnostic record.
   */
  virtual const DiagnosticRecordStorage& GetDiagnosticRecords() const {
    return diagnosticRecords;
  }

  /**
   * Get diagnostic record.
   *
   * @return Diagnostic record.
   */
  virtual DiagnosticRecordStorage& GetDiagnosticRecords() {
    return diagnosticRecords;
  }

  /**
   * Add new status record.
   *
   * @param sqlState SQL state.
   * @param message Message.
   * @param rowNum Associated row number.
   * @param columnNum Associated column number.
   */
  virtual void AddStatusRecord(SqlState::Type sqlState,
                               const std::string& message,
                               trino::odbc::LogLevel::Type logLevel,
                               int32_t rowNum, int32_t columnNum);

  /**
   * Add new status record.
   *
   * @param sqlState SQL state.
   * @param message Message.
   * @param logLevel Log level
   */
  virtual void AddStatusRecord(
      SqlState::Type sqlState, const std::string& message,
      trino::odbc::LogLevel::Type logLevel =
          trino::odbc::LogLevel::Type::ERROR_LEVEL);

  /**
   * Add new status record with SqlState::SHY000_GENERAL_ERROR state.
   *
   * @param message Message.
   */
  virtual void AddStatusRecord(const std::string& message);

  /**
   * Add new status record.
   *
   * @param err Error.
   */
  virtual void AddStatusRecord(const ignite::odbc::OdbcError& err);

  /**
   * Add new status record.
   *
   * @param rec Record.
   */
  virtual void AddStatusRecord(const DiagnosticRecord& rec);

 protected:
  /** Diagnostic records. */
  DiagnosticRecordStorage diagnosticRecords;

 private:
  /** Connection. */
  const Connection* connection;
};
}  // namespace diagnostic
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_DIAGNOSTIC_DIAGNOSABLE_ADAPTER
