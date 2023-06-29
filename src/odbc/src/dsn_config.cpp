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

#include "timestream/odbc/dsn_config.h"
#include "timestream/odbc/config/connection_string_parser.h"
#include <timestream/odbc/authentication/auth_type.h>
#include <timestream/odbc/log_level.h>
#include <timestream/odbc/log.h>
#include "timestream/odbc/system/odbc_constants.h"
#include "timestream/odbc/utility.h"

using namespace timestream::odbc::config;
using namespace timestream::odbc::utility;

#define BUFFER_SIZE (1024 * 1024)
#define CONFIG_FILE u8"ODBC.INI"

namespace timestream {
namespace odbc {
void GetLastSetupError(IgniteError& error) {
  DWORD code;
  ignite::odbc::common::FixedSizeArray< SQLWCHAR > msg(BUFFER_SIZE);

  SQLInstallerError(1, &code, msg.GetData(), msg.GetSize(), NULL);

  std::stringstream buf;

  buf << "SQLInstallerError: Message: \""
      << utility::SqlWcharToString(msg.GetData(), msg.GetSize())
      << "\", SQLInstallerError Code: " << code;

  LOG_ERROR_MSG(buf.str());
  error = IgniteError(IgniteError::IGNITE_ERR_GENERIC, buf.str().c_str());
}

void ThrowLastSetupError() {
  IgniteError error;
  GetLastSetupError(error);
  throw error;
}

bool WriteDsnString(const char* dsn, const char* key, const char* value,
                    IgniteError& error) {
  LOG_DEBUG_MSG("WriteDsnString is called");
  if (!SQLWritePrivateProfileString(
          utility::ToWCHARVector(dsn).data(),
          utility::ToWCHARVector(key).data(),
          utility::ToWCHARVector(value).data(),
          utility::ToWCHARVector(CONFIG_FILE).data())) {
    GetLastSetupError(error);
    return false;
  }
  return true;
}

SettableValue< std::string > ReadDsnString(const char* dsn,
                                           const std::string& key,
                                           const std::string& dflt = "") {
  LOG_DEBUG_MSG("ReadDsnString is called with dsn is " << dsn << ", key is "
                                                       << key);
  static const char* unique = "35a920dd-8837-43d2-a846-e01a2e7b5f84";

  SettableValue< std::string > val(dflt);

  ignite::odbc::common::FixedSizeArray< SQLWCHAR > buf(BUFFER_SIZE);

  int ret = SQLGetPrivateProfileString(
      utility::ToWCHARVector(dsn).data(), utility::ToWCHARVector(key).data(),
      utility::ToWCHARVector(unique).data(), buf.GetData(), buf.GetSize(),
      utility::ToWCHARVector(CONFIG_FILE).data());

  if (ret > BUFFER_SIZE) {
    buf.Reset(ret + 1);

    ret = SQLGetPrivateProfileString(
        utility::ToWCHARVector(dsn).data(), utility::ToWCHARVector(key).data(),
        utility::ToWCHARVector(unique).data(), buf.GetData(), buf.GetSize(),
        utility::ToWCHARVector(CONFIG_FILE).data());
  }

  std::string res = utility::SqlWcharToString(buf.GetData());

  if (res != unique)
    val.SetValue(res);

  LOG_DEBUG_MSG("val is " << val.GetValue());
  return val;
}

SettableValue< int32_t > ReadDsnInt(const char* dsn, const std::string& key,
                                    int32_t dflt = 0) {
  LOG_DEBUG_MSG("ReadDsnInt is called with dsn is " << dsn << ", key is "
                                                    << key);
  SettableValue< std::string > str = ReadDsnString(dsn, key, "");

  SettableValue< int32_t > res(dflt);

  if (str.IsSet())
    res.SetValue(timestream::odbc::common::LexicalCast< int, std::string >(
        str.GetValue()));

  LOG_DEBUG_MSG("res is " << res.GetValue());
  return res;
}

SettableValue< bool > ReadDsnBool(const char* dsn, const std::string& key,
                                  bool dflt = false) {
  LOG_DEBUG_MSG("ReadDsnBool is called with dsn is " << dsn << ", key is "
                                                     << key);
  SettableValue< std::string > str = ReadDsnString(dsn, key, "");

  SettableValue< bool > res(dflt);

  if (str.IsSet())
    res.SetValue(str.GetValue() == "true");

  LOG_DEBUG_MSG("res is " << res.GetValue());
  return res;
}

void ReadDsnConfiguration(const char* dsn, Configuration& config,
                          diagnostic::DiagnosticRecordStorage* diag) {
  LOG_DEBUG_MSG("ReadDsnConfiguration is called with dsn is " << dsn);
  SettableValue< std::string > uid =
      ReadDsnString(dsn, ConnectionStringParser::Key::uid);

  if (uid.IsSet() && !config.IsUidSet())
    config.SetUid(uid.GetValue());

  SettableValue< std::string > pwd =
      ReadDsnString(dsn, ConnectionStringParser::Key::pwd);

  if (pwd.IsSet() && !config.IsPwdSet())
    config.SetPwd(pwd.GetValue());

  SettableValue< std::string > accessKeyId =
      ReadDsnString(dsn, ConnectionStringParser::Key::accessKeyId);

  if (accessKeyId.IsSet() && !config.IsAccessKeyIdSet())
    config.SetAccessKeyId(accessKeyId.GetValue());

  SettableValue< std::string > secretKey =
      ReadDsnString(dsn, ConnectionStringParser::Key::secretKey);

  if (secretKey.IsSet() && !config.IsSecretKeySet())
    config.SetSecretKey(secretKey.GetValue());

  SettableValue< std::string > sessionToken =
      ReadDsnString(dsn, ConnectionStringParser::Key::sessionToken);

  if (sessionToken.IsSet() && !config.IsSessionTokenSet())
    config.SetSessionToken(sessionToken.GetValue());

  SettableValue< std::string > profileName =
      ReadDsnString(dsn, ConnectionStringParser::Key::profileName);

  if (profileName.IsSet() && !config.IsProfileNameSet())
    config.SetProfileName(profileName.GetValue());

  SettableValue< int32_t > reqTimeout =
      ReadDsnInt(dsn, ConnectionStringParser::Key::reqTimeout);

  if (reqTimeout.IsSet() && !config.IsReqTimeoutSet())
    config.SetReqTimeout(reqTimeout.GetValue());

  SettableValue< int32_t > connectionTimeout =
      ReadDsnInt(dsn, ConnectionStringParser::Key::connectionTimeout);

  if (reqTimeout.IsSet() && !config.IsConnectionTimeoutSet())
    config.SetConnectionTimeout(connectionTimeout.GetValue());

  SettableValue< int32_t > maxRetryCountClient =
      ReadDsnInt(dsn, ConnectionStringParser::Key::maxRetryCountClient);

  if (maxRetryCountClient.IsSet() && !config.IsMaxRetryCountClientSet())
    config.SetMaxRetryCountClient(maxRetryCountClient.GetValue());

  SettableValue< int32_t > maxConnections =
      ReadDsnInt(dsn, ConnectionStringParser::Key::maxConnections);

  if (maxConnections.IsSet() && !config.IsMaxConnectionsSet())
    config.SetMaxConnections(maxConnections.GetValue());

  SettableValue< std::string > endpoint =
      ReadDsnString(dsn, ConnectionStringParser::Key::endpoint);

  if (endpoint.IsSet() && !config.IsEndpointSet())
    config.SetEndpoint(endpoint.GetValue());

  SettableValue< std::string > region =
      ReadDsnString(dsn, ConnectionStringParser::Key::region);

  if (region.IsSet() && !config.IsRegionSet())
    config.SetRegion(region.GetValue());

  SettableValue< std::string > authType =
      ReadDsnString(dsn, ConnectionStringParser::Key::authType);

  if (authType.IsSet() && !config.IsAuthTypeSet()) {
    AuthType::Type type =
        AuthType::FromString(authType.GetValue(), AuthType::Type::AWS_PROFILE);
    config.SetAuthType(type);
  }

  SettableValue< std::string > idPHost =
      ReadDsnString(dsn, ConnectionStringParser::Key::idPHost);

  if (idPHost.IsSet() && !config.IsIdPHostSet())
    config.SetIdPHost(idPHost.GetValue());

  SettableValue< std::string > idPUserName =
      ReadDsnString(dsn, ConnectionStringParser::Key::idPUserName);

  if (idPUserName.IsSet() && !config.IsIdPUserNameSet())
    config.SetIdPUserName(idPUserName.GetValue());

  SettableValue< std::string > idPPassword =
      ReadDsnString(dsn, ConnectionStringParser::Key::idPPassword);

  if (idPPassword.IsSet() && !config.IsIdPPasswordSet())
    config.SetIdPPassword(idPPassword.GetValue());

  SettableValue< std::string > idPArn =
      ReadDsnString(dsn, ConnectionStringParser::Key::idPArn);

  if (idPArn.IsSet() && !config.IsIdPArnSet())
    config.SetIdPArn(idPArn.GetValue());

  SettableValue< std::string > oktaAppId =
      ReadDsnString(dsn, ConnectionStringParser::Key::oktaAppId);

  if (oktaAppId.IsSet() && !config.IsOktaAppIdSet())
    config.SetOktaAppId(oktaAppId.GetValue());

  SettableValue< std::string > roleArn =
      ReadDsnString(dsn, ConnectionStringParser::Key::roleArn);

  if (roleArn.IsSet() && !config.IsRoleArnSet())
    config.SetRoleArn(roleArn.GetValue());

  SettableValue< std::string > aadAppId =
      ReadDsnString(dsn, ConnectionStringParser::Key::aadAppId);

  if (aadAppId.IsSet() && !config.IsAADAppIdSet())
    config.SetAADAppId(aadAppId.GetValue());

  SettableValue< std::string > aadClientSecret =
      ReadDsnString(dsn, ConnectionStringParser::Key::aadClientSecret);

  if (aadClientSecret.IsSet() && !config.IsAADClientSecretSet())
    config.SetAADClientSecret(aadClientSecret.GetValue());

  SettableValue< std::string > aadTenant =
      ReadDsnString(dsn, ConnectionStringParser::Key::aadTenant);

  if (aadTenant.IsSet() && !config.IsAADTenantSet())
    config.SetAADTenant(aadTenant.GetValue());

  SettableValue< std::string > logLevel =
      ReadDsnString(dsn, ConnectionStringParser::Key::logLevel);

  if (logLevel.IsSet() && !config.IsLogLevelSet()) {
    LogLevel::Type level = LogLevel::FromString(logLevel.GetValue(),
                                                LogLevel::Type::WARNING_LEVEL);
    config.SetLogLevel(level);
  }

  SettableValue< std::string > logPath =
      ReadDsnString(dsn, ConnectionStringParser::Key::logPath);

  if (logPath.IsSet() && !config.IsLogPathSet())
    config.SetLogPath(logPath.GetValue());

  SettableValue< int32_t > maxRowPerPage =
      ReadDsnInt(dsn, ConnectionStringParser::Key::maxRowPerPage);

  if (maxRowPerPage.IsSet() && !config.IsMaxRowPerPageSet())
    config.SetMaxRowPerPage(maxRowPerPage.GetValue());
}

bool WriteDsnConfiguration(const config::Configuration& config,
                           IgniteError& error) {
  LOG_DEBUG_MSG("WriteDsnConfiguration is called");
  if (config.GetDsn("").empty() || config.GetDriver().empty()) {
    return false;
  }
  return RegisterDsn(
      config, reinterpret_cast< const LPCSTR >(config.GetDriver().c_str()),
      error);
}

bool DeleteDsnConfiguration(const std::string dsn, IgniteError& error) {
  return UnregisterDsn(dsn, error);
}

bool RegisterDsn(const Configuration& config, const LPCSTR driver,
                 IgniteError& error) {
  LOG_DEBUG_MSG("RegisterDsn is called");
  using namespace timestream::odbc::config;
  using timestream::odbc::common::LexicalCast;

  typedef Configuration::ArgumentMap ArgMap;

  const char* dsn = config.GetDsn().c_str();
  LOG_DEBUG_MSG("dsn is " << dsn << ", driver is " << driver);

  std::vector< SQLWCHAR > dsn0 = ToWCHARVector(dsn);
  std::vector< SQLWCHAR > driver0 = ToWCHARVector(driver);
  if (!SQLWriteDSNToIni(dsn0.data(), driver0.data())) {
    GetLastSetupError(error);
    return false;
  }

  ArgMap map;

  config.ToMap(map);

  map.erase(ConnectionStringParser::Key::dsn);
  map.erase(ConnectionStringParser::Key::driver);

  for (ArgMap::const_iterator it = map.begin(); it != map.end(); ++it) {
    const std::string& key = it->first;
    const std::string& value = it->second;

    if (!WriteDsnString(dsn, key.c_str(), value.c_str(), error)) {
      return false;
    }
  }

  return true;
}

bool UnregisterDsn(const std::string& dsn, IgniteError& error) {
  LOG_DEBUG_MSG("UnregisterDsn is called");
  if (!SQLRemoveDSNFromIni(ToWCHARVector(dsn).data())) {
    GetLastSetupError(error);
    return false;
  }
  return true;
}
}  // namespace odbc
}  // namespace timestream
