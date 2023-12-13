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

#include "trino/odbc/connection.h"

#include <trino/odbc/ignite_error.h>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <sstream>

#include "trino/odbc/utils.h"
#include "trino/odbc/config/configuration.h"
#include "trino/odbc/config/connection_string_parser.h"
#include "trino/odbc/dsn_config.h"
#include "trino/odbc/environment.h"
#include "trino/odbc/log.h"
#include "trino/odbc/statement.h"
#include "trino/odbc/system/system_dsn.h"
#include "trino/odbc/utility.h"

/*@*/
#include "trino/odbc/authentication/aad.h"
#include "trino/odbc/authentication/okta.h"

/*#*/
#include <aws/trino-query/model/QueryRequest.h>
#include <aws/trino-query/model/QueryResult.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/client/DefaultRetryStrategy.h>

using namespace trino::odbc;
using namespace ignite::odbc::common;
using trino::odbc::IgniteError;

namespace trino {
namespace odbc {

Connection::Connection()
    : info_(config_), metadataID_(false) {
  LOG_DEBUG_MSG("Connection is called");
}

Connection::~Connection() {
  Close();
}

const config::ConnectionInfo& Connection::GetInfo() const {
  return info_;
}

void Connection::GetInfo(config::ConnectionInfo::InfoType type, void* buf,
                         short buflen, short* reslen) {
  LOG_INFO_MSG("SQLGetInfo called: "
               << type << " (" << config::ConnectionInfo::InfoTypeToString(type)
               << "), " << std::hex << reinterpret_cast< size_t >(buf) << ", "
               << buflen << ", " << std::hex
               << reinterpret_cast< size_t >(reslen) << std::dec);

  IGNITE_ODBC_API_CALL(InternalGetInfo(type, buf, buflen, reslen));
}

SqlResult::Type Connection::InternalGetInfo(
    config::ConnectionInfo::InfoType type, void* buf, short buflen,
    short* reslen) {
  const config::ConnectionInfo& info = GetInfo();

  SqlResult::Type res = info.GetInfo(type, buf, buflen, reslen);

  if (res != SqlResult::AI_SUCCESS) {
    std::stringstream ss;
    ss << "SQLGetInfo input " << type << " is not implemented.";

    AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED, ss.str(),
                    trino::odbc::LogLevel::Type::INFO_LEVEL);
  }

