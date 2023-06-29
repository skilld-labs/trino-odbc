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

namespace timestream {
namespace odbc {
AuthType::Type AuthType::FromString(const std::string& val, Type dflt) {
  std::string lowerVal = utility::Trim(timestream::odbc::common::ToLower(val));

  if (lowerVal == "aws_profile")
    return AuthType::Type::AWS_PROFILE;

  if (lowerVal == "iam")
    return AuthType::Type::IAM;

  if (lowerVal == "aad")
    return AuthType::Type::AAD;

  if (lowerVal == "okta")
    return AuthType::Type::OKTA;

  return dflt;
}

std::string AuthType::ToString(Type val) {
  switch (val) {
    case AuthType::Type::AWS_PROFILE:
      return "aws_profile";

    case AuthType::Type::IAM:
      return "iam";

    case AuthType::Type::AAD:
      return "aad";

    case AuthType::Type::OKTA:
      return "okta";

    default:
      return "aws_profile";
  }
}

std::wstring AuthType::ToCBString(Type val) {
  switch (val) {
    case AuthType::Type::AWS_PROFILE:
      return L"AWS Profile";

    case AuthType::Type::IAM:
      return L"AWS IAM Credentials";

    case AuthType::Type::AAD:
      return L"Identitiy Provider: Azure AD";

    case AuthType::Type::OKTA:
      return L"Identitiy Provider: Okta";

    default:
      return L"AWS Profile";
  }
}
}  // namespace odbc
}  // namespace timestream
