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

#ifndef _TRINO_ODBC_CONFIG_CONFIGURATION
#define _TRINO_ODBC_CONFIG_CONFIGURATION

#include <stdint.h>

#include <map>
#include <string>

#include "ignite/odbc/config/settable_value.h"
#include "ignite/odbc/diagnostic/diagnosable.h"
#include "trino/odbc/authentication/auth_type.h"
#include "ignite/odbc/odbc_error.h"
#include "trino/odbc/log_level.h"

#define DEFAULT_DSN "Trino DSN"
#define DEFAULT_DRIVER "Trino ODBC Driver"
#define DEFAULT_UID ""
#define DEFAULT_PWD ""

#define DEFAULT_PROFILE_NAME ""

#define DEFAULT_REQ_TIMEOUT 3000
#define DEFAULT_CONNECTION_TIMEOUT 1000
#define DEFAULT_MAX_RETRY_COUNT_CLIENT 0
#define DEFAULT_MAX_CONNECTIONS 25

#define DEFAULT_ENDPOINT ""
#define DEFAULT_REGION "us-east-1"

#define DEFAULT_AUTH_TYPE AuthType::Type::PASSWORD
#define DEFAULT_LOG_LEVEL LogLevel::Type::WARNING_LEVEL
#define DEFAULT_MAX_ROW_PER_PAGE -1

using ignite::odbc::config::SettableValue;

namespace trino {
namespace odbc {
namespace config {
/**
 * ODBC configuration abstraction.
 */
class IGNITE_IMPORT_EXPORT Configuration {
 public:
  /** Argument map type. */
  typedef std::map< std::string, std::string > ArgumentMap;

  /** Default values for configuration. */
  struct DefaultValue {
    /** Default value for DSN attribute. */
    static const std::string dsn;

    /** Default value for Driver attribute. */
    static const std::string driver;

    /** Default value for uid attribute. */
    static const std::string uid;

    /** Default value for pwd attribute. */
    static const std::string pwd;

    /** Default value for profileName attribute. */
    static const std::string profileName;

    /** Default value for reqTimeout attribute. */
    static const int32_t reqTimeout;

    /** Default value for connectionTimeout attribute. */
    static const int32_t connectionTimeout;

    /** Default value for maxRetryCountClient attribute. */
    static const int32_t maxRetryCountClient;

    /** Default value for maxConnections attribute. */
    static const int32_t maxConnections;

    /** Default value for endpoint attribute. */
    static const std::string endpoint;

    /** Default value for region attribute. */
    static const std::string region;

    /** Default value for authType attribute. */
    static const AuthType::Type authType;

    /** Default value for logLevel attribute. */
    static const LogLevel::Type logLevel;

    /** Default value for logPath attribute. */
    static const std::string logPath;

    /** Default value for maxRowPerPage attribute */
    static const int32_t maxRowPerPage;
  };

  /**
   * Default constructor.
   */
  Configuration() = default;

  /**
   * Destructor.
   */
  ~Configuration() = default;

  /**
   * Convert configure to connect string.
   *
   * @return Connect string.
   */
  std::string ToConnectString() const;

  /**
   * Get DSN.
   *
   * @return Data Source Name.
   */
  const std::string& GetDsn(const std::string& dflt = DefaultValue::dsn) const;

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsDsnSet() const;

  /**
   * Set DSN.
   *
   * @param dsn Data Source Name.
   */
  void SetDsn(const std::string& dsnName);

  /**
   * Get Driver.
   *
   * @return Driver name.
   */
  const std::string& GetDriver() const;

  /**
   * Set driver.
   *
   * @param driver Driver.
   */
  void SetDriver(const std::string& driverName);

  /**
   * Get username based on authType.
   *
   * @return username.
   */
  const std::string& GetDSNUserName() const;

  /**
   * Get password based on authType.
   *
   * @return password.
   */
  const std::string& GetDSNPassword() const;
  /**
   * Get uid.
   *
   * @return uid.
   */
  const std::string& GetUid() const;

  /**
   * Set uid.
   *
   * @param uidValue uid.
   */
  void SetUid(const std::string& uidValue);

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsUidSet() const;

  /**
   * Get pwd.
   *
   * @return pwd.
   */
  const std::string& GetPwd() const;

  /**
   * Set pwd.
   *
   * @param pwdValue pwd.
   */
  void SetPwd(const std::string& pwdValue);

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsPwdSet() const;

  /**
   * Get profileName.
   *
   * @return name Profile Name.
   */
  const std::string& GetProfileName() const;

  /**
   * Set profileName.
   *
   * @param name Profile Name.
   */
  void SetProfileName(const std::string& name);

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsProfileNameSet() const;

  /**
   * Get request timeout in milliseconds.
   *
   * @return ms Request timeout.
   */
  int32_t GetReqTimeout() const;

  /**
   * Set request timeout in milliseconds.
   *
   * @param ms Request timeout.
   */
  void SetReqTimeout(int32_t ms);

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsReqTimeoutSet() const;

  /**
   * Get connection timeout in milliseconds.
   *
   * @return ms Connection timeout.
   */
  int32_t GetConnectionTimeout() const;

  /**
   * Set connection timeout in milliseconds.
   *
   * @param ms Connection timeout.
   */
  void SetConnectionTimeout(int32_t ms);

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsConnectionTimeoutSet() const;

  /**
   * Get maximum # of retry attempts.
   *
   * @return count Maximum # of retry attempts.
   */
  int32_t GetMaxRetryCountClient() const;

  /**
   * Set maximum # of retry attempts.
   *
   * @param count Maximum # of retry attempts.
   */
  void SetMaxRetryCountClient(int32_t count);

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsMaxRetryCountClientSet() const;

  /**
   * Get maximum # of connections.
   *
   * @return count Maximum # of connections.
   */
  int32_t GetMaxConnections() const;

  /**
   * Set maximum # of connections.
   *
   * @param count Maximum # of connections.
   */
  void SetMaxConnections(int32_t count);

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsMaxConnectionsSet() const;

  /**
   * Get endpoint.
   *
   * @return value Endpoint.
   */
  const std::string& GetEndpoint() const;

  /**
   * Set endpoint.
   *
   * @param value Endpoint.
   */
  void SetEndpoint(const std::string& value);

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsEndpointSet() const;

  /**
   * Get region.
   *
   * @return value Region.
   */
  const std::string& GetRegion() const;

  /**
   * Set region.
   *
   * @param value region.
   */
  void SetRegion(const std::string& value);

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsRegionSet() const;

  /**
   * Get authType.
   *
   * @return value AuthType.
   */
  AuthType::Type GetAuthType() const;

  /**
   * Set authType.
   *
   * @param value AuthType.
   */
  void SetAuthType(const AuthType::Type value);

  /**
   * Check if the value set.
   *
   * @return @true if the value set.
   */
  bool IsAuthTypeSet() const;

  /**
   * Get log level.
   *
   * @return Log level.
   */
  LogLevel::Type GetLogLevel() const;

  /**
   * Set log level.
   *
   * @param level Log level.
   */
  void SetLogLevel(const LogLevel::Type level);

  /**
   * Check if log level set.
   *
   * @return @true if Log level set.
   */
  bool IsLogLevelSet() const;

  /**
   * Get log path to save.
   *
   * @return Log path.
   */
  const std::string& GetLogPath() const;

  /**
   * Set log path to save.
   *
   * @param path Log path.
   */
  void SetLogPath(const std::string& path);

  /**
   * Check if log path set.
   *
   * @return @true if Log path set.
   */
  bool IsLogPathSet() const;

  /**
   * Get maxRowPerPage.
   *
   * @return value MaxRowPerPage.
   */
  int32_t GetMaxRowPerPage() const;

  /**
   * Set maxRowPerPage to save.
   *
   * @param value MaxRowPerPage.
   */
  void SetMaxRowPerPage(int32_t value);

  /**
   * Check if the value set.
   *
   * @return @true if MaxRowPerPage set.
   */
  bool IsMaxRowPerPageSet() const;

  /**
   * Get argument map.
   *
   * @param res Resulting argument map.
   */
  void ToMap(ArgumentMap& res) const;

  /**
   * Checks that a valid JDBC connection string with all the required properties
   * can be built from the configuration. Throws error on incomplete properties.
   */
  void Validate() const;

 private:
  /**
   * Add key and value to the argument map.
   *
   * @param map Map.
   * @param key Key.
   * @param value Value.
   */
  template < typename T >
  static void AddToMap(ArgumentMap& map, const std::string& key,
                       const SettableValue< T >& value);

  /**
   * Add key and value to the argument map.
   *
   * @param map Map.
   * @param key Key.
   * @param value Value.
   */
  template < typename T >
  static void AddToMap(ArgumentMap& map, const std::string& key,
                       const SettableValue< T >& value, bool isJdbcFormat);

  /** DSN. */
  SettableValue< std::string > dsn = DefaultValue::dsn;

  /** Driver Name. */
  SettableValue< std::string > driver = DefaultValue::driver;

  /** UID. */
  SettableValue< std::string > uid = DefaultValue::uid;

  /** PWD. */
  SettableValue< std::string > pwd = DefaultValue::pwd;

  /** Profile Name. */
  SettableValue< std::string > profileName = DefaultValue::profileName;

  /** Request timeout in milliseconds.  */
  SettableValue< int32_t > reqTimeout = DefaultValue::reqTimeout;

  /** Connection timeout in milliseconds.  */
  SettableValue< int32_t > connectionTimeout = DefaultValue::connectionTimeout;

  /** Max Retry Count.  */
  SettableValue< int32_t > maxRetryCountClient =
      DefaultValue::maxRetryCountClient;

  /** Max Connections.  */
  SettableValue< int32_t > maxConnections = DefaultValue::maxConnections;

  /** Endpoint. */
  SettableValue< std::string > endpoint = DefaultValue::endpoint;

  /** Region. */
  SettableValue< std::string > region = DefaultValue::region;

  /** Auth Type. */
  SettableValue< AuthType::Type > authType = DefaultValue::authType;

  /** The log level for the log file. */
  SettableValue< LogLevel::Type > logLevel = DefaultValue::logLevel;

  /** The logging file path. */
  SettableValue< std::string > logPath = DefaultValue::logPath;

  /** The max row number in one page returned from Trino */
  SettableValue< int32_t > maxRowPerPage = DefaultValue::maxRowPerPage;
};

template <>
void Configuration::AddToMap< std::string >(
    ArgumentMap& map, const std::string& key,
    const SettableValue< std::string >& value);

template <>
void Configuration::AddToMap< uint16_t >(
    ArgumentMap& map, const std::string& key,
    const SettableValue< uint16_t >& value);

template <>
void Configuration::AddToMap< int32_t >(ArgumentMap& map,
                                        const std::string& key,
                                        const SettableValue< int32_t >& value);

template <>
void Configuration::AddToMap< bool >(ArgumentMap& map, const std::string& key,
                                     const SettableValue< bool >& value);

template <>
void Configuration::AddToMap< AuthType::Type >(
    ArgumentMap& map, const std::string& key,
    const SettableValue< AuthType::Type >& value);

template <>
void Configuration::AddToMap< LogLevel::Type >(
    ArgumentMap& map, const std::string& key,
    const SettableValue< LogLevel::Type >& value);
}  // namespace config
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_CONFIG_CONFIGURATION