  return res;
}

void Connection::Establish(const std::string& connectStr, void* parentWindow) {
  IGNITE_ODBC_API_CALL(InternalEstablish(connectStr, parentWindow));
}

SqlResult::Type Connection::InternalEstablish(const std::string& connectStr,
                                              void* parentWindow) {
  LOG_DEBUG_MSG("InternalEstablish is called");
  config::ConnectionStringParser parser(config_);
  parser.ParseConnectionString(connectStr, &GetDiagnosticRecords());

  if (config_.IsDsnSet()) {
    std::string dsn = config_.GetDsn();
    LOG_DEBUG_MSG("dsn is " << dsn);

    ReadDsnConfiguration(dsn.c_str(), config_, &GetDiagnosticRecords());
  }

#ifdef _WIN32
  if (parentWindow) {
    LOG_DEBUG_MSG("Parent window is passed. Creating configuration window.");
    if (!DisplayConnectionWindow(parentWindow, config_)) {
      AddStatusRecord(odbc::SqlState::SHY008_OPERATION_CANCELED,
                      "Connection canceled by user",
                      trino::odbc::LogLevel::Type::INFO_LEVEL);

      return SqlResult::AI_ERROR;
    }
  }
#endif  // _WIN32

  return InternalEstablish(config_);
}

void Connection::Establish(const config::Configuration& cfg) {
  IGNITE_ODBC_API_CALL(InternalEstablish(cfg));
}

SqlResult::Type Connection::InternalEstablish(
    const config::Configuration& cfg) {
  LOG_DEBUG_MSG("InternalEstablish is called");
  config_ = cfg;

  if (queryClient_) {
    AddStatusRecord(SqlState::S08002_ALREADY_CONNECTED, "Already connected.",
                    trino::odbc::LogLevel::Type::INFO_LEVEL);

    return SqlResult::AI_ERROR;
  }

  try {
    config_.Validate();
  } catch (const ignite::odbc::OdbcError& err) {
    AddStatusRecord(err);
    return SqlResult::AI_ERROR;
  }

  IgniteError err;
  bool connected = TryRestoreConnection(cfg, err); /*PPP3*/

  if (!connected) {
    std::string errMessage = "Failed to establish connection to Trino.\n";
    errMessage.append(err.GetText());
    AddStatusRecord(SqlState::S08001_CANNOT_CONNECT, errMessage);

    return SqlResult::AI_ERROR;
  }

  bool errors = GetDiagnosticRecords().GetStatusRecordsNumber() > 0;

  LOG_DEBUG_MSG("errors is " << errors);

  return errors ? SqlResult::AI_SUCCESS_WITH_INFO : SqlResult::AI_SUCCESS;
}

void Connection::Release() {
  IGNITE_ODBC_API_CALL(InternalRelease());
}

void Connection::Deregister() {
  env_->DeregisterConnection(this);
}

std::shared_ptr< Aws::TrinoQuery::TrinoQueryClient > /*@*/
Connection::GetQueryClient() const {
  return queryClient_;
}

SqlResult::Type Connection::InternalRelease() {
  LOG_DEBUG_MSG("InternalRelease is called");
  if (!queryClient_) {
    AddStatusRecord(SqlState::S08003_NOT_CONNECTED, "Connection is not open.",
                    trino::odbc::LogLevel::Type::WARNING_LEVEL);

    Close();

    // It is important to return SUCCESS_WITH_INFO and not ERROR here, as if we
    // return an error, Windows Driver Manager may decide that connection is not
    // valid anymore which results in memory leak.
    return SqlResult::AI_SUCCESS_WITH_INFO;
  }

  Close();

  return SqlResult::AI_SUCCESS;
}

void Connection::Close() {
  if (queryClient_) {
    queryClient_.reset(); /*$*/
  }

}

Statement* Connection::CreateStatement() {
  Statement* statement;

  IGNITE_ODBC_API_CALL(InternalCreateStatement(statement));

  return statement;
}

SqlResult::Type Connection::InternalCreateStatement(Statement*& statement) {
  LOG_DEBUG_MSG("InternalCreateStatement is called");
  statement = new Statement(*this);

  if (!statement) {
    AddStatusRecord(SqlState::SHY001_MEMORY_ALLOCATION, "Not enough memory.");

    return SqlResult::AI_ERROR;
  }

  // set statement attributes that are set to this connection
  statement->SetAttribute(stmtAttr_);
  return SqlResult::AI_SUCCESS;
}

const config::Configuration& Connection::GetConfiguration() const {
  return config_;
}

bool Connection::IsAutoCommit() const {
  return autoCommit_;
}

diagnostic::DiagnosticRecord Connection::CreateStatusRecord(
    SqlState::Type sqlState, const std::string& message, int32_t rowNum,
    int32_t columnNum) {
  return diagnostic::DiagnosticRecord(sqlState, message, "", "", rowNum,
                                      columnNum);
}

int32_t Connection::GetEnvODBCVer() {
  SQLINTEGER version;
  SqlLen outResLen;

  app::ApplicationDataBuffer outBuffer(
      type_traits::OdbcNativeType::AI_SIGNED_LONG, &version, 0, &outResLen);
  env_->GetAttribute(SQL_ATTR_ODBC_VERSION, outBuffer);

  return outBuffer.GetInt32();
}

void Connection::GetAttribute(int attr, void* buf, SQLINTEGER bufLen,
                              SQLINTEGER* valueLen) {
  IGNITE_ODBC_API_CALL(InternalGetAttribute(attr, buf, bufLen, valueLen));
}

SqlResult::Type Connection::InternalGetAttribute(int attr, void* buf,
                                                 SQLINTEGER,
                                                 SQLINTEGER* valueLen) {
  LOG_DEBUG_MSG("InternalGetAttribute is called, attr is " << attr);
  if (!buf) {
    AddStatusRecord(SqlState::SHY009_INVALID_USE_OF_NULL_POINTER,
                    "Data buffer is null.");

    return SqlResult::AI_ERROR;
  }

  switch (attr) {
    case SQL_ATTR_CONNECTION_DEAD: {
      SQLUINTEGER* val = reinterpret_cast< SQLUINTEGER* >(buf);

      *val = queryClient_ ? SQL_CD_FALSE : SQL_CD_TRUE;

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;
    }

    case SQL_ATTR_CONNECTION_TIMEOUT: {
      SQLUINTEGER* val = reinterpret_cast< SQLUINTEGER* >(buf);

      // connection timeout is disabled. Same behavior as version 1.0
      *val = 0;

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;

      break;
    }

    case SQL_ATTR_AUTOCOMMIT: {
      SQLUINTEGER* val = reinterpret_cast< SQLUINTEGER* >(buf);

      *val = autoCommit_ ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;

      break;
    }

    case SQL_ATTR_METADATA_ID: {
      SQLUINTEGER* val = reinterpret_cast< SQLUINTEGER* >(buf);

      *val = metadataID_ ? SQL_TRUE : SQL_FALSE;

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;

      break;
    }

    case SQL_ATTR_AUTO_IPD: {
      SQLUINTEGER* val = reinterpret_cast< SQLUINTEGER* >(buf);

      // could only be false as we do not support SQLPrepare.
      *val = false;

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;

      break;
    }

    case SQL_ATTR_ASYNC_ENABLE: {
      SQLUINTEGER* val = reinterpret_cast< SQLUINTEGER* >(buf);

      // Asynchronous execution is not supported
      *val = SQL_ASYNC_ENABLE_OFF;

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;

      break;
    }

    case SQL_ATTR_TRINOLOG_DEBUG: {
      SQLUINTEGER* val = reinterpret_cast< SQLUINTEGER* >(buf);

      std::shared_ptr< Logger > logger = Logger::GetLoggerInstance();
      *val = static_cast< SQLUINTEGER >(logger->GetLogLevel());

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;

      break;
    }

    default: {
      AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                      "Specified attribute is not supported.",
                      trino::odbc::LogLevel::Type::INFO_LEVEL);

      return SqlResult::AI_ERROR;
    }
  }

