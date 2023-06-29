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

#include "timestream/odbc/config/connection_string_parser.h"
#include "timestream/odbc/log.h"

#include <vector>
#include <fstream>

#include "timestream/odbc/utils.h"
#include "timestream/odbc/utility.h"

using namespace timestream::odbc;

namespace timestream {
namespace odbc {
namespace config {
const std::string ConnectionStringParser::Key::dsn = "dsn";
const std::string ConnectionStringParser::Key::driver = "driver";
const std::string ConnectionStringParser::Key::uid = "uid";
const std::string ConnectionStringParser::Key::pwd = "pwd";
const std::string ConnectionStringParser::Key::accessKeyId = "accesskeyid";
const std::string ConnectionStringParser::Key::secretKey = "secretkey";
const std::string ConnectionStringParser::Key::sessionToken = "sessiontoken";
const std::string ConnectionStringParser::Key::profileName = "profilename";
const std::string ConnectionStringParser::Key::reqTimeout = "requesttimeout";
const std::string ConnectionStringParser::Key::connectionTimeout =
    "connectiontimeout";
const std::string ConnectionStringParser::Key::maxRetryCountClient =
    "maxretrycountclient";
const std::string ConnectionStringParser::Key::maxConnections =
    "maxconnections";
const std::string ConnectionStringParser::Key::endpoint = "endpointoverride";
const std::string ConnectionStringParser::Key::region = "region";
const std::string ConnectionStringParser::Key::authType = "auth";
const std::string ConnectionStringParser::Key::idPHost = "idphost";
const std::string ConnectionStringParser::Key::idPUserName = "idpusername";
const std::string ConnectionStringParser::Key::idPPassword = "idppassword";
const std::string ConnectionStringParser::Key::idPArn = "idparn";
const std::string ConnectionStringParser::Key::oktaAppId = "oktaapplicationid";
const std::string ConnectionStringParser::Key::roleArn = "rolearn";
const std::string ConnectionStringParser::Key::aadAppId = "aadapplicationid";
const std::string ConnectionStringParser::Key::aadClientSecret =
    "aadclientsecret";
const std::string ConnectionStringParser::Key::aadTenant = "aadtenant";
const std::string ConnectionStringParser::Key::logLevel = "loglevel";
const std::string ConnectionStringParser::Key::logPath = "logoutput";
const std::string ConnectionStringParser::Key::maxRowPerPage = "maxrowperpage";

ConnectionStringParser::ConnectionStringParser(Configuration& cfg) : cfg(cfg) {
  // No-op.
}

void ConnectionStringParser::ParseConnectionString(
    const char* str, size_t len, char delimiter,
    diagnostic::DiagnosticRecordStorage* diag) {
  LOG_DEBUG_MSG("ParseConnectionString is called with len is "
                << len << ", delimiter is " << delimiter);
  std::string connect_str(str, len);

  while (connect_str.rbegin() != connect_str.rend()
         && *connect_str.rbegin() == 0)
    connect_str.erase(connect_str.size() - 1);

  while (!connect_str.empty()) {
    size_t attr_begin = connect_str.rfind(delimiter);

    if (attr_begin == std::string::npos)
      attr_begin = 0;
    else
      ++attr_begin;

    size_t attr_eq_pos = connect_str.rfind('=');

    if (attr_eq_pos == std::string::npos)
      attr_eq_pos = 0;

    if (attr_begin < attr_eq_pos) {
      const char* key_begin = connect_str.data() + attr_begin;
      const char* key_end = connect_str.data() + attr_eq_pos;

      const char* value_begin = connect_str.data() + attr_eq_pos + 1;
      const char* value_end = connect_str.data() + connect_str.size();

      std::string key = timestream::odbc::utility::Trim(
          connect_str.substr(attr_begin, attr_eq_pos - attr_begin));
      std::string value = timestream::odbc::utility::Trim(connect_str.substr(
          attr_eq_pos + 1, connect_str.size() - attr_eq_pos));

      if (value[0] == '{' && value[value.size() - 1] == '}')
        value = value.substr(1, value.size() - 2);

      HandleAttributePair(key, value, diag);
    }

    if (!attr_begin)
      break;

    connect_str.erase(attr_begin - 1);
  }
}

void ConnectionStringParser::ParseConnectionString(
    const std::string& str, diagnostic::DiagnosticRecordStorage* diag) {
  ParseConnectionString(str.data(), str.size(), ';', diag);
}

void ConnectionStringParser::ParseConfigAttributes(
    const char* str, diagnostic::DiagnosticRecordStorage* diag) {
  size_t len = 0;

  // Getting list length. List is terminated by two '\0'.
  while (str[len] || str[len + 1])
    ++len;

  ++len;

  ParseConnectionString(str, len, '\0', diag);
}

void ConnectionStringParser::HandleAttributePair(
    const std::string& key, const std::string& value,
    diagnostic::DiagnosticRecordStorage* diag) {
  LOG_DEBUG_MSG("HandleAttributePair is called");
  std::string lKey = timestream::odbc::common::ToLower(key);

  if (lKey == Key::uid || lKey == Key::accessKeyId || lKey == Key::idPUserName
      || lKey == Key::pwd || lKey == Key::secretKey || lKey == Key::sessionToken
      || lKey == Key::idPPassword || lKey == Key::aadClientSecret) {
    LOG_DEBUG_MSG(lKey << " is found");
  } else {
    LOG_DEBUG_MSG("key:value is " << lKey << ":" << value);
  }

  if (lKey == Key::dsn) {
    cfg.SetDsn(value);
  } else if (lKey == Key::profileName) {
    cfg.SetProfileName(value);
  } else if (lKey == Key::reqTimeout) {
    if (value.empty()) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Request Timeout attribute value is empty. Using "
                             "default value.",
                             key, value));
      }
      return;
    }

