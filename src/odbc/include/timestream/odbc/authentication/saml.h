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

#ifndef _TRINO_ODBC_AUTHENTICATION_SAML
#define _TRINO_ODBC_AUTHENTICATION_SAML

#include "trino/odbc/config/configuration.h"

/*@*/
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/sts/model/AssumeRoleWithSAMLRequest.h>
#include <aws/sts/STSClient.h>

namespace trino {
namespace odbc {

class IGNITE_IMPORT_EXPORT TrinoSAMLCredentialsProvider {
 public:
  /**
   * Constructor.
   *
   * @param config Configuration object reference
   * @param httpClient Shared pointer to httpClient
   * @param stsClient Shared pointer to STSClient
   */
  TrinoSAMLCredentialsProvider(
      const config::Configuration& config,
      std::shared_ptr< Aws::Http::HttpClient > httpClient,
      std::shared_ptr< Aws::STS::STSClient > stsClient)
      : config_(config), httpClient_(httpClient), stsClient_(stsClient) {
    // No-op.
  }

  /**
   * Get AWSCredentials.
   *
   * @param credentials Credentials returned from AWS.
   * @param errInfo Error message when there is a failure
   * @return @c true on success and @c false otherwise.
   */
  bool GetAWSCredentials(Aws::Auth::AWSCredentials& credentials,
                         std::string& errInfo);

  /**
   * Get SAML asseration which is a base64 encoded SAML authentication response
   * provided by the IdP
   *
   * @param errInfo Error message when there is a failure
   * @return SAML assertion value.
   */
  virtual std::string GetSAMLAssertion(std::string& errInfo) = 0;

 protected:
  /**
   * Get AWSCredentials based on SAML assertion
   *
   * @param samlRequest AssumeRoleWithSAMLRequest.
   * @param credentials Credentials returned from AWS.
   * @param errInfo Error message when there is a failure
   * @return @c true on success and @c false otherwise.
   */
  bool FetchCredentialsWithSAMLAssertion(
      Aws::STS::Model::AssumeRoleWithSAMLRequest& samlRequest,
      Aws::Auth::AWSCredentials& credentials, std::string& errInfo);

  /** Configuration object */
  config::Configuration config_;

  /** STSClient pointer */
  std::shared_ptr< Aws::STS::STSClient > stsClient_;

  /** HttpClient pointer */
  std::shared_ptr< Aws::Http::HttpClient > httpClient_;
};

}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_AUTHENTICATION_SAML
