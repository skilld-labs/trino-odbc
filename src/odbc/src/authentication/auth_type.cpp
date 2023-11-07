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

#include "timestream/odbc/authentication/auth_type.h"

#include <timestream/odbc/utils.h>
#include <timestream/odbc/utility.h>

/*$*/
namespace timestream {
namespace odbc {
AuthType::Type AuthType::FromString(const std::string& val, Type dflt) {
  std::string lowerVal = utility::Trim(timestream::odbc::common::ToLower(val));

  if (lowerVal == "password")
    return AuthType::Type::PASSWORD;

  if (lowerVal == "oauth2")
    return AuthType::Type::OAUTH2;

  if (lowerVal == "kerberos")
    return AuthType::Type::KERBEROS;

  if (lowerVal == "certificate")
    return AuthType::Type::CERTIFICATE;

  if (lowerVal == "jwt")
    return AuthType::Type::JWT;

  if (lowerVal == "header")
    return AuthType::Type::HEADER;

  return dflt;
}

std::string AuthType::ToString(Type val) {
  switch (val) {
    case AuthType::Type::PASSWORD:
      return "password";

    case AuthType::Type::OAUTH2:
      return "oauth2";

    case AuthType::Type::KERBEROS:
      return "kerberos";

    case AuthType::Type::CERTIFICATE:
      return "certificate";

    case AuthType::Type::JWT:
      return "jwt";

    case AuthType::Type::HEADER:
      return "header";

    default:
      return "password";
  }
}

std::wstring AuthType::ToCBString(Type val) {
  switch (val) {
    case AuthType::Type::PASSWORD:
      return L"Password";

    case AuthType::Type::OAUTH2:
      return L"Oauth2";

    case AuthType::Type::KERBEROS:
      return L"Kerberos";

    case AuthType::Type::CERTIFICATE:
      return L"Certificate";

    case AuthType::Type::JWT:
      return L"JWT";

    case AuthType::Type::HEADER:
      return L"Header";

    default:
      return L"Password";
  }
}
}  // namespace odbc
}  // namespace timestream
