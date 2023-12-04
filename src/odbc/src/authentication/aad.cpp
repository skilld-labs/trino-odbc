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
#include "trino/odbc/authentication/aad.h"
#include "trino/odbc/diagnostic/diagnosable_adapter.h"

/*#*/
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/base64/Base64.h>
#include <aws/core/http/HttpClient.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/http/standard/StandardHttpRequest.h>
#include <aws/core/http/standard/StandardHttpResponse.h>
#include <aws/core/utils/Array.h>

namespace trino {
namespace odbc {

// Base64URL encoding table
const char BASE64_ENCODING_TABLE_URL[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// Base64URL instance to encode or decode
const Aws::Utils::Base64::Base64 BASE64_URL =
    Aws::Utils::Base64::Base64(BASE64_ENCODING_TABLE_URL);

// The ASCII format for colon
const std::string COLON = "%3A";

std::string TrinoAADCredentialsProvider::GetAccessToken(
    std::string& errInfo) {
  LOG_DEBUG_MSG("GetAccessToken is called");

  using Aws::Http::URI;
  std::string accessToken("");

  // create HTTP request with DSN configuration
  Aws::String accessTokenEndpoint = "https://login.microsoftonline.com/"
                                    + config_.GetAADTenant() + "/oauth2/token";
  LOG_DEBUG_MSG("accessTokenEndpoint is " << accessTokenEndpoint);

  std::shared_ptr< Aws::Http::HttpRequest > req = Aws::Http::CreateHttpRequest(
      accessTokenEndpoint, Aws::Http::HttpMethod::HTTP_POST,
      Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);
  req->SetHeaderValue(Aws::Http::ACCEPT_HEADER, "application/json");
  req->SetHeaderValue(Aws::Http::CONTENT_TYPE_HEADER,
                      "application/x-www-form-urlencoded");

  std::shared_ptr< Aws::StringStream > ss =
      Aws::MakeShared< Aws::StringStream >("");
  *ss << "grant_type=password&requested_token_type=urn" << COLON << "ietf"
      << COLON << "params" << COLON << "oauth" << COLON << "token-type" << COLON
      << "saml2&username=" << URI::URLEncodePath(config_.GetDSNUserName())
      << "&password=" << URI::URLEncodePath(config_.GetDSNPassword())
      << "&client_secret=" << URI::URLEncodePath(config_.GetAADClientSecret())
      << "&client_id=" << URI::URLEncodePath(config_.GetAADAppId())
      << "&resource=" << URI::URLEncodePath(config_.GetAADAppId());

  req->AddContentBody(ss);
  req->SetContentLength(std::to_string(ss.get()->str().size()));

  std::shared_ptr< Aws::Http::HttpResponse > res =
      httpClient_->MakeRequest(req);

  if (res->GetResponseCode() != Aws::Http::HttpResponseCode::OK) {
    // HTTP response code is not 200(OK), proceed to log error
    errInfo = "Request to Azure Active Directory for access token failed.";

    if (res->HasClientError()) {
      errInfo += " Client error: '" + res->GetClientErrorMessage() + "'.";
    }
    LOG_ERROR_MSG(errInfo);

    return accessToken;
  }

  // Parse response body
  Aws::Utils::Json::JsonValue resJson(res->GetResponseBody());

  if (!resJson.WasParseSuccessful()) {
    errInfo = "Error parsing response body: " + resJson.GetErrorMessage();
    LOG_ERROR_MSG(errInfo);
    return accessToken;
  }

  // Extract access token from response body
  const Aws::Utils::Json::JsonView& bodyJsonView = resJson.View();
  if (bodyJsonView.ValueExists("access_token")) {
    accessToken = bodyJsonView.GetString("access_token");
  } else {
    errInfo =
        "Unable to extract the access token from the Azure AD response body.";
    LOG_ERROR_MSG(errInfo);
  }

  return accessToken;
}

std::string TrinoAADCredentialsProvider::GetSAMLAssertion(
    std::string& errInfo) {
  LOG_DEBUG_MSG("GetSAMLAssertion is called");

  std::string accessToken = GetAccessToken(errInfo);

  if (accessToken.empty()) {
    // return empty string to indicate that SAML assertion retrieval has failed
    LOG_ERROR_MSG("Access token is empty");
    return accessToken;
  }

  // Microsoft Azure AD doesn't send tail padding to driver, and
  // the size of the access token may not be a multiple of 4.
  // Since AWS::Utils::Base64 is expecting the size of the encoded is a multiple
  // of 4, the driver needs to pad for the AWS::Utils::Base64 decoder
  int mod = accessToken.size() % 4;
  switch (mod) {
    case 1:
      accessToken += "===";
      break;
    case 2:
      accessToken += "==";
      break;
    case 3:
      accessToken += "=";
      break;
      // No action needed when mod is 0.
  }

  // Decode BASE64 encoded access token into SAML assertion
  const Aws::Utils::ByteBuffer& decodedBuffer = BASE64_URL.Decode(accessToken);
  size_t size = decodedBuffer.GetLength();

  std::string decoded(
      reinterpret_cast< char const* >(decodedBuffer.GetUnderlyingData()), size);

  // Construct SAML response with SAML assertion
  std::string assertion(
      "<samlp:Response "
      "xmlns:samlp=\"urn:oasis:names:tc:SAML:2.0:protocol\"><samlp:Status><"
      "samlp:StatusCode "
      "Value=\"urn:oasis:names:tc:SAML:2.0:status:Success\"/></samlp:Status>"
      + decoded + "</samlp:Response>");

  Aws::Utils::ByteBuffer encodeBuffer(
      reinterpret_cast< unsigned char const* >(assertion.c_str()),
      assertion.size());

  return BASE64_URL.Encode(encodeBuffer);
}

}  // namespace odbc
}  // namespace trino