  LOG_DEBUG_MSG("buf: " << *reinterpret_cast< SQLUINTEGER* >(buf));

  return SqlResult::AI_SUCCESS;
}

void Connection::SetAttribute(int attr, void* value, SQLINTEGER valueLen) {
  IGNITE_ODBC_API_CALL(InternalSetAttribute(attr, value, valueLen));
}

SqlResult::Type Connection::InternalSetAttribute(int attr, void* value,
                                                 SQLINTEGER) {
  LOG_DEBUG_MSG("InternalSetAttribute is called, attr is " << attr);
  switch (attr) {
    case SQL_ATTR_CONNECTION_DEAD: {
      AddStatusRecord(SqlState::SHY092_OPTION_TYPE_OUT_OF_RANGE,
                      "Attribute is read only.");

      return SqlResult::AI_ERROR;
    }

    case SQL_ATTR_AUTOCOMMIT: {
      SQLUINTEGER mode =
          static_cast< SQLUINTEGER >(reinterpret_cast< ptrdiff_t >(value));

      if (mode != SQL_AUTOCOMMIT_ON && mode != SQL_AUTOCOMMIT_OFF) {
        AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                        "Specified attribute is not supported.",
                        trino::odbc::LogLevel::Type::INFO_LEVEL);

        return SqlResult::AI_ERROR;
      }

      autoCommit_ = mode == SQL_AUTOCOMMIT_ON;

      break;
    }

    case SQL_ATTR_METADATA_ID: {
      SQLUINTEGER id =
          static_cast< SQLUINTEGER >(reinterpret_cast< ptrdiff_t >(value));

      if (id != SQL_TRUE && id != SQL_FALSE) {
        AddStatusRecord(SqlState::SHY024_INVALID_ATTRIBUTE_VALUE,
                        "Invalid argument value");

        return SqlResult::AI_ERROR;
      }
      LOG_INFO_MSG("SQL_ATTR_METADATA_ID is set to " << id);
      metadataID_ = id;

      break;
    }

    case SQL_ATTR_ANSI_APP: {
      // According to Microsoft, if a driver exhibits the same behavior for both
      // ANSI and Unicode applications, it should return SQL_ERROR for this
      // attribute. Our driver has same behavior for ANSI and Unicode
      // applications.
      AddStatusRecord(SqlState::SHY000_GENERAL_ERROR,
                      "Same behavior for ANSI and Unicode applications.");

      return SqlResult::AI_ERROR;
    }

    case SQL_ATTR_TRINOLOG_DEBUG: {
      LogLevel::Type type =
          static_cast< LogLevel::Type >(reinterpret_cast< ptrdiff_t >(value));

      std::shared_ptr< Logger > logger = Logger::GetLoggerInstance();
      logger->SetLogLevel(type);
      LOG_INFO_MSG("log level is set to " << static_cast< int >(type));
      break;
    }
    default: {
      AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                      "Specified attribute is not supported.");

      return SqlResult::AI_ERROR;
    }
  }

  return SqlResult::AI_SUCCESS;
}

/**
 * Updates connection runtime information used by SQLGetInfo.
 *
 */
void UpdateConnectionRuntimeInfo(const config::Configuration& config,
                                 config::ConnectionInfo& info) {
#ifdef SQL_USER_NAME
  info.SetInfo(SQL_USER_NAME, config.GetDSNUserName());
#endif
#ifdef SQL_DATA_SOURCE_NAME
  info.SetInfo(SQL_DATA_SOURCE_NAME, config.GetDsn());
#endif
}

std::shared_ptr< Aws::Http::HttpClient > Connection::GetHttpClient() { /*@*/
  return Aws::Http::CreateHttpClient(Aws::Client::ClientConfiguration()); /*#*/
}

Aws::Utils::Logging::LogLevel Connection::GetAWSLogLevelFromString(std::string trinoLogLvl) { /*@*/
  std::transform(trinoLogLvl.begin(), trinoLogLvl.end(), trinoLogLvl.begin(), ::toupper);

  switch (trinoLogLvl) {
  case "OFF":
    return Aws::Utils::Logging::LogLevel::Off; /*@*/

  case "FATAL":
    return Aws::Utils::Logging::LogLevel::Fatal; /*@*/

  case "ERROR":
    return Aws::Utils::Logging::LogLevel::Error; /*@*/

  case "WARN":
    return Aws::Utils::Logging::LogLevel::Warn; /*@*/

  case "INFO":
    return Aws::Utils::Logging::LogLevel::Info; /*@*/

  case "DEBUG":
    return Aws::Utils::Logging::LogLevel::Debug; /*@*/

  case "TRACE":
    return Aws::Utils::Logging::LogLevel::Trace; /*@*/
  
  default:
    return Aws::Utils::Logging::LogLevel::Warn; /*@*/
  }
}

