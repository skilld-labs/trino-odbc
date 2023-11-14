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

#ifndef _TRINO_ODBC_CONFIG_CONNECTION_STRING_PARSER
#define _TRINO_ODBC_CONFIG_CONNECTION_STRING_PARSER

#include <string>

#include "trino/odbc/config/configuration.h"
#include "trino/odbc/diagnostic/diagnostic_record_storage.h"

namespace trino {
namespace odbc {
namespace config {
/**
 * ODBC configuration parser abstraction.
 */
class IGNITE_IMPORT_EXPORT ConnectionStringParser {
 public:
  /** Connection attribute keywords. */
  struct Key {
    /** Connection attribute keyword for DSN attribute. */
    static const std::string dsn;

    /** Connection attribute keyword for Driver attribute. */
    static const std::string driver;

    /** Connection attribute keyword for uid attribute. */
    static const std::string uid;

    /** Connection attribute keyword for pwd attribute. */
    static const std::string pwd;

    /** Connection attribute keyword for profileName attribute. */
    static const std::string profileName;

    /** Connection attribute keyword for reqTimeout attribute. */
    static const std::string reqTimeout;

    /** Connection attribute keyword for connectionTimeout attribute. */
    static const std::string connectionTimeout;

    /** Connection attribute keyword for maxRetryCountClient attribute. */
    static const std::string maxRetryCountClient;

    /** Connection attribute keyword for maxConnections attribute. */
    static const std::string maxConnections;

    /** Connection attribute keyword for endpoint attribute. */
    static const std::string endpoint;

    /** Connection attribute keyword for region attribute. */
    static const std::string region;

    /** Connection attribute keyword for authType attribute. */
    static const std::string authType;

    /** Connection attribute keyword for idPHost attribute. */
    static const std::string idPHost;

    /** Connection attribute keyword for idPUserName attribute. */
    static const std::string idPUserName;

    /** Connection attribute keyword for idPPassword attribute. */
    static const std::string idPPassword;

    /** Connection attribute keyword for idPArn attribute. */
    static const std::string idPArn;

/*$*/
    /** Connection attribute keyword for oktaAppId attribute. */
    static const std::string oktaAppId;

    /** Connection attribute keyword for roleArn attribute. */
    static const std::string roleArn;

    /** Connection attribute keyword for aadAppId attribute. */
    static const std::string aadAppId;

    /** Connection attribute keyword for aadClientSecret attribute. */
    static const std::string aadClientSecret;

    /** Connection attribute keyword for aadTenant attribute. */
    static const std::string aadTenant;

    /** Connection attribute keyword for log level. */
    static const std::string logLevel;

    /** Connection attribute keyword for log path. */
    static const std::string logPath;

    /** Max number of rows in one page returned from TS. */
    static const std::string maxRowPerPage;
  };

  /**
   * Constructor.
   *
   * @param cfg Configuration.
   */
  ConnectionStringParser(Configuration& cfg);

  /**
   * Destructor.
   */
  ~ConnectionStringParser() = default;

  /**
   * Parse connect string.
   *
   * @param str String to parse.
   * @param len String length.
   * @param delimiter delimiter.
   * @param diag Diagnostics collector.
   */
  void ParseConnectionString(const char* str, size_t len, char delimiter,
                             diagnostic::DiagnosticRecordStorage* diag);

  /**
   * Parse connect string.
   *
   * @param str String to parse.
   * @param diag Diagnostics collector.
   */
  void ParseConnectionString(const std::string& str,
                             diagnostic::DiagnosticRecordStorage* diag);

  /**
   * Parse config attributes.
   *
   * @param str String to parse.
   * @param diag Diagnostics collector.
   */
  void ParseConfigAttributes(const char* str,
                             diagnostic::DiagnosticRecordStorage* diag);

 private:
  /**
   * Result of parsing string value to bool.
   */
  struct BoolParseResult {
    enum class Type { AI_FALSE, AI_TRUE, AI_UNRECOGNIZED };
  };

  /**
   * Handle new attribute pair callback.
   *
   * @param key Key.
   * @param value Value.
   * @param diag Diagnostics collector.
   */
  void HandleAttributePair(const std::string& key, const std::string& value,
                           diagnostic::DiagnosticRecordStorage* diag);

  /**
   * Convert string to boolean value.
   *
   * @param value Value to convert to bool.
   * @return Result.
   */
  static BoolParseResult::Type StringToBool(const std::string& value);

  /**
   * Convert string to boolean value.
   *
   * @param msg Error message.
   * @param key Key.
   * @param value Value.
   * @return Resulting error message.
   */
  static std::string MakeErrorMessage(const std::string& msg,
                                      const std::string& key,
                                      const std::string& value);

  /** Configuration. */
  Configuration& cfg;
};
}  // namespace config

}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_CONFIG_CONNECTION_STRING_PARSER