    if (!timestream::odbc::common::AllDigits(value)) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Request Timeout attribute value contains "
                             "unexpected characters."
                             " Using default value.",
                             key, value));
      }
      return;
    }

    if (value.size() >= sizeof(std::to_string(UINT32_MAX))) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Request Timeout attribute value is too large. "
                             "Using default value.",
                             key, value));
      }
      return;
    }

    int64_t numValue = 0;
    std::stringstream conv;

    conv << value;
    conv >> numValue;

    if (numValue <= 0 || numValue > UINT32_MAX) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Request Timeout attribute value is out of range. "
                             "Using default value.",
                             key, value));
      }
      return;
    }

    cfg.SetReqTimeout(static_cast< uint32_t >(numValue));
  } else if (lKey == Key::connectionTimeout) {
    if (value.empty()) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage(
                "Connection Timeout attribute value is empty. Using "
                "default value.",
                key, value));
      }
      return;
    }

    if (!timestream::odbc::common::AllDigits(value)) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Connection Timeout attribute value contains "
                             "unexpected characters."
                             " Using default value.",
                             key, value));
      }
      return;
    }

    if (value.size() >= sizeof(std::to_string(UINT32_MAX))) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Connection Timeout attribute value is too large. "
                             "Using default value.",
                             key, value));
      }
      return;
    }

    int64_t numValue = 0;
    std::stringstream conv;

    conv << value;
    conv >> numValue;

    if (numValue <= 0 || numValue > UINT32_MAX) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage(
                "Connection Timeout attribute value is out of range. "
                "Using default value.",
                key, value));
      }
      return;
    }

    cfg.SetConnectionTimeout(static_cast< uint32_t >(numValue));
  } else if (lKey == Key::maxRetryCountClient) {
    if (value.empty()) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage(
                "Max Retry Count Client attribute value is empty. Using "
                "default value.",
                key, value));
      }
      return;
    }

    if (!timestream::odbc::common::AllDigits(value)) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Max Retry Count Client attribute value contains "
                             "unexpected characters."
                             " Using default value.",
                             key, value));
      }
      return;
    }

    if (value.size() >= sizeof(std::to_string(UINT32_MAX))) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage(
                "Max Retry Count Client attribute value is too large. "
                "Using default value.",
                key, value));
      }
      return;
    }

    int64_t numValue = 0;
    std::stringstream conv;

    conv << value;
    conv >> numValue;

    if (numValue < 0 || numValue > UINT32_MAX) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage(
                "Max Retry Count Client attribute value is out of range. "
                "Using default value.",
                key, value));
      }
      return;
    }

    cfg.SetMaxRetryCountClient(static_cast< uint32_t >(numValue));
  } else if (lKey == Key::maxConnections) {
    if (value.empty()) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Max Connections attribute value is empty. Using "
                             "default value.",
                             key, value));
      }
      return;
    }

    if (!timestream::odbc::common::AllDigits(value)) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Max Connections attribute value contains "
                             "unexpected characters."
                             " Using default value.",
                             key, value));
      }
      return;
    }

    if (value.size() >= sizeof(std::to_string(UINT32_MAX))) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Max Connections attribute value is too large. "
                             "Using default value.",
                             key, value));
      }
      return;
    }

    int64_t numValue = 0;
    std::stringstream conv;

    conv << value;
    conv >> numValue;

    if (numValue <= 0 || numValue > UINT32_MAX) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Max Connections attribute value is out of range. "
                             "Using default value.",
                             key, value));
      }
      return;
    }

    cfg.SetMaxConnections(static_cast< uint32_t >(numValue));
  } else if (lKey == Key::endpoint) {
    cfg.SetEndpoint(value);
  } else if (lKey == Key::region) {
    cfg.SetRegion(value);
  } else if (lKey == Key::authType) {
    AuthType::Type authType = AuthType::FromString(value);

    std::string val = utility::Trim(timestream::odbc::common::ToLower(value));
    if (val != "aws_profile" && authType == AuthType::Type::AWS_PROFILE) {
      if (diag) {
        diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                              "Specified AUTH is not supported. "
                              "Default value used ('AWS_PROFILE').");
      }
      return;
    }

    cfg.SetAuthType(authType);
  } else if (lKey == Key::idPHost) {
    cfg.SetIdPHost(value);
  } else if (lKey == Key::idPUserName) {
    if (!cfg.GetIdPUserName().empty() && diag) {
      diag->AddStatusRecord(
          SqlState::S01S02_OPTION_VALUE_CHANGED,
          "Re-writing IdPUserName (have you specified it several times?");
    }

    if (!cfg.GetUid().empty()) {
      LOG_WARNING_MSG(
          "UID is already set, but IdPUserName is being set too. Only one of "
          "{UID, IdPUserName} is needed. UID will take precedence when making "
          "a connection.");
    }

    cfg.SetIdPUserName(value);
  } else if (lKey == Key::idPPassword) {
    if (!cfg.GetIdPPassword().empty() && diag) {
      diag->AddStatusRecord(
          SqlState::S01S02_OPTION_VALUE_CHANGED,
          "Re-writing IdPPassword (have you specified it several times?");
    }

    if (!cfg.GetPwd().empty()) {
      LOG_WARNING_MSG(
          "PWD is already set, but IdPPassword is being set too. Only one of "
          "{PWD, IdPPassword} is needed. PWD will take precedence when making "
          "a connection.");
    }

    cfg.SetIdPPassword(value);
  } else if (lKey == Key::idPArn) {
    cfg.SetIdPArn(value);
  } else if (lKey == Key::oktaAppId) {
    cfg.SetOktaAppId(value);
  } else if (lKey == Key::roleArn) {
    cfg.SetRoleArn(value);
  } else if (lKey == Key::aadAppId) {
    cfg.SetAADAppId(value);
  } else if (lKey == Key::aadClientSecret) {
    if (!cfg.GetAADClientSecret().empty() && diag) {
      diag->AddStatusRecord(
          SqlState::S01S02_OPTION_VALUE_CHANGED,
          "Re-writing AADClientSecret (have you specified it several times?");
    }

    cfg.SetAADClientSecret(value);
  } else if (lKey == Key::aadTenant) {
    cfg.SetAADTenant(value);
  } else if (lKey == Key::logLevel) {
    LogLevel::Type level = LogLevel::FromString(value);

    if (level == LogLevel::Type::UNKNOWN) {
      if (diag) {
        diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                              "Specified Log Level is not supported. "
                              "Default value used ('2').");
      }
      return;
    }

    cfg.SetLogLevel(level);
  } else if (lKey == Key::logPath) {
    cfg.SetLogPath(value);
  } else if (lKey == Key::driver) {
    cfg.SetDriver(value);
  } else if (lKey == Key::uid) {
    if (!cfg.GetUid().empty() && diag) {
      diag->AddStatusRecord(
          SqlState::S01S02_OPTION_VALUE_CHANGED,
          "Re-writing UID (have you specified it several times?");
    }

    if (!cfg.GetAccessKeyId().empty()) {
      LOG_WARNING_MSG(
          "AccessKeyId is already set, but UID is being set too. Only one of "
          "{UID, AccessKeyId} is needed. UID will take precedence when making "
          "a connection.");
    }

    if (!cfg.GetIdPUserName().empty()) {
      LOG_WARNING_MSG(
          "IdPUserName is already set, but UID is being set too. Only one of "
          "{UID, IdPUserName} is needed. UID will take precedence when making "
          "a connection.");
    }

    cfg.SetUid(value);
  } else if (lKey == Key::pwd) {
    if (!cfg.GetPwd().empty() && diag) {
      diag->AddStatusRecord(
          SqlState::S01S02_OPTION_VALUE_CHANGED,
          "Re-writing PWD (have you specified it several times?");
    }

    if (!cfg.GetSecretKey().empty()) {
      LOG_WARNING_MSG(
          "SecretKey is already set, but PWD is being set too. Only one of "
          "{PWD, SecretKey} is needed. PWD will take precedence when making a "
          "connection.");
    }

    if (!cfg.GetIdPPassword().empty()) {
      LOG_WARNING_MSG(
          "IdPPassword is already set, but PWD is being set too. Only one of "
          "{PWD, IdPPassword} is needed. PWD will take precedence when making "
          "a connection.");
    }

    cfg.SetPwd(value);
  } else if (lKey == Key::accessKeyId) {
    if (!cfg.GetAccessKeyId().empty() && diag) {
      diag->AddStatusRecord(
          SqlState::S01S02_OPTION_VALUE_CHANGED,
          "Re-writing AccessKeyId (have you specified it several times?");
    }

    if (!cfg.GetUid().empty()) {
      LOG_WARNING_MSG(
          "UID is already set, but AccessKeyId is being set too. Only one of "
          "{UID, AccessKeyId} is needed. UID will take precedence when making "
          "a connection.");
    }

    cfg.SetAccessKeyId(value);
  } else if (lKey == Key::secretKey) {
    if (!cfg.GetSecretKey().empty() && diag) {
      diag->AddStatusRecord(
          SqlState::S01S02_OPTION_VALUE_CHANGED,
          "Re-writing SecretKey (have you specified it several times?");
    }

    if (!cfg.GetPwd().empty()) {
      LOG_WARNING_MSG(
          "PWD is already set, but SecretKey is being set too. Only one of "
          "{PWD, SecretKey} is needed. PWD will take precedence when making a "
          "connection.");
    }

    cfg.SetSecretKey(value);
  } else if (lKey == Key::sessionToken) {
    if (!cfg.GetSessionToken().empty() && diag) {
      diag->AddStatusRecord(
          SqlState::S01S02_OPTION_VALUE_CHANGED,
          "Re-writing SessionToken (have you specified it several times?");
    }

    cfg.SetSessionToken(value);
  } else if (lKey == Key::maxRowPerPage) {
    if (value.empty()) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Max Row Per Page attribute value is empty. Using "
                             "default value.",
                             key, value));
      }
      return;
    }

    if (!timestream::odbc::common::AllDigits(value)) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Max Row Per Page attribute value contains "
                             "unexpected characters."
                             " Using default value.",
                             key, value));
      }
      return;
    }

    if (value.size() >= sizeof(std::to_string(UINT32_MAX))) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage("Max Row Per Page attribute value is too large. "
                             "Using default value.",
                             key, value));
      }
      return;
    }

    int64_t numValue = 0;
    std::stringstream conv;

    conv << value;
    conv >> numValue;

    if (numValue < 0 || numValue > UINT32_MAX) {
      if (diag) {
        diag->AddStatusRecord(
            SqlState::S01S02_OPTION_VALUE_CHANGED,
            MakeErrorMessage(
                "Max Row Per Page attribute value is out of range. "
                "Using default value.",
                key, value));
      }
      return;
    }

    cfg.SetMaxRowPerPage(static_cast< uint32_t >(numValue));
  } else if (diag) {
    std::stringstream stream;

    stream << "Unknown attribute: '" << key << "'. Ignoring.";

    diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED, stream.str());
  }
}

ConnectionStringParser::BoolParseResult::Type
ConnectionStringParser::StringToBool(const std::string& value) {
  std::string lower = timestream::odbc::common::ToLower(value);

  if (lower == "true")
    return BoolParseResult::Type::AI_TRUE;

  if (lower == "false")
    return BoolParseResult::Type::AI_FALSE;

  return BoolParseResult::Type::AI_UNRECOGNIZED;
}

std::string ConnectionStringParser::MakeErrorMessage(const std::string& msg,
                                                     const std::string& key,
                                                     const std::string& value) {
  std::stringstream stream;

  stream << msg << " [key='" << key << "', value='" << value << "']";

  return stream.str();
}
}  // namespace config
}  // namespace odbc
}  // namespace timestream