void Connection::SetClientProxy(Aws::Client::ClientConfiguration& clientCfg) { /*@*/
  LOG_DEBUG_MSG("SetClientProxy is called");
  // proxy host
  std::string proxyHost = utility::Trim(GetEnv("TRINO_PROXY_HOST"));
  if (!proxyHost.empty()) {
    LOG_DEBUG_MSG("proxy host is " << proxyHost);
    clientCfg.proxyHost = proxyHost; /*@*/
  }

  // proxy port
  int proxyPort = 0;
  std::string portStr = utility::Trim(GetEnv("TRINO_PROXY_PORT"));
  if (!portStr.empty()) {
    LOG_DEBUG_MSG("proxy port is " << portStr);
    proxyPort = utility::StringToInt(portStr);
  }
  if (proxyPort > 0) {
    clientCfg.proxyPort = proxyPort; /*@*/
  }

  // proxy scheme
  std::string proxyScheme = utility::Trim(GetEnv("TRINO_PROXY_SCHEME"));
  if (!proxyScheme.empty()) {
    LOG_DEBUG_MSG("proxy scheme is " << proxyScheme);
    std::transform(proxyScheme.begin(), proxyScheme.end(), proxyScheme.begin(),
                   ::toupper);
    if (proxyScheme == "HTTPS") {
      clientCfg.proxyScheme = Aws::Http::Scheme::HTTPS; /*@*/
    } else {
      clientCfg.proxyScheme = Aws::Http::Scheme::HTTP; /*@*/
    }
  }

  // proxy user name
  std::string proxyUser = utility::Trim(GetEnv("TRINO_PROXY_USER"));
  if (!proxyUser.empty()) {
    LOG_DEBUG_MSG("proxy username is set");
    clientCfg.proxyUserName = proxyUser; /*@*/
  }

  // proxy user password
  std::string proxyPassword = utility::Trim(GetEnv("TRINO_PROXY_PASSWORD"));
  if (!proxyPassword.empty()) {
    LOG_DEBUG_MSG("proxy user password is set");
    clientCfg.proxyPassword = proxyPassword; /*@*/
  }

  // proxy SSL certificate path
  std::string proxySSLCertPath =
      utility::Trim(GetEnv("TRINO_PROXY_SSL_CERT_PATH"));
  if (!proxySSLCertPath.empty()) {
    LOG_DEBUG_MSG("proxy SSL certificate path is " << proxySSLCertPath);
    clientCfg.proxySSLCertPath = proxySSLCertPath; /*@*/
  }

  // proxy SSL certificate type
  std::string proxySSLCertType =
      utility::Trim(GetEnv("TRINO_PROXY_SSL_CERT_TYPE"));
  if (!proxySSLCertType.empty()) {
    LOG_DEBUG_MSG("proxy SSL certificate type is " << proxySSLCertType);
    clientCfg.proxySSLCertType = proxySSLCertType; /*@*/
  }

  // proxy SSL key path
  std::string proxySSLKeyPath = utility::Trim(GetEnv("TRINO_PROXY_SSL_KEY_PATH"));
  if (!proxySSLKeyPath.empty()) {
    LOG_DEBUG_MSG("proxy SSL key path is " << proxySSLKeyPath);
    clientCfg.proxySSLKeyPath = proxySSLKeyPath; /*@*/
  }

  // proxy SSL key type
  std::string proxySSLKeyType = utility::Trim(GetEnv("TRINO_PROXY_SSL_KEY_TYPE"));
  if (!proxySSLKeyType.empty()) {
    LOG_DEBUG_MSG("proxy SSL key type is " << proxySSLKeyType);
    clientCfg.proxySSLCertType = proxySSLKeyType; /*@*/
  }

  // proxy SSL key password
  std::string proxySSLKeyPassword =
      utility::Trim(GetEnv("TRINO_PROXY_SSL_KEY_PASSWORD"));
  if (!proxySSLKeyPassword.empty()) {
    LOG_DEBUG_MSG("proxy SSL key password is set");
    clientCfg.proxySSLKeyPassword = proxySSLKeyPassword; /*@*/
  }
}

std::shared_ptr< Aws::STS::STSClient > Connection::GetStsClient() { /*@*/
  return std::make_shared< Aws::STS::STSClient >(); /*@*/
}

