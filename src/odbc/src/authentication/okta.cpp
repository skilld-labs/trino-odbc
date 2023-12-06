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
/*#*/

#include "trino/odbc/log.h"
#include "trino/odbc/authentication/okta.h"
#include "trino/odbc/utility.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/http/HttpClient.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <regex>

namespace trino {
namespace odbc {

const size_t TrinoOktaCredentialsProvider::SINGLE_NUM_CHAR_REF_LENGTH = 6;
const std::string TrinoOktaCredentialsProvider::SAML_RESPONSE_PATTERN =
    std::string(
        "<input name=\"SAMLResponse\" type=\"hidden\" value=\"(.*?)\"/>");

std::shared_ptr< Aws::Http::HttpRequest >
TrinoOktaCredentialsProvider::CreateSessionTokenReq() {
  LOG_DEBUG_MSG("CreateSessionTokenReq is called");

  std::string baseUri = "https://" + config_.GetIdPHost() + "/api/v1/authn";
  LOG_DEBUG_MSG("baseUri is " << baseUri);

  std::shared_ptr< Aws::Http::HttpRequest > req = Aws::Http::CreateHttpRequest(
      baseUri, Aws::Http::HttpMethod::HTTP_POST,
      Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);

  // set header
  req->SetHeaderValue(Aws::Http::ACCEPT_HEADER, "application/json");
  req->SetHeaderValue(Aws::Http::CONTENT_TYPE_HEADER, "application/json");

  // username and password are wrapped in json block
  Aws::Utils::Json::JsonValue idpCredentials;
  idpCredentials.WithString("username", Aws::String(config_.GetDSNUserName()));
  idpCredentials.WithString("password", Aws::String(config_.GetDSNPassword()));

  // set content
  std::string body = idpCredentials.View().WriteReadable();
  std::shared_ptr< Aws::StringStream > ss =
      std::make_shared< Aws::StringStream >();
  *ss << body;

  req->AddContentBody(ss);
  req->SetContentLength(std::to_string(body.size()));

  return req;
}

std::string TrinoOktaCredentialsProvider::GetSessionToken(
    std::string& errInfo) {
  LOG_DEBUG_MSG("GetSessionToken is called");

  std::string sessionToken("");

  // create request
  std::shared_ptr< Aws::Http::HttpRequest > req = CreateSessionTokenReq();

  // check response code
  std::shared_ptr< Aws::Http::HttpResponse > response =
      httpClient_->MakeRequest(req);
  if (response->GetResponseCode() != Aws::Http::HttpResponseCode::OK) {
    errInfo = "Failed to get Okta session token.";
    if (response->HasClientError()) {
      errInfo += " Error info: '" + response->GetClientErrorMessage() + "'.";
    }
    LOG_ERROR_MSG(errInfo);
    return sessionToken;
  }

  // get response body
  Aws::Utils::Json::JsonValue responseBody(response->GetResponseBody());
  if (!responseBody.WasParseSuccessful()) {
    errInfo = "Error parsing response body. " + responseBody.GetErrorMessage();
    LOG_ERROR_MSG(errInfo);
    return sessionToken;
  }

  // extract session token
  Aws::Utils::Json::JsonView bodyJsonView = responseBody.View();
  if (bodyJsonView.ValueExists("sessionToken")) {
    sessionToken = bodyJsonView.GetString("sessionToken");
  } else {
    errInfo = "No session token in the Okta response body";
    LOG_ERROR_MSG(errInfo);
  }

  return sessionToken;
}

std::string TrinoOktaCredentialsProvider::DecodeNumericCharacters(
    const std::string& htmlString) {
  LOG_DEBUG_MSG("DecodeNumericCharacters is called");
  if (htmlString.length() < SINGLE_NUM_CHAR_REF_LENGTH) {
    return htmlString;
  }

  std::string result;
  result.reserve(htmlString.size());
  for (size_t i = 0; i < htmlString.length(); i++) {
    char c;
    // Numeric character reference has 6 characters. It starts with "&#x" and
    // ends with ";".
    if (htmlString.substr(i, 3) == "&#x" && htmlString[i + 5] == ';') {
      // Extract the 2-digit hex code. e.g. "&#x2d;" -> "2d"
      std::string hexCode = htmlString.substr(i + 3, 2);
      int x = utility::StringToInt(hexCode, nullptr, 16);
      c = (char)x;

      i += 5;
    } else {
      c = htmlString[i];
    }
    result += c;
  }

  return result;
}

std::string TrinoOktaCredentialsProvider::GetSAMLAssertion(
    std::string& errInfo) {
  LOG_DEBUG_MSG("GetSAMLAssertion is called");
  std::string samlResponse("");

  // get session token
  std::string sessionToken = GetSessionToken(errInfo);
  if (sessionToken.empty()) {
    LOG_ERROR_MSG("Could not get one time session token for Okta");
    return samlResponse;
  }

  std::string baseUri = "https://" + config_.GetIdPHost() + "/app/amazon_aws/"
                        + config_.GetOktaAppId() + "/sso/saml";

  // create saml request
  std::shared_ptr< Aws::Http::HttpRequest > samlGetRequest =
      Aws::Http::CreateHttpRequest(
          baseUri, Aws::Http::HttpMethod::HTTP_GET,
          Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);
  samlGetRequest->AddQueryStringParameter("onetimetoken", sessionToken);

  std::shared_ptr< Aws::Http::HttpResponse > response =
      httpClient_->MakeRequest(samlGetRequest);

  // check response code
  if (response->GetResponseCode() != Aws::Http::HttpResponseCode::OK) {
    errInfo = "Failed to get SAML asseration.";
    if (response->HasClientError()) {
      errInfo += " Client error: '" + response->GetClientErrorMessage() + "'.";
    }
    LOG_ERROR_MSG(errInfo);
    return samlResponse;
  }

  std::istreambuf_iterator< char > eos;
  std::string body(
      std::istreambuf_iterator< char >(response->GetResponseBody().rdbuf()),
      eos);

  // retrieve SAMLResponse value
  std::smatch matches;
  if (std::regex_search(body, matches, std::regex(SAML_RESPONSE_PATTERN))) {
    samlResponse = DecodeNumericCharacters(matches.str(1));
  } else {
    errInfo = "Could not extract SAMLResponse from the Okta response body";
    LOG_ERROR_MSG(errInfo);
  }

  return samlResponse;
}

}  // namespace odbc
}  // namespace trino
