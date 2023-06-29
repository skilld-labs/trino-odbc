/*
 * Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

#include "timestream/odbc/log_level.h"

#include <timestream/odbc/utils.h>
#include <timestream/odbc/utility.h>

namespace timestream {
namespace odbc {
LogLevel::Type LogLevel::FromString(const std::string& val, Type dflt) {
  std::string logLevel = utility::Trim(val);

  if (logLevel == "4")
    return LogLevel::Type::DEBUG_LEVEL;

  if (logLevel == "3")
    return LogLevel::Type::INFO_LEVEL;

  if (logLevel == "2")
    return LogLevel::Type::WARNING_LEVEL;

  if (logLevel == "1")
    return LogLevel::Type::ERROR_LEVEL;

  if (logLevel == "0")
    return LogLevel::Type::OFF;

  return dflt;
}

std::string LogLevel::ToString(Type val) {
  switch (val) {
    case LogLevel::Type::DEBUG_LEVEL:
      return "4";

    case LogLevel::Type::INFO_LEVEL:
      return "3";

    case LogLevel::Type::WARNING_LEVEL:
      return "2";

    case LogLevel::Type::ERROR_LEVEL:
      return "1";

    case LogLevel::Type::OFF:
      return "0";

    default:
      return "unknown";
  }
}

std::wstring LogLevel::ToCBString(Type val) {
  switch (val) {
    case LogLevel::Type::DEBUG_LEVEL:
      return L"Debug";

    case LogLevel::Type::INFO_LEVEL:
      return L"Info";

    case LogLevel::Type::WARNING_LEVEL:
      return L"Warning";

    case LogLevel::Type::ERROR_LEVEL:
      return L"Error";

    case LogLevel::Type::OFF:
      return L"Off";

    default:
      return L"Error";
  }
}
}  // namespace odbc
}  // namespace timestream