bool Connection::TryRestoreConnection(const config::Configuration& cfg,
                                      IgniteError& err) { /*$*/
  LOG_DEBUG_MSG("TryRestoreConnection is called");
  // Aws::Auth::AWSCredentials credentials; /*$*/
  std::string errInfo("");

  AuthType::Type authType = cfg.GetAuthType();
  LOG_DEBUG_MSG("auth type is " << static_cast< int >(authType));
  if (authType == AuthType::Type::PASSWORD) {
    // Aws::Auth::ProfileConfigFileAWSCredentialsProvider credProvider(cfg.GetProfileName().data()); /*#*/ /*$*/
    // credentials = credProvider.GetAWSCredentials(); /*@*/
    LOG_DEBUG_MSG("profile name is " << cfg.GetProfileName());
  } else {
    std::string errMsg =
        "AuthType is not PASSWORD, but "
        "TryRestoreConnection is "
        "called.";
    LOG_ERROR_MSG(errMsg);
    err = IgniteError(IgniteError::IGNITE_ERR_TRINO_CONNECT, errMsg.data());

    Close();
    return false;
  }

  // if (credentials.IsExpiredOrEmpty()) { /*$*/
  //   if (errInfo.empty())
  //     errInfo += "Empty or expired credentials";

  //   LOG_ERROR_MSG(errInfo);
  //   err = IgniteError(IgniteError::IGNITE_ERR_TRINO_CONNECT, errInfo.data());

  //   Close();
  //   return false;
  // }

  // Aws::Client::ClientConfiguration clientCfg; /*#*/ /*$*/
  // clientCfg.region = cfg.GetRegion();
  // clientCfg.enableEndpointDiscovery = true;
  // clientCfg.connectTimeoutMs = cfg.GetConnectionTimeout();
  // clientCfg.requestTimeoutMs = cfg.GetReqTimeout();
  // clientCfg.maxConnections = cfg.GetMaxConnections();

#if defined(_WIN32)
  std::string platform("Windows");
#elif defined(__APPLE__)
  std::string platform("macOS");
#else
  std::string platform("Linux");
#endif
  // pass driver info to Trino as user agent
  // clientCfg.userAgent = "trino-odbc." + utility::GetFormatedDriverVersion() + " on " + platform; /*$*/
  // LOG_DEBUG_MSG("region is "
  //               << cfg.GetRegion() << ", connection timeout is "
  //               << clientCfg.connectTimeoutMs << ", request timeout is "
  //               << clientCfg.requestTimeoutMs << ", max connection is "
  //               << clientCfg.maxConnections << ", user agent is "
  //               << clientCfg.userAgent); /*$*/

  // SetClientProxy(clientCfg); /*$*/

  if (cfg.GetMaxRetryCountClient() > 0) {
    // clientCfg.retryStrategy =
    //     std::make_shared< Aws::Client::DefaultRetryStrategy >(cfg.GetMaxRetryCountClient()); /*#*/ /*$*/
    LOG_DEBUG_MSG("max retry count is " << cfg.GetMaxRetryCountClient());
  }

  queryClient_ = CreateTRINOQueryClient(/* credentials, clientCfg */); /*$*/

  const std::string& endpoint = cfg.GetEndpoint();
  // endpoint could not be set to empty string
  if (!endpoint.empty()) {
    queryClient_->OverrideEndpoint(endpoint);
    LOG_DEBUG_MSG("endpoint is set to " << endpoint);
  }
  // try a simple query with query client
  Aws::TrinoQuery::Model::QueryRequest queryRequest; /*#*/
  queryRequest.SetQueryString("SELECT 1");

  Aws::TrinoQuery::Model::QueryOutcome outcome = queryClient_->Query(queryRequest); /*#*/ /*@*/
  if (!outcome.IsSuccess()) {
    auto error = outcome.GetError();
    LOG_DEBUG_MSG("ERROR: " << error.GetExceptionName() << ": "
                            << error.GetMessage());

    err = IgniteError(IgniteError::IGNITE_ERR_TRINO_CONNECT,
                      std::string(error.GetExceptionName())
                          .append(": ")
                          .append(error.GetMessage())
                          .c_str());

    Close();
    return false;
  }

  UpdateConnectionRuntimeInfo(config_, info_);

  return true;
}

std::shared_ptr< Aws::TrinoQuery::TrinoQueryClient > /*@*/
Connection::CreateTRINOQueryClient(
    const Aws::Auth::AWSCredentials& credentials,
    const Aws::Client::ClientConfiguration& clientCfg) { /*@*/ /*$*/
  return std::make_shared< Aws::TrinoQuery::TrinoQueryClient >(credentials, clientCfg); /*@*/
}

Descriptor* Connection::CreateDescriptor() {
  Descriptor* desc;

  IGNITE_ODBC_API_CALL(InternalCreateDescriptor(desc));

  return desc;
}

SqlResult::Type Connection::InternalCreateDescriptor(Descriptor*& desc) {
  LOG_DEBUG_MSG("InternalCreateDescriptor is called");
  desc = new Descriptor();

  if (!desc) {
    AddStatusRecord(SqlState::SHY001_MEMORY_ALLOCATION, "Not enough memory.");

    return SqlResult::AI_ERROR;
  }

  // The explicit descriptor is created by SQLAllocHandle..
  // It must belong to a connection. It is an application descriptor.
  desc->SetConnection(this);
  desc->InitAppHead(false);

  return SqlResult::AI_SUCCESS;
}

std::string Connection::GetCursorName(const Statement* stmt) {
  LOG_DEBUG_MSG("GetCursorName is called");

  std::map< const Statement*, std::string >::iterator itr =
      cursorNameMap_.find(stmt);
  if (itr != cursorNameMap_.end()) {
    return itr->second;
  }
  return "";
}

bool Connection::CursorNameExists(std::string& cursorName) {
  LOG_DEBUG_MSG("CursorNameExists is called");

  bool retval = false;
  if (cursorNames_.find(cursorName) != cursorNames_.end()) {
    retval = true;
  }
  return retval;
}

SqlResult::Type Connection::AddCursorName(const Statement* stmt, std::string& cursorName) {
  LOG_DEBUG_MSG("AddCursorName is called");

  // in case multiple statement in different threads
  std::lock_guard< std::mutex > lock(cursorNameMutex_);
  std::map< const Statement*, std::string >::iterator itr = cursorNameMap_.find(stmt);
  if (itr != cursorNameMap_.end()) {
    cursorNames_.erase(itr->second);
  }
  cursorNameMap_[stmt] = cursorName;
  cursorNames_.insert(cursorName);

  return SqlResult::AI_SUCCESS;
}

