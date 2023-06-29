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

#ifndef _IGNITE_ODBC_TEST_TEST_UTILS
#define _IGNITE_ODBC_TEST_TEST_UTILS

#ifdef _WIN32
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>

#include "timestream/odbc/utils.h"

#define ODBC_THROW_ON_ERROR(ret, type, handle)         \
  if (!SQL_SUCCEEDED(ret)) {                           \
    throw timestream_test::GetOdbcError(type, handle); \
  }

#define ODBC_FAIL_ON_ERROR(ret, type, handle)                       \
  if (!SQL_SUCCEEDED(ret)) {                                        \
    BOOST_FAIL(timestream_test::GetOdbcErrorMessage(type, handle)); \
  }

#define ODBC_FAIL_ON_ERROR1(ret, type, handle, msg)               \
  if (!SQL_SUCCEEDED(ret)) {                                      \
    BOOST_FAIL(timestream_test::GetOdbcErrorMessage(type, handle) \
               + ", msg = " + msg);                               \
  }

#define MUTE_TEST_FOR_TEAMCITY                                            \
  if (jetbrains::teamcity::underTeamcity()) {                             \
    BOOST_TEST_MESSAGE(                                                   \
        "Muted on TeamCity because of periodical non-critical failures"); \
    BOOST_CHECK(jetbrains::teamcity::underTeamcity());                    \
    return;                                                               \
  }

/**
 * Copy std::string to buffer.
 *
 * @param dst Destination buffer.
 * @param src Source std::string.
 * @param n Copy at most n bytes of src.
 */
inline void CopyStringToBuffer(char* dst, const std::string& src, size_t n) {
  if (n == 0) {
    return;
  }

  using std::min;
  size_t size = min(src.size(), n - 1);

  memset(dst + size, '\0', n - size);
  memcpy(dst, src.c_str(), size);
}

/**
 * Client ODBC erorr.
 */
class OdbcClientError : public std::exception {
 public:
  /**
   * Constructor
   *
   * @param sqlstate SQL state.
   * @param message Error message.
   */
  OdbcClientError(const std::string& sqlstate, const std::string& message)
      : sqlstate(sqlstate), message(message) {
    // No-op.
  }

  /**
   * Destructor.
   */
  virtual ~OdbcClientError() IGNITE_NO_THROW {
    // No-op.
  }

  /**
   * Implementation of the standard std::exception::what() method.
   * Synonym for GetText() method.
   *
   * @return Error message string.
   */
  virtual const char* what() const IGNITE_NO_THROW {
    return message.c_str();
  }

  /** SQL state. */
  std::string sqlstate;

  /** Error message. */
  std::string message;
};

namespace timestream_test {
/** Read buffer size. */
enum { ODBC_BUFFER_SIZE = 1024 };

/**
 * Extract error.
 *
 * @param handleType Type of the handle.
 * @param handle Handle.
 * @return Error.
 */
OdbcClientError GetOdbcError(SQLSMALLINT handleType, SQLHANDLE handle);

/**
 * Extract error state.
 *
 * @param handleType Type of the handle.
 * @param handle Handle.
 * @param idx Error record index.
 * @return Error state.
 */
std::string GetOdbcErrorState(SQLSMALLINT handleType, SQLHANDLE handle,
                              int idx = 1);

/**
 * Extract error message.
 *
 * @param handleType Type of the handle.
 * @param handle Handle.
 * @param idx Error record index.
 * @return Error message.
 */
std::string GetOdbcErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle,
                                int idx = 1);
}  // namespace timestream_test

#endif  // _IGNITE_ODBC_TEST_TEST_UTILS
