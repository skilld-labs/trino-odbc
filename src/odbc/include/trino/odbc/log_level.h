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

#ifndef _TRINO_ODBC_LOG_LEVEL
#define _TRINO_ODBC_LOG_LEVEL

#include <string>
#include <ignite/common/common.h>

namespace trino {
namespace odbc {
/** Log Level enum. */
struct IGNITE_IMPORT_EXPORT LogLevel {
  enum class Type {
    OFF,
    ERROR_LEVEL,
    WARNING_LEVEL,
    INFO_LEVEL,
    DEBUG_LEVEL,
    UNKNOWN
  };

  /**
   * Convert log level from string.
   *
   * @param val String value.
   * @param dflt Default value to return on error.
   * @return Corresponding enum value.
   */
  static Type FromString(const std::string& val, Type dflt = Type::UNKNOWN);

  /**
   * Convert log level to string.
   *
   * @param val Value to convert.
   * @return String value.
   */
  static std::string ToString(Type val);

  /**
   * Convert log level to comboBox string.
   *
   * @param val Value to convert.
   * @return String value.
   */
  static std::wstring ToCBString(Type val);
};
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_LOG_LEVEL