void Connection::RemoveCursorName(const Statement* stmt) {
  LOG_DEBUG_MSG("RemoveCursorName is called");

  // locks to support having multiple statement in different threads
  std::lock_guard< std::mutex > lock(cursorNameMutex_);
  std::map< const Statement*, std::string >::iterator itr = cursorNameMap_.find(stmt);
  if (itr != cursorNameMap_.end()) {
    cursorNames_.erase(itr->second);
    cursorNameMap_.erase(itr);
  }
}

#if defined(__APPLE__)
void Connection::GetFunctions(SQLUSMALLINT funcId, SQLUSMALLINT* valueBuf) {
  IGNITE_ODBC_API_CALL(InternalGetFunctions(funcId, valueBuf));
}

SqlResult::Type Connection::InternalGetFunctions(SQLUSMALLINT funcId, SQLUSMALLINT* valueBuf) {
  LOG_DEBUG_MSG("InternalGetFunctions is called, funcId is " << funcId);
  switch (funcId) {
    case SQL_API_ODBC3_ALL_FUNCTIONS: {
      memset(valueBuf, 0, sizeof(UWORD) * SQL_API_ODBC3_ALL_FUNCTIONS_SIZE);
      SetODBC3FunctionsValue(valueBuf);
      break;
    }

    case SQL_API_ALL_FUNCTIONS: {
      memset(valueBuf, 0, sizeof(valueBuf[0]) * 100);
      SetODBC2FunctionsValue(valueBuf);
      break;
    }

    case SQL_API_SQLALLOCHANDLE: {
      *valueBuf = true;
      break;
    }
    case SQL_API_SQLGETDESCFIELD: {
      *valueBuf = true;
      break;
    }
    case SQL_API_SQLBINDCOL: {
      *valueBuf = true;
      break;
    }
    case SQL_API_SQLGETDESCREC: {
      *valueBuf = true;
      break;
    }
    case SQL_API_SQLCANCEL: {
      *valueBuf = true;
      break;
    }
    case SQL_API_SQLGETDIAGFIELD: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLCLOSECURSOR: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETDIAGREC: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLCOLATTRIBUTE: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETENVATTR: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLCONNECT: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETFUNCTIONS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLCOPYDESC: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETINFO: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLDATASOURCES: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETSTMTATTR: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLDESCRIBECOL: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETTYPEINFO: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLDISCONNECT: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLNUMRESULTCOLS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLDRIVERS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLPARAMDATA: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLENDTRAN: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLPREPARE: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLEXECDIRECT: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLPUTDATA: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLEXECUTE: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLROWCOUNT: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLFETCH: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETCONNECTATTR: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLFETCHSCROLL: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETCURSORNAME: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLFREEHANDLE: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETDESCFIELD: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLFREESTMT: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETDESCREC: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETCONNECTATTR: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETENVATTR: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETCURSORNAME: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETSTMTATTR: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETDATA: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLCOLUMNS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSTATISTICS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSPECIALCOLUMNS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLTABLES: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLNATIVESQL: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLBROWSECONNECT: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLNUMPARAMS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLBULKOPERATIONS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLPRIMARYKEYS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLCOLUMNPRIVILEGES: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLPROCEDURECOLUMNS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLDESCRIBEPARAM: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLPROCEDURES: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLDRIVERCONNECT: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETPOS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLFOREIGNKEYS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLTABLEPRIVILEGES: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLMORERESULTS: {
      *valueBuf = true;
      break;
    }

    /* ODBC 2.* functions */
    case SQL_API_SQLALLOCCONNECT: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLALLOCENV: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLALLOCSTMT: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLBINDPARAMETER: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLERROR: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLEXTENDEDFETCH: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLFREECONNECT: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLFREEENV: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETCONNECTOPTION: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLGETSTMTOPTION: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLPARAMOPTIONS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETCONNECTOPTION: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETPARAM: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETSCROLLOPTIONS: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLSETSTMTOPTION: {
      *valueBuf = true;
      break;
    }

    case SQL_API_SQLTRANSACT: {
      *valueBuf = true;
      break;
    }
    default: {
      break;
    }
  }

  return SqlResult::AI_SUCCESS;
}

#define SQL_FUNC_SET(pfExists, uwAPI) \
  (*(((UWORD*)(pfExists)) + ((uwAPI) >> 4)) |= (1 << ((uwAPI)&0x000F)))

