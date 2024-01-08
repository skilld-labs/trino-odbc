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

#include "trino/odbc/config/configuration.h"

#include <iterator>
#include <sstream>
#include <string>

#include "trino/odbc/utils.h"
#include "trino/odbc/config/connection_string_parser.h"
#include "trino/odbc/authentication/auth_type.h"
#include "trino/odbc/log.h"
#include "trino/odbc/utility.h"

using trino::odbc::common::EncodeURIComponent;

namespace trino {
namespace odbc {
namespace config {
// Connection (Basic Authentication) Settings
const std::string Configuration::DefaultValue::dsn = DEFAULT_DSN;
const std::string Configuration::DefaultValue::driver = DEFAULT_DRIVER;
const std::string Configuration::DefaultValue::uid = DEFAULT_UID;
const std::string Configuration::DefaultValue::pwd = DEFAULT_PWD;

// Credential Providers Options
const std::string Configuration::DefaultValue::profileName = DEFAULT_PROFILE_NAME;

// Connection Options
const int32_t Configuration::DefaultValue::reqTimeout = DEFAULT_REQ_TIMEOUT;
const int32_t Configuration::DefaultValue::connectionTimeout = DEFAULT_CONNECTION_TIMEOUT;
const int32_t Configuration::DefaultValue::maxRetryCountClient = DEFAULT_MAX_RETRY_COUNT_CLIENT;

// Endpoint Options
const std::string Configuration::DefaultValue::endpoint = DEFAULT_ENDPOINT;

// Advance Authentication Settings
const AuthType::Type Configuration::DefaultValue::authType = DEFAULT_AUTH_TYPE;

// Logging Configuration Options
const LogLevel::Type Configuration::DefaultValue::logLevel = DEFAULT_LOG_LEVEL;
const std::string Configuration::DefaultValue::logPath = DEFAULT_LOG_PATH;
const int32_t Configuration::DefaultValue::maxRowPerPage = DEFAULT_MAX_ROW_PER_PAGE;

std::string Configuration::ToConnectString() const {
  LOG_DEBUG_MSG("ToConnectString is called");
  ArgumentMap arguments;

  ToMap(arguments);

  std::stringstream connect_string_buffer;

  for (ArgumentMap::const_iterator it = arguments.begin();
       it != arguments.end(); ++it) {
    const std::string& key = it->first;
    const std::string& value = it->second;

    if (value.empty())
      continue;

    // If there is space in the value, add brackets around it.
    if (value.find(' ') == std::string::npos)
      connect_string_buffer << key << '=' << value << ';';
    else
      connect_string_buffer << key << "={" << value << "};";
  }

  LOG_DEBUG_MSG("connect_string_buffer is " << connect_string_buffer.str());
  return connect_string_buffer.str();
}

const std::string& Configuration::GetDsn(const std::string& dflt) const {
  if (!dsn.IsSet())
    return dflt;

  return dsn.GetValue();
}

bool Configuration::IsDsnSet() const {
  return dsn.IsSet();
}

void Configuration::SetDsn(const std::string& dsnName) {
  this->dsn.SetValue(dsnName);
}

const std::string& Configuration::GetDriver() const {
  return driver.GetValue();
}

void Configuration::SetDriver(const std::string& driverName) {
  this->driver.SetValue(driverName);
}

const std::string& Configuration::GetDSNUserName() const {
  LOG_DEBUG_MSG("GetDSNUserName is called");
  // if (!GetUid().empty())
  //   return GetUid();

  LOG_DEBUG_MSG("AuthType: " << AuthType::ToCBString(GetAuthType()));
  // switch (GetAuthType()) {
  //   default:
  return GetUid();
  // }
}

const std::string& Configuration::GetDSNPassword() const {
  LOG_DEBUG_MSG("GetDSNPassword is called");
  // if (!GetPwd().empty())
  //   return GetPwd();

  LOG_DEBUG_MSG("AuthType: " << AuthType::ToCBString(GetAuthType()));
  // switch (GetAuthType()) {
  //   default:
  return GetPwd();
  // }
}

const std::string& Configuration::GetUid() const {
  return uid.GetValue();
}

void Configuration::SetUid(const std::string& uidValue) {
  this->uid.SetValue(uidValue);
}

bool Configuration::IsUidSet() const {
  return uid.IsSet();
}

const std::string& Configuration::GetPwd() const {
  return pwd.GetValue();
}

void Configuration::SetPwd(const std::string& pwdValue) {
  this->pwd.SetValue(pwdValue);
}

bool Configuration::IsPwdSet() const {
  return pwd.IsSet();
}

const std::string& Configuration::GetProfileName() const {
  return profileName.GetValue();
}

void Configuration::SetProfileName(const std::string& name) {
  this->profileName.SetValue(name);
}

bool Configuration::IsProfileNameSet() const {
  return profileName.IsSet() && !profileName.GetValue().empty();
}

int32_t Configuration::GetReqTimeout() const {
  return reqTimeout.GetValue();
}

void Configuration::SetReqTimeout(int32_t ms) {
  this->reqTimeout.SetValue(ms);
}

bool Configuration::IsReqTimeoutSet() const {
  return reqTimeout.IsSet();
}

int32_t Configuration::GetConnectionTimeout() const {
  return connectionTimeout.GetValue();
}

void Configuration::SetConnectionTimeout(int32_t ms) {
  this->connectionTimeout.SetValue(ms);
}

bool Configuration::IsConnectionTimeoutSet() const {
  return connectionTimeout.IsSet();
}

int32_t Configuration::GetMaxRetryCountClient() const {
  return maxRetryCountClient.GetValue();
}

void Configuration::SetMaxRetryCountClient(int32_t count) {
  this->maxRetryCountClient.SetValue(count);
}

bool Configuration::IsMaxRetryCountClientSet() const {
  return maxRetryCountClient.IsSet();
}

const std::string& Configuration::GetEndpoint() const {
  return endpoint.GetValue();
}

void Configuration::SetEndpoint(const std::string& value) {
  this->endpoint.SetValue(value);
}

bool Configuration::IsEndpointSet() const {
  return endpoint.IsSet();
}

AuthType::Type Configuration::GetAuthType() const {
  return authType.GetValue();
}

void Configuration::SetAuthType(const AuthType::Type value) {
  this->authType.SetValue(value);
}

bool Configuration::IsAuthTypeSet() const {
  return authType.IsSet();
}

LogLevel::Type Configuration::GetLogLevel() const {
  return logLevel.GetValue();
}

void Configuration::SetLogLevel(const LogLevel::Type level) {
  if (level != LogLevel::Type::UNKNOWN) {
    this->logLevel.SetValue(level);
    Logger::GetLoggerInstance()->SetLogLevel(level);
  }
}

bool Configuration::IsLogLevelSet() const {
  return logLevel.IsSet();
}

const std::string& Configuration::GetLogPath() const {
  return logPath.GetValue();
}

void Configuration::SetLogPath(const std::string& path) {
  if (ignite::odbc::common::IsValidDirectory(path)) {
    this->logPath.SetValue(path);
    Logger::GetLoggerInstance()->SetLogPath(path);
  }
}

bool Configuration::IsLogPathSet() const {
  return logPath.IsSet();
}

int32_t Configuration::GetMaxRowPerPage() const {
  return maxRowPerPage.GetValue();
}

void Configuration::SetMaxRowPerPage(int32_t value) {
  this->maxRowPerPage.SetValue(value);
}

bool Configuration::IsMaxRowPerPageSet() const {
  return maxRowPerPage.IsSet();
}

void Configuration::ToMap(ArgumentMap& res) const {
  AddToMap(res, ConnectionStringParser::Key::dsn, dsn);
  AddToMap(res, ConnectionStringParser::Key::driver, driver);
  AddToMap(res, ConnectionStringParser::Key::uid, uid);
  AddToMap(res, ConnectionStringParser::Key::pwd, pwd);
  AddToMap(res, ConnectionStringParser::Key::profileName, profileName);
  AddToMap(res, ConnectionStringParser::Key::reqTimeout, reqTimeout);
  AddToMap(res, ConnectionStringParser::Key::connectionTimeout, connectionTimeout);
  AddToMap(res, ConnectionStringParser::Key::maxRetryCountClient, maxRetryCountClient);
  AddToMap(res, ConnectionStringParser::Key::endpoint, endpoint);
  AddToMap(res, ConnectionStringParser::Key::authType, authType);
  AddToMap(res, ConnectionStringParser::Key::logLevel, logLevel);
  AddToMap(res, ConnectionStringParser::Key::logPath, logPath);
  AddToMap(res, ConnectionStringParser::Key::maxRowPerPage, maxRowPerPage);
}

void Configuration::Validate() const {
  LOG_DEBUG_MSG("Validate is called");
  if ((GetAuthType() == trino::odbc::AuthType::Type::PASSWORD)
      && (GetDSNUserName().empty() || GetDSNPassword().empty())) {
    throw ignite::odbc::OdbcError(
        SqlState::S01S00_INVALID_CONNECTION_STRING_ATTRIBUTE,
        "The following is required to connect:\n"
        "AUTH is \"PASSWORD\" and "
        "UID and PWD");
  }
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< uint16_t >& value) {
  if (value.IsSet())
    map[key] =
        trino::odbc::common::LexicalCast< std::string >(value.GetValue());
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< int32_t >& value) {
  if (value.IsSet())
    map[key] =
        trino::odbc::common::LexicalCast< std::string >(value.GetValue());
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< std::string >& value) {
  if (value.IsSet())
    map[key] = value.GetValue();
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< bool >& value) {
  if (value.IsSet())
    map[key] = value.GetValue() ? "true" : "false";
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< AuthType::Type >& value) {
  if (value.IsSet())
    map[key] = AuthType::ToString(value.GetValue());
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< LogLevel::Type >& value) {
  if (value.IsSet())
    map[key] = LogLevel::ToString(value.GetValue());
}
}  // namespace config
}  // namespace odbc
}  // namespace trino
