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
const std::string Configuration::DefaultValue::profileName =
    DEFAULT_PROFILE_NAME;

// Connection Options
const int32_t Configuration::DefaultValue::reqTimeout = DEFAULT_REQ_TIMEOUT;
const int32_t Configuration::DefaultValue::connectionTimeout =
    DEFAULT_CONNECTION_TIMEOUT;
const int32_t Configuration::DefaultValue::maxRetryCountClient =
    DEFAULT_MAX_RETRY_COUNT_CLIENT;
const int32_t Configuration::DefaultValue::maxConnections =
    DEFAULT_MAX_CONNECTIONS;

// Endpoint Options
const std::string Configuration::DefaultValue::endpoint = DEFAULT_ENDPOINT;
const std::string Configuration::DefaultValue::region = DEFAULT_REGION;

// Advance Authentication Settings
const AuthType::Type Configuration::DefaultValue::authType = DEFAULT_AUTH_TYPE;
const std::string Configuration::DefaultValue::idPHost = DEFAULT_IDP_HOST;
const std::string Configuration::DefaultValue::idPUserName =
    DEFAULT_IDP_USER_NAME;
const std::string Configuration::DefaultValue::idPPassword =
    DEFAULT_IDP_PASSWORD;
const std::string Configuration::DefaultValue::idPArn = DEFAULT_IDP_ARN;
const std::string Configuration::DefaultValue::oktaAppId = DEFAULT_OKTA_APP_ID;
const std::string Configuration::DefaultValue::roleArn = DEFAULT_ROLE_ARN;
const std::string Configuration::DefaultValue::aadAppId = DEFAULT_AAD_APP_ID;
const std::string Configuration::DefaultValue::aadClientSecret =
    DEFAULT_ACCESS_CLIENT_SECRET;
const std::string Configuration::DefaultValue::aadTenant = DEFAULT_AAD_TENANT;

// Logging Configuration Options
const LogLevel::Type Configuration::DefaultValue::logLevel = DEFAULT_LOG_LEVEL;
const std::string Configuration::DefaultValue::logPath = DEFAULT_LOG_PATH;
const int32_t Configuration::DefaultValue::maxRowPerPage =
    DEFAULT_MAX_ROW_PER_PAGE;

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

/*$*/
const std::string& Configuration::GetDSNUserName() const {
  LOG_DEBUG_MSG("GetDSNUserName is called");
  if (!GetUid().empty())
    return GetUid();

  LOG_DEBUG_MSG("AuthType: " << AuthType::ToCBString(GetAuthType()));
  switch (GetAuthType()) {
    case AuthType::Type::AAD:
      return GetIdPUserName();

    default:
      return GetUid();
  }
}