void Connection::SetODBC3FunctionsValue(SQLUSMALLINT* valueBuf) {
  SQL_FUNC_SET(valueBuf, SQL_API_SQLALLOCHANDLE);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETDESCFIELD);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLBINDCOL);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETDESCREC);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLCANCEL);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETDIAGFIELD);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLCLOSECURSOR);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETDIAGREC);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLCOLATTRIBUTE);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETENVATTR);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLCONNECT);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETFUNCTIONS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLCOPYDESC);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETINFO);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLDATASOURCES);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETSTMTATTR);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLDESCRIBECOL);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETTYPEINFO);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLDISCONNECT);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLNUMRESULTCOLS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLDRIVERS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLPARAMDATA);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLENDTRAN);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLPREPARE);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLEXECDIRECT);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLPUTDATA);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLEXECUTE);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLROWCOUNT);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLFETCH);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLSETCONNECTATTR);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLFETCHSCROLL);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLSETCURSORNAME);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLFREEHANDLE);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLSETDESCFIELD);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLFREESTMT);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLSETDESCREC);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETCONNECTATTR);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLSETENVATTR);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETCURSORNAME);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLSETSTMTATTR);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLGETDATA);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLCOLUMNS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLSTATISTICS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLSPECIALCOLUMNS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLTABLES);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLBINDPARAMETER);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLNATIVESQL);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLBROWSECONNECT);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLNUMPARAMS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLBULKOPERATIONS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLPRIMARYKEYS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLCOLUMNPRIVILEGES);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLPROCEDURECOLUMNS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLDESCRIBEPARAM);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLPROCEDURES);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLDRIVERCONNECT);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLSETPOS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLFOREIGNKEYS);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLTABLEPRIVILEGES);
  SQL_FUNC_SET(valueBuf, SQL_API_SQLMORERESULTS);
}

void Connection::SetODBC2FunctionsValue(SQLUSMALLINT* valueBuf) {
  valueBuf[SQL_API_SQLALLOCCONNECT] = true;
  valueBuf[SQL_API_SQLALLOCENV] = true;
  valueBuf[SQL_API_SQLALLOCSTMT] = true;
  valueBuf[SQL_API_SQLBINDCOL] = true;
  valueBuf[SQL_API_SQLBINDPARAMETER] = true;
  valueBuf[SQL_API_SQLBROWSECONNECT] = true;
  valueBuf[SQL_API_SQLCANCEL] = true;
  valueBuf[SQL_API_SQLCOLATTRIBUTES] = true;
  valueBuf[SQL_API_SQLCOLUMNPRIVILEGES] = true;
  valueBuf[SQL_API_SQLCOLUMNS] = true;
  valueBuf[SQL_API_SQLCONNECT] = true;
  valueBuf[SQL_API_SQLDATASOURCES] = true;
  valueBuf[SQL_API_SQLDESCRIBECOL] = true;
  valueBuf[SQL_API_SQLDESCRIBEPARAM] = true;
  valueBuf[SQL_API_SQLDISCONNECT] = true;
  valueBuf[SQL_API_SQLDRIVERCONNECT] = true;
  valueBuf[SQL_API_SQLDRIVERS] = true;
  valueBuf[SQL_API_SQLERROR] = true;
  valueBuf[SQL_API_SQLEXECDIRECT] = true;
  valueBuf[SQL_API_SQLEXECUTE] = true;
  valueBuf[SQL_API_SQLEXTENDEDFETCH] = true;
  valueBuf[SQL_API_SQLFETCH] = true;
  valueBuf[SQL_API_SQLFOREIGNKEYS] = true;
  valueBuf[SQL_API_SQLFREECONNECT] = true;
  valueBuf[SQL_API_SQLFREEENV] = true;
  valueBuf[SQL_API_SQLFREESTMT] = true;
  valueBuf[SQL_API_SQLGETCONNECTOPTION] = true;
  valueBuf[SQL_API_SQLGETCURSORNAME] = true;
  valueBuf[SQL_API_SQLGETDATA] = true;
  valueBuf[SQL_API_SQLGETFUNCTIONS] = true;
  valueBuf[SQL_API_SQLGETINFO] = true;
  valueBuf[SQL_API_SQLGETSTMTOPTION] = true;
  valueBuf[SQL_API_SQLGETTYPEINFO] = true;
  valueBuf[SQL_API_SQLMORERESULTS] = true;
  valueBuf[SQL_API_SQLNATIVESQL] = true;
  valueBuf[SQL_API_SQLNUMPARAMS] = true;
  valueBuf[SQL_API_SQLNUMRESULTCOLS] = true;
  valueBuf[SQL_API_SQLPARAMDATA] = true;
  valueBuf[SQL_API_SQLPARAMOPTIONS] = true;
  valueBuf[SQL_API_SQLPREPARE] = true;
  valueBuf[SQL_API_SQLPRIMARYKEYS] = true;
  valueBuf[SQL_API_SQLPROCEDURECOLUMNS] = true;
  valueBuf[SQL_API_SQLPROCEDURES] = true;
  valueBuf[SQL_API_SQLPUTDATA] = true;
  valueBuf[SQL_API_SQLROWCOUNT] = true;
  valueBuf[SQL_API_SQLSETCONNECTOPTION] = true;
  valueBuf[SQL_API_SQLSETCURSORNAME] = true;
  valueBuf[SQL_API_SQLSETPARAM] = true;
  valueBuf[SQL_API_SQLSETPOS] = true;
  valueBuf[SQL_API_SQLSETSCROLLOPTIONS] = true;
  valueBuf[SQL_API_SQLSETSTMTOPTION] = true;
  valueBuf[SQL_API_SQLSPECIALCOLUMNS] = true;
  valueBuf[SQL_API_SQLSTATISTICS] = true;
  valueBuf[SQL_API_SQLTABLEPRIVILEGES] = true;
  valueBuf[SQL_API_SQLTABLES] = true;
  valueBuf[SQL_API_SQLTRANSACT] = true;
}
#endif //__APPLE__

