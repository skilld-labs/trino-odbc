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

#include "trino/odbc/log.h"
#include "trino/odbc/authentication/saml.h"

/*#*/
#include <aws/sts/model/Credentials.h>

namespace trino {
namespace odbc {

bool TrinoSAMLCredentialsProvider::FetchCredentialsWithSAMLAssertion(
    Aws::STS::Model::AssumeRoleWithSAMLRequest& samlRequest,
    Aws::Auth::AWSCredentials& awsCredentials, std::string& errInfo) {
  LOG_DEBUG_MSG("FetchCredentialsWithSAMLAssertion is called");

  Aws::STS::Model::AssumeRoleWithSAMLOutcome outcome =
      stsClient_->AssumeRoleWithSAML(samlRequest);

  bool retval = false;
  if (outcome.IsSuccess()) {
    const Aws::STS::Model::Credentials& credentials =
        outcome.GetResult().GetCredentials();

    retval = true;
  } else {
    auto error = outcome.GetError();
    errInfo = "Failed to fetch credentials, ERROR: " + error.GetExceptionName()
              + ": " + error.GetMessage();
    LOG_ERROR_MSG(errInfo);
  }

  return retval;
}

bool TrinoSAMLCredentialsProvider::GetAWSCredentials(
    Aws::Auth::AWSCredentials& credentials, std::string& errInfo) {
  LOG_DEBUG_MSG("GetAWSCredentials is called");

  std::string samlAsseration = GetSAMLAssertion(errInfo);
  bool retval = false;
  if (samlAsseration.empty()) {
    LOG_ERROR_MSG("Failed to get SAML asseration.");
  } else {
    Aws::STS::Model::AssumeRoleWithSAMLRequest samlRequest;
    samlRequest.WithRoleArn(config_.GetRoleArn().c_str())
        .WithSAMLAssertion(samlAsseration)
        .WithPrincipalArn(config_.GetIdPArn().c_str());

    retval =
        FetchCredentialsWithSAMLAssertion(samlRequest, credentials, errInfo);
  }

  return retval;
}

}  // namespace odbc
}  // namespace trino