const std::string& Configuration::GetDSNPassword() const {
  LOG_DEBUG_MSG("GetDSNPassword is called");
  if (!GetPwd().empty())
    return GetPwd();

  LOG_DEBUG_MSG("AuthType: " << AuthType::ToCBString(GetAuthType()));
  switch (GetAuthType()) {
    case AuthType::Type::AAD:
      return GetIdPPassword();

    default:
      return GetPwd();
  }
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

int32_t Configuration::GetMaxConnections() const {
  return maxConnections.GetValue();
}

void Configuration::SetMaxConnections(int32_t count) {
  this->maxConnections.SetValue(count);
}

bool Configuration::IsMaxConnectionsSet() const {
  return maxConnections.IsSet();
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

const std::string& Configuration::GetRegion() const {
  return region.GetValue();
}

void Configuration::SetRegion(const std::string& value) {
  this->region.SetValue(value);
}

bool Configuration::IsRegionSet() const {
  return region.IsSet();
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

const std::string& Configuration::GetIdPHost() const {
  return idPHost.GetValue();
}

void Configuration::SetIdPHost(const std::string& value) {
  this->idPHost.SetValue(value);
}

bool Configuration::IsIdPHostSet() const {
  return idPHost.IsSet();
}

const std::string& Configuration::GetIdPUserName() const {
  return idPUserName.GetValue();
}

void Configuration::SetIdPUserName(const std::string& value) {
  this->idPUserName.SetValue(value);
}

bool Configuration::IsIdPUserNameSet() const {
  return idPUserName.IsSet();
}

const std::string& Configuration::GetIdPPassword() const {
  return idPPassword.GetValue();
}

void Configuration::SetIdPPassword(const std::string& value) {
  this->idPPassword.SetValue(value);
}

bool Configuration::IsIdPPasswordSet() const {
  return idPPassword.IsSet();
}

const std::string& Configuration::GetIdPArn() const {
  return idPArn.GetValue();
}

void Configuration::SetIdPArn(const std::string& value) {
  this->idPArn.SetValue(value);
}

bool Configuration::IsIdPArnSet() const {
  return idPArn.IsSet();
}

const std::string& Configuration::GetOktaAppId() const {
  return oktaAppId.GetValue();
}

void Configuration::SetOktaAppId(const std::string& value) {
  this->oktaAppId.SetValue(value);
}

bool Configuration::IsOktaAppIdSet() const {
  return oktaAppId.IsSet();
}

const std::string& Configuration::GetRoleArn() const {
  return roleArn.GetValue();
}

void Configuration::SetRoleArn(const std::string& value) {
  this->roleArn.SetValue(value);
}

bool Configuration::IsRoleArnSet() const {
  return roleArn.IsSet();
}

const std::string& Configuration::GetAADAppId() const {
  return aadAppId.GetValue();
}

void Configuration::SetAADAppId(const std::string& value) {
  this->aadAppId.SetValue(value);
}

bool Configuration::IsAADAppIdSet() const {
  return aadAppId.IsSet();
}

const std::string& Configuration::GetAADClientSecret() const {
  return aadClientSecret.GetValue();
}

void Configuration::SetAADClientSecret(const std::string& value) {
  this->aadClientSecret.SetValue(value);
}

bool Configuration::IsAADClientSecretSet() const {
  return aadClientSecret.IsSet();
}

const std::string& Configuration::GetAADTenant() const {
  return aadTenant.GetValue();
}

void Configuration::SetAADTenant(const std::string& value) {
  this->aadTenant.SetValue(value);
}

bool Configuration::IsAADTenantSet() const {
  return aadTenant.IsSet();
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
  AddToMap(res, ConnectionStringParser::Key::connectionTimeout,
           connectionTimeout);
  AddToMap(res, ConnectionStringParser::Key::maxRetryCountClient,
           maxRetryCountClient);
  AddToMap(res, ConnectionStringParser::Key::maxConnections, maxConnections);
  AddToMap(res, ConnectionStringParser::Key::endpoint, endpoint);
  AddToMap(res, ConnectionStringParser::Key::region, region);
  AddToMap(res, ConnectionStringParser::Key::authType, authType);
  AddToMap(res, ConnectionStringParser::Key::idPHost, idPHost);
  AddToMap(res, ConnectionStringParser::Key::idPUserName, idPUserName);
  AddToMap(res, ConnectionStringParser::Key::idPPassword, idPPassword);
  AddToMap(res, ConnectionStringParser::Key::idPArn, idPArn);
  AddToMap(res, ConnectionStringParser::Key::oktaAppId, oktaAppId);
  AddToMap(res, ConnectionStringParser::Key::roleArn, roleArn);
  AddToMap(res, ConnectionStringParser::Key::aadAppId, aadAppId);
  AddToMap(res, ConnectionStringParser::Key::aadClientSecret, aadClientSecret);
  AddToMap(res, ConnectionStringParser::Key::aadTenant, aadTenant);
  AddToMap(res, ConnectionStringParser::Key::logLevel, logLevel);
  AddToMap(res, ConnectionStringParser::Key::logPath, logPath);
  AddToMap(res, ConnectionStringParser::Key::maxRowPerPage, maxRowPerPage);
}

void Configuration::Validate() const {
  LOG_DEBUG_MSG("Validate is called");
  // Validate minimum required properties.
/*$*/
  if ((GetAuthType() == trino::odbc::AuthType::Type::OKTA)
      && (GetIdPHost().empty() || GetDSNUserName().empty()
          || GetDSNPassword().empty() || GetIdPArn().empty()
          || GetRoleArn().empty() || GetOktaAppId().empty())) {
    throw ignite::odbc::OdbcError(
        SqlState::S01S00_INVALID_CONNECTION_STRING_ATTRIBUTE,
        "The following is required to connect:\n"
        "AUTH is \"OKTA\" and "
        "IdpHost, UID or IdpUserName, PWD or IdpPassword, OktaAppId, RoleArn "
        "and IdpArn");
  }

/*$*/
  if ((GetAuthType() == trino::odbc::AuthType::Type::AAD)
      && (GetDSNUserName().empty() || GetDSNPassword().empty()
          || GetIdPArn().empty() || GetRoleArn().empty()
          || GetAADAppId().empty() || GetAADTenant().empty()
          || GetAADClientSecret().empty())) {
    throw ignite::odbc::OdbcError(
        SqlState::S01S00_INVALID_CONNECTION_STRING_ATTRIBUTE,
        "The following is required to connect:\n"
        "AUTH is \"AAD\" and "
        "UID or IdpUserName, PWD or IdpPassword, and "
        "AADAppId, RoleArn, IdpArn, AADTenant and AADClientSecret");
  }

/*$*/
  if ((GetAuthType() == trino::odbc::AuthType::Type::IAM)
      && (GetDSNUserName().empty() || GetDSNPassword().empty())) {
    throw ignite::odbc::OdbcError(
        SqlState::S01S00_INVALID_CONNECTION_STRING_ATTRIBUTE,
        "The following is required to connect:\n"
        "AUTH is ");
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
