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

#include "trino/odbc/config/connection_string_parser.h"
#include "trino/odbc/log.h"

#include <vector>
#include <fstream>

#include "trino/odbc/utils.h"
#include "trino/odbc/utility.h"

using namespace trino::odbc;

namespace trino {
namespace odbc {
namespace config {
const std::string ConnectionStringParser::Key::dsn = "dsn";
const std::string ConnectionStringParser::Key::driver = "driver";
const std::string ConnectionStringParser::Key::uid = "uid";
const std::string ConnectionStringParser::Key::pwd = "pwd";
const std::string ConnectionStringParser::Key::profileName = "profilename";
const std::string ConnectionStringParser::Key::reqTimeout = "requesttimeout";
const std::string ConnectionStringParser::Key::maxRetryCountClient = "maxretrycountclient";
const std::string ConnectionStringParser::Key::endpoint = "endpointoverride";
const std::string ConnectionStringParser::Key::authType = "auth";
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

      std::string key = trino::odbc::utility::Trim(
          connect_str.substr(attr_begin, attr_eq_pos - attr_begin));
      std::string value = trino::odbc::utility::Trim(connect_str.substr(
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
  std::string lKey = trino::odbc::common::ToLower(key);

  if (lKey == Key::uid || lKey == Key::pwd) {
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

    if (!trino::odbc::common::AllDigits(value)) {
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

    if (!trino::odbc::common::AllDigits(value)) {
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
  } else if (lKey == Key::endpoint) {
    cfg.SetEndpoint(value);
  } else if (lKey == Key::authType) {
    AuthType::Type authType = AuthType::FromString(value);

    std::string val = utility::Trim(trino::odbc::common::ToLower(value));
    if (val != "password" && authType == AuthType::Type::PASSWORD) {
      if (diag) {
        diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                              "Specified AUTH is not supported. "
                              "Default value used ('PASSWORD').");
      }
      return;
    }

    cfg.SetAuthType(authType);
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

    cfg.SetUid(value);
  } else if (lKey == Key::pwd) {
    if (!cfg.GetPwd().empty() && diag) {
      diag->AddStatusRecord(
          SqlState::S01S02_OPTION_VALUE_CHANGED,
          "Re-writing PWD (have you specified it several times?");
    }

    cfg.SetPwd(value);
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

    if (!trino::odbc::common::AllDigits(value)) {
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
  std::string lower = trino::odbc::common::ToLower(value);

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
}  // namespace trino
