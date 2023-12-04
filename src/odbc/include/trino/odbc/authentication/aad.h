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

#ifndef _TRINO_ODBC_AUTHENTICATION_AAD
#define _TRINO_ODBC_AUTHENTICATION_AAD

#include "trino/odbc/authentication/saml.h"

namespace trino {
namespace odbc {

class IGNITE_IMPORT_EXPORT TrinoAADCredentialsProvider
    : public TrinoSAMLCredentialsProvider {
 public:
  /**
   * Constructor.
   *
   * @param config Configuration object reference
   * @param httpClient Shared pointer to httpClient
   * @param stsClient Shared pointer to STSClient
   */
  TrinoAADCredentialsProvider(
      const config::Configuration& config,
      std::shared_ptr< Aws::Http::HttpClient > httpClient,
      std::shared_ptr< Aws::STS::STSClient > stsClient)
      : TrinoSAMLCredentialsProvider(config, httpClient, stsClient) {
    // No-op.
  }

 protected:
  /*
   * Get SAML assertion response. Called by SAMLCredentialsProvider
   * @param errInfo Error message when there is a failure
   */
  virtual std::string GetSAMLAssertion(std::string& errInfo);

 private:
  /*
   * Get access token using values from config. Called by GetSAMLAssertion
   * Empty string is returned if unable to obtain access token
   * @param errInfo Error message when there is a failure
   */
  std::string GetAccessToken(std::string& errInfo);
};

}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_AUTHENTICATION_AAD
