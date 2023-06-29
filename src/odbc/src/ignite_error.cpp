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

#include <timestream/odbc/utils.h>
#include <timestream/odbc/ignite_error.h>

#include <utility>

using namespace timestream::odbc::common;

namespace timestream {
namespace odbc {
void IgniteError::ThrowIfNeeded(const IgniteError& err) {
  if (err.code != IGNITE_SUCCESS)
    throw err;
}

IgniteError::IgniteError() : code(IGNITE_SUCCESS), msg(NULL) {
  // No-op.
}

IgniteError::IgniteError(int32_t code) : code(code), msg(NULL) {
}

IgniteError::IgniteError(int32_t code, const char* msg)
    : code(code), msg(CopyChars(msg)) {
  // No-op.
}

IgniteError::IgniteError(const IgniteError& other)
    : code(other.code), msg(CopyChars(other.msg)) {
  // No-op.
}

IgniteError& IgniteError::operator=(const IgniteError& other) {
  if (this != &other) {
    IgniteError tmp(other);

    std::swap(code, tmp.code);
    std::swap(msg, tmp.msg);
  }

  return *this;
}

IgniteError::~IgniteError() IGNITE_NO_THROW {
  ReleaseChars(msg);
}

int32_t IgniteError::GetCode() const {
  return code;
}

const char* IgniteError::GetText() const IGNITE_NO_THROW {
  if (code == IGNITE_SUCCESS)
    return "Operation completed successfully.";
  else if (msg)
    return msg;
  else
    return "No additional information available.";
}

const char* IgniteError::what() const IGNITE_NO_THROW {
  return GetText();
}
}  // namespace odbc
}  // namespace timestream
