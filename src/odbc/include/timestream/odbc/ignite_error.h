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

/**
 * @file
 * Declares trino::odbc::IgniteError class.
 */

#ifndef _TRINO_ODBC_IGNITE_ERROR
#define _TRINO_ODBC_IGNITE_ERROR

#include <ignite/common/common.h>
#include <stdint.h>

#include <exception>
#include <sstream>

// Define can be removed once the duplicated code was removed
#ifndef _IGNITE_ERROR_MACRO
#define _IGNITE_ERROR_MACRO

#define IGNITE_ERROR_1(code, part1)                \
  {                                                \
    std::stringstream stream;                      \
    stream << (part1);                             \
    throw IgniteError(code, stream.str().c_str()); \
  }

#define IGNITE_ERROR_2(code, part1, part2)         \
  {                                                \
    std::stringstream stream;                      \
    stream << (part1) << (part2);                  \
    throw IgniteError(code, stream.str().c_str()); \
  }

#define IGNITE_ERROR_3(code, part1, part2, part3)  \
  {                                                \
    std::stringstream stream;                      \
    stream << (part1) << (part2) << (part3);       \
    throw IgniteError(code, stream.str().c_str()); \
  }

#define IGNITE_ERROR_FORMATTED_1(code, msg, key1, val1)    \
  {                                                        \
    std::stringstream stream;                              \
    stream << msg << " [" << key1 << "=" << (val1) << "]"; \
    throw IgniteError(code, stream.str().c_str());         \
  }

#define IGNITE_ERROR_FORMATTED_2(code, msg, key1, val1, key2, val2)       \
  {                                                                       \
    std::stringstream stream;                                             \
    stream << msg << " [" << key1 << "=" << (val1) << ", " << key2 << "=" \
           << (val2) << "]";                                              \
    throw IgniteError(code, stream.str().c_str());                        \
  }

#define IGNITE_ERROR_FORMATTED_3(code, msg, key1, val1, key2, val2, key3, \
                                 val3)                                    \
  {                                                                       \
    std::stringstream stream;                                             \
    stream << msg << " [" << key1 << "=" << (val1) << ", " << key2 << "=" \
           << (val2) << ", " << key3 << "=" << (val3) << "]";             \
    throw IgniteError(code, stream.str().c_str());                        \
  }

#define IGNITE_ERROR_FORMATTED_4(code, msg, key1, val1, key2, val2, key3,    \
                                 val3, key4, val4)                           \
  {                                                                          \
    std::stringstream stream;                                                \
    stream << msg << " [" << key1 << "=" << (val1) << ", " << key2 << "="    \
           << (val2) << ", " << key3 << "=" << (val3) << ", " << key4 << "=" \
           << (val4) << "]";                                                 \
    throw IgniteError(code, stream.str().c_str());                           \
  }

#endif  //_IGNITE_ERROR_MACRO

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4275)
#endif  //_MSC_VER

namespace trino {
namespace odbc {

/**
 * %Ignite error information.
 */
class IGNITE_IMPORT_EXPORT IgniteError : public std::exception {
 public:
  /** Success. */
  static const int IGNITE_SUCCESS = 0;

  /** Failed to connect to Trino */
  static const int IGNITE_ERR_TS_CONNECT = 102;

  /** Memory operation error. */
  static const int IGNITE_ERR_MEMORY = 1001;

  /** Binary error. */
  static const int IGNITE_ERR_BINARY = 1002;

  /** Standard library exception. */
  static const int IGNITE_ERR_STD = 1003;

  /** Generic %Ignite error. */
  static const int IGNITE_ERR_GENERIC = 2000;

  /** Illegal argument passed. */
  static const int IGNITE_ERR_ILLEGAL_ARGUMENT = 2001;

  /** Illegal state. */
  static const int IGNITE_ERR_ILLEGAL_STATE = 2002;

  /** Unsupported operation. */
  static const int IGNITE_ERR_UNSUPPORTED_OPERATION = 2003;

  /** Thread has been interrup. */
  static const int IGNITE_ERR_INTERRUPTED = 2004;

  /** Cluster group is empty. */
  static const int IGNITE_ERR_CLUSTER_GROUP_EMPTY = 2005;

  /** Cluster topology problem. */
  static const int IGNITE_ERR_CLUSTER_TOPOLOGY = 2006;

