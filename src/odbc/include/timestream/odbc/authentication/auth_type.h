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

#ifndef _TIMESTREAM_ODBC_AUTHENTICATION_AUTH_TYPE
#define _TIMESTREAM_ODBC_AUTHENTICATION_AUTH_TYPE

#include <string>
#include <ignite/common/common.h>

namespace timestream {
namespace odbc {
/** Auth Type enum. */
struct IGNITE_IMPORT_EXPORT AuthType {
  // enum class Type { AWS_PROFILE, IAM, AAD, OKTA };
  enum class Type { PASSWORD, OAUTH2, KERBEROS, CERTIFICATE, JWT, HEADER };

  /**
   * Convert Auth Type from string.
   *
   * @param val String value.
   * @param dflt Default value to return on error.
   * @return Corresponding enum value.
   */
  static Type FromString(const std::string& val, Type dflt = Type::PASSWORD);

  /**
   * Convert Auth Type to string.
   *
   * @param val Value to convert.
   * @return String value.
   */
  static std::string ToString(Type val);

  /**
   * Convert Auth Type to combo box string.
   *
   * @param val Value to convert.
   * @return String value.
   */
  static std::wstring ToCBString(Type val);
};
}  // namespace odbc
}  // namespace timestream
#endif  //_TIMESTREAM_ODBC_AUTHENTICATION_AUTH_TYPE