void Connection::SetStmtAttribute(SQLUSMALLINT option, SQLULEN value) {
  IGNITE_ODBC_API_CALL(InternalSetStmtAttribute(option, value));
}

SqlResult::Type Connection::InternalSetStmtAttribute(SQLUSMALLINT option, SQLULEN value) {
  switch (option) {
    case SQL_BIND_TYPE: {
      if (value != SQL_BIND_BY_COLUMN) {
        AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                        "Only binding by column is currently supported");

        return SqlResult::AI_ERROR;
      }
      stmtAttr_.bindType = value;
      break;
    }
    case SQL_CONCURRENCY: {
      if (value != SQL_CONCUR_READ_ONLY) {
        AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                        "Only read-only cursors are supported");

        return SqlResult::AI_ERROR;
      }
      stmtAttr_.concurrency = value;
      break;
    }
    case SQL_CURSOR_TYPE: {
      if (value != SQL_CURSOR_FORWARD_ONLY) {
        AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                        "Only forward cursors are currently supported");

        return SqlResult::AI_ERROR;
      }
      stmtAttr_.cursorType = value;
      break;
    }

    case SQL_RETRIEVE_DATA: {
      if (value != SQL_RD_ON) {
        AddStatusRecord(
            SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
            "SQLFetch can only retrieve data after it positions the cursor");

        return SqlResult::AI_ERROR;
      }
      stmtAttr_.retrievData = value;
      break;
    }
    case SQL_ROWSET_SIZE: {
      if (value > 1000) {
        AddStatusRecord(
            SqlState::SIM001_FUNCTION_NOT_SUPPORTED,
            "Array size value cannot be set to a value other than 1000");

        return SqlResult::AI_ERROR;
      }
      stmtAttr_.rowsetSize = value;
      break;
    }

    // ignored attributes
    case SQL_NOSCAN:
    case SQL_QUERY_TIMEOUT:
    case SQL_MAX_ROWS:
    case SQL_MAX_LENGTH:
    case SQL_KEYSET_SIZE:
    case SQL_ASYNC_ENABLE: {
      AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                      "Specified attribute is ignored.",
                      trino::odbc::LogLevel::Type::WARNING_LEVEL);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    // not supported attributes
    case SQL_USE_BOOKMARKS:
    case SQL_SIMULATE_CURSOR:
    default: {
      AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                      "Specified attribute is not supported");

      return SqlResult::AI_ERROR;
    }
  }
  return SqlResult::AI_SUCCESS;
}

void Connection::SetConnectOption(SQLUSMALLINT option, SQLULEN value) {
  IGNITE_ODBC_API_CALL(InternalSetConnectOption(option, value));
}

SqlResult::Type Connection::InternalSetConnectOption(SQLUSMALLINT option,
                                                     SQLULEN value) {
  switch (option) {
    case SQL_ASYNC_ENABLE:
    case SQL_BIND_TYPE:
    case SQL_CONCURRENCY:
    case SQL_CURSOR_TYPE:
    case SQL_KEYSET_SIZE:
    case SQL_MAX_LENGTH:
    case SQL_MAX_ROWS:
    case SQL_NOSCAN:
    case SQL_QUERY_TIMEOUT:
    case SQL_RETRIEVE_DATA:
    case SQL_ROWSET_SIZE:
    case SQL_SIMULATE_CURSOR:
    case SQL_USE_BOOKMARKS:
      return InternalSetStmtAttribute(option, value);

    // ignored options
    case SQL_TRANSLATE_DLL:
    case SQL_TRANSLATE_OPTION:
    case SQL_TXN_ISOLATION:
    case SQL_ACCESS_MODE:
    case SQL_CURRENT_QUALIFIER:
    case SQL_PACKET_SIZE:
    case SQL_QUIET_MODE:
    case SQL_LOGIN_TIMEOUT: {
      AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                      "Specified attribute is ignored.",
                      trino::odbc::LogLevel::Type::WARNING_LEVEL);
      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case SQL_AUTOCOMMIT:
    default:
      return InternalSetAttribute(option, reinterpret_cast< SQLPOINTER >(value), 0);
  }
}

void Connection::GetConnectOption(SQLUSMALLINT option, SQLPOINTER value) {
  IGNITE_ODBC_API_CALL(InternalGetConnectOption(option, value));
}

SqlResult::Type Connection::InternalGetConnectOption(SQLUSMALLINT option,
                                                     SQLPOINTER value) {
  switch (option) {
    // ignored options
#if defined(__APPLE__)
    case SQL_ODBC_CURSORS:
#endif
    case SQL_TRANSLATE_DLL:
    case SQL_TRANSLATE_OPTION:
    case SQL_QUERY_TIMEOUT:
    case SQL_ACCESS_MODE:
    case SQL_TXN_ISOLATION:
    case SQL_CURRENT_QUALIFIER:
    case SQL_PACKET_SIZE:
    case SQL_QUIET_MODE:
    case SQL_LOGIN_TIMEOUT: {
      AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                      "Specified attribute is ignored.",
                      trino::odbc::LogLevel::Type::WARNING_LEVEL);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case SQL_AUTOCOMMIT:
    default:
      return InternalGetAttribute(option, value, 0, nullptr);
  }
}
}  // namespace odbc
}  // namespace trino
