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

#include "test_utils.h"

#include <ignite/common/include/common/platform_utils.h>
#include <trino/odbc/utility.h>

#include <boost/test/unit_test.hpp>
#include <cassert>

using namespace trino::odbc;

namespace trino_test {
OdbcClientError GetOdbcError(SQLSMALLINT handleType, SQLHANDLE handle) {
  SQLWCHAR sqlstate[7] = {};
  SQLINTEGER nativeCode;

  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT reallen = 0;

  // On Windows, reallen is in bytes, on Linux reallen is in chars.
  // Can't rely on returned length.
  SQLGetDiagRec(handleType, handle, 1, sqlstate, &nativeCode, message,
                ODBC_BUFFER_SIZE, &reallen);

  return OdbcClientError(utility::SqlWcharToString(sqlstate),
                         utility::SqlWcharToString(message));
}

std::string GetOdbcErrorState(SQLSMALLINT handleType, SQLHANDLE handle,
                              int idx) {
  SQLWCHAR sqlstate[7] = {};
  SQLINTEGER nativeCode;

  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT reallen = 0;

  // On Windows, reallen is in bytes, on Linux reallen is in chars.
  // Can't rely on returned length.
  SQLGetDiagRec(handleType, handle, idx, sqlstate, &nativeCode, message,
                ODBC_BUFFER_SIZE, &reallen);

  return utility::SqlWcharToString(sqlstate);
}

std::string GetOdbcErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle,
                                int idx) {
  SQLWCHAR sqlstate[7] = {};
  SQLINTEGER nativeCode;

  SQLWCHAR message[ODBC_BUFFER_SIZE] = {};
  SQLSMALLINT reallen = 0;

  // On Windows, reallen is in bytes, on Linux reallen is in chars.
  // Can't rely on returned length.
  SQLGetDiagRec(handleType, handle, idx, sqlstate, &nativeCode, message,
                ODBC_BUFFER_SIZE, &reallen);

  std::string res = utility::SqlWcharToString(sqlstate);

  if (res.empty() || !message[0]) {
    res = "Cannot find ODBC error message";
  } else {
    res.append(": ").append(utility::SqlWcharToString(message));
  }

  return res;
}

std::string AppendPath(const std::string& base, const std::string& toAdd) {
  std::stringstream stream;

  stream << base << ignite::odbc::common::Fs << toAdd;

  return stream.str();
}
}  // namespace trino_test