  /** Compute execution rejected. */
  static const int IGNITE_ERR_COMPUTE_EXECUTION_REJECTED = 2007;

  /** Compute job failover. */
  static const int IGNITE_ERR_COMPUTE_JOB_FAILOVER = 2008;

  /** Compute task cancelled. */
  static const int IGNITE_ERR_COMPUTE_TASK_CANCELLED = 2009;

  /** Compute task timeout. */
  static const int IGNITE_ERR_COMPUTE_TASK_TIMEOUT = 2010;

  /** Compute user undeclared exception. */
  static const int IGNITE_ERR_COMPUTE_USER_UNDECLARED_EXCEPTION = 2011;

  /** Generic cache error. */
  static const int IGNITE_ERR_CACHE = 2012;

  /** Generic cache loader error. */
  static const int IGNITE_ERR_CACHE_LOADER = 2013;

  /** Generic cache writer error. */
  static const int IGNITE_ERR_CACHE_WRITER = 2014;

  /** Generic cache entry processor error. */
  static const int IGNITE_ERR_ENTRY_PROCESSOR = 2015;

  /** Cache atomic update timeout. */
  static const int IGNITE_ERR_CACHE_ATOMIC_UPDATE_TIMEOUT = 2016;

  /** Cache partial update. */
  static const int IGNITE_ERR_CACHE_PARTIAL_UPDATE = 2017;

  /** Transaction optimisitc exception. */
  static const int IGNITE_ERR_TX_OPTIMISTIC = 2018;

  /** Transaction timeout. */
  static const int IGNITE_ERR_TX_TIMEOUT = 2019;

  /** Transaction rollback. */
  static const int IGNITE_ERR_TX_ROLLBACK = 2020;

  /** Transaction heuristic exception. */
  static const int IGNITE_ERR_TX_HEURISTIC = 2021;

  /** Authentication error. */
  static const int IGNITE_ERR_AUTHENTICATION = 2022;

  /** Security error. */
  static const int IGNITE_ERR_SECURITY = 2023;

  /** Future state error. */
  static const int IGNITE_ERR_FUTURE_STATE = 2024;

  /** Networking error. */
  static const int IGNITE_ERR_NETWORK_FAILURE = 2025;

  /** SSL/TLS error. */
  static const int IGNITE_ERR_SECURE_CONNECTION_FAILURE = 2026;

  /** Transaction already started by current thread. */
  static const int IGNITE_ERR_TX_THIS_THREAD = 2027;

  /** Generic transaction error. */
  static const int IGNITE_ERR_TX = 2028;

  /** Unknown error. */
  static const int IGNITE_ERR_UNKNOWN = -1;

  /**
   * Throw an error if code is not IGNITE_SUCCESS.
   *
   * @param err Error.
   */
  static void ThrowIfNeeded(const IgniteError& err);

  /**
   * Default constructor.
   * Creates empty error. Code is IGNITE_SUCCESS and message is NULL.
   */
  IgniteError();

  /**
   * Create error with specific code. Message is set to NULL.
   *
   * @param code Error code.
   */
  IgniteError(const int32_t code);

  /**
   * Create error with specific code and message.
   *
   * @param code Error code.
   * @param msg Message.
   */
  IgniteError(const int32_t code, const char* msg);

  /**
   * Copy constructor.
   *
   * @param other Other instance.
   */
  IgniteError(const IgniteError& other);

  /**
   * Assignment operator.
   *
   * @param other Other instance.
   * @return *this.
   */
  IgniteError& operator=(const IgniteError& other);

  /**
   * Destructor.
   */
  ~IgniteError() IGNITE_NO_THROW;

  /**
   * Get error code.
   *
   * @return Error code.
   */
  int32_t GetCode() const;

  /**
   * Get error message.
   *
   * @return Error message. Can be NULL.
   */
  const char* GetText() const IGNITE_NO_THROW;

  /**
   * Implementation of the standard std::exception::what() method.
   * Synonym for GetText() method.
   *
   * @return Error message string.
   */
  virtual const char* what() const IGNITE_NO_THROW;

 private:
  /** Error code. */
  int32_t code;

  /** Error message. */
  char* msg;
};
}  // namespace odbc
}  // namespace trino

#ifdef _MSC_VER
#pragma warning(pop)
#endif  //_MSC_VER

#endif  //_TRINO_ODBC_IGNITE_ERROR
