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

#ifndef _TIMESTREAM_ODBC_AUTHENTICATION_OKTA
#define _TIMESTREAM_ODBC_AUTHENTICATION_OKTA

/*@*/
#include <aws/core/client/ClientConfiguration.h>

#include "timestream/odbc/authentication/saml.h"

namespace timestream {
namespace odbc {

class IGNITE_IMPORT_EXPORT TimestreamOktaCredentialsProvider
    : public TimestreamSAMLCredentialsProvider {
 public:
  /**
   * Constructor.
   *
   * @param config Configuration object reference
   * @param httpClient Shared pointer to httpClient
   * @param stsClient Shared pointer to STSClient
   */
  TimestreamOktaCredentialsProvider(
      const config::Configuration& config,
      std::shared_ptr< Aws::Http::HttpClient > httpClient,
      std::shared_ptr< Aws::STS::STSClient > stsClient)
      : TimestreamSAMLCredentialsProvider(config, httpClient, stsClient) {
    // No-op.
  }

  /**
   * Get SAML asseration which is a base64 encoded SAML authentication response
   * provided by the IdP.
   *
   * @param errInfo Error message when there is a failure
   * @return SAML assertion value.
   */
  virtual std::string GetSAMLAssertion(std::string& errInfo);

 private:
  /**
   * Create a http request to get session token.
   *
   * @return The created http request.
   */
  std::shared_ptr< Aws::Http::HttpRequest > CreateSessionTokenReq();

  /**
   * Decode numberic character reference in SAMLResponse
   *
   * @param html string containing numberic character references.
   * @return The decoded SAML Response string.
   */
  std::string DecodeNumericCharacters(const std::string& htmlString);

  /**
   * Get session token.
   *
   * @param errInfo Error message when there is a failure
   * @return session token in string.
   */
  std::string GetSessionToken(std::string& errInfo);

  /** single number character reference length */
  static const size_t SINGLE_NUM_CHAR_REF_LENGTH;

  /** SAML response pattern, used for regular expression search in SAML
   * assertion */
  static const std::string SAML_RESPONSE_PATTERN;
};

}  // namespace odbc
}  // namespace timestream

#endif  //_TIMESTREAM_ODBC_AUTHENTICATION_OKTA
