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

#include <mock/mock_httpclient.h>
#include <aws/core/http/standard/StandardHttpResponse.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <regex>

using namespace Aws::Http::Standard;

namespace timestream {
namespace odbc {

std::shared_ptr< HttpResponse > MockHttpClient::MakeRequest(
    const std::shared_ptr< HttpRequest >& request,
    Aws::Utils::RateLimits::RateLimiterInterface* readLimiter,
    Aws::Utils::RateLimits::RateLimiterInterface* writeLimiter) const {
  std::shared_ptr< HttpResponse > response =
      std::make_shared< StandardHttpResponse >(request);

  Aws::String path = request->GetUri().GetPath();
  std::smatch matches;

  // handle different request based on uri path
  if (path == "/api/v1/authn") {
    HandleSessionTokenRequest(request, response);
  } else if (std::regex_search(path, matches, std::regex("/sso/saml"))) {
    HandleSAMLAssertion(path, request, response);
  } else if (path == "/aad_valid_tenant/oauth2/token") {
    HandleAADAccessTokenRequest(request, response);
  } else if (path == "/aad_invalid_tenant/oauth2/token") {
    response->SetResponseCode(Aws::Http::HttpResponseCode::UNAUTHORIZED);
  }

  return response;
}

void MockHttpClient::HandleSessionTokenRequest(
    const std::shared_ptr< HttpRequest >& request,
    std::shared_ptr< HttpResponse >& response) const {
  std::string body(
      std::istreambuf_iterator< char >(*(request->GetContentBody())), {});

  std::smatch matches;
  std::string user("");
  if (std::regex_search(
          body, matches,
          std::regex(std::string(R"#("username":\t"(.*?)",)#")))) {
    user = matches.str(1);
  }

  // The response is determined by username
  if (user == "okta_valid_user") {
    // valid user
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);

    response->GetResponseBody() << "{\"sessionToken\" : \"1234567890abcdefg\"}";
  } else if (user == "okta_fail_session_token") {
    // fail to get session token
    response->SetResponseCode(Aws::Http::HttpResponseCode::UNAUTHORIZED);

    response->SetClientErrorType(
        Aws::Client::CoreErrors::INVALID_ACCESS_KEY_ID);
    response->SetClientErrorMessage("Invalid access key id");
  } else if (user == "okta_invalid_rsp_body") {
    // session token response body is broken
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);

    response->GetResponseBody() << "{ \"sessionToken\"";
  } else if (user == "okta_no_session_token") {
    // no session token in the response body
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);

    response->GetResponseBody()
        << "{\"notSessionToken\" : \"1234567890abcdefg\"}";
  } else if (user == "okta_empty_session_token") {
    // empty session token in the response body
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);

    response->GetResponseBody() << "{\"sessionToken\" : \"\"}";
  } else {
    // unhandled case
    response->SetResponseCode(Aws::Http::HttpResponseCode::REQUEST_NOT_MADE);
  }
}

void MockHttpClient::HandleAADAccessTokenRequest(
    const std::shared_ptr< HttpRequest >& request,
    std::shared_ptr< HttpResponse >& response) const {
  std::string body(
      std::istreambuf_iterator< char >(*(request->GetContentBody())), {});

  std::smatch matches;
  std::string user("");
  if (std::regex_search(body, matches,
                        std::regex(std::string("username=\\s*(\\w*)")))) {
    user = matches.str(1);
  }

  // The response is determined by username
  if (user == "aad_valid_user") {
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);

    response->GetResponseBody()
        << "{\"access_token\" : \"correctAccessToken\"}";
  } else if (user == "aad_wrong_access_token") {
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);

    response->GetResponseBody() << "{\"access_token\" : "
                                   "\"abcdefghijklmn123456890\"}";
  } else if (user == "aad_fail_access_token") {
    response->SetResponseCode(Aws::Http::HttpResponseCode::BAD_REQUEST);
  } else if (user == "aad_client_error") {
    response->SetResponseCode(Aws::Http::HttpResponseCode::UNAUTHORIZED);

    response->SetClientErrorType(Aws::Client::CoreErrors::NETWORK_CONNECTION);
    response->SetClientErrorMessage("Network connection error");
  } else if (user == "aad_invalid_rsp_body") {
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);

    response->GetResponseBody() << "{ \"access_token\"";
  } else if (user == "aad_no_access_token") {
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);

    response->GetResponseBody()
        << "{\"no_access_token\" : \"1234567890abcdefg\"}";
  } else if (user == "aad_empty_access_token") {
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);

    response->GetResponseBody() << "{\"access_token\" : \"\"}";
  }
}

void MockHttpClient::HandleSAMLAssertion(
    const Aws::String& path, const std::shared_ptr< HttpRequest >& request,
    std::shared_ptr< HttpResponse >& response) const {
  std::smatch matches;

  // The response is determined by application id in the path.
  if (std::regex_search(path, matches, std::regex("okta_valid_app_id"))) {
    // respond with valid SAMLResponse value
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);

    std::string samlResponse("");
    samlResponse =
        "<input name=\"SAMLResponse\" type=\"hidden\" "
        "value=\"TUw6Mi4wOmF0dHJuYW1lLWZvcm1hdDpiYXNpYyI&#x2b;"
        "aGVtYS1pbnN0YW5jZSIgeHNpOnR5cGU9InhzOnN0cmluZyI&#x2b;\"/>";
    response->GetResponseBody() << samlResponse;
  } else if (std::regex_search(path, matches,
                               std::regex("okta_error_app_id"))) {
    // Respond with failure
    response->SetResponseCode(Aws::Http::HttpResponseCode::BAD_REQUEST);

    response->SetClientErrorType(
        Aws::Client::CoreErrors::INVALID_QUERY_PARAMETER);
    response->SetClientErrorMessage("Invalid query parameter");
  } else if (std::regex_search(path, matches,
                               std::regex("okta_no_saml_response_app_id"))) {
    // No SAMLResponse in the response body
    response->SetResponseCode(Aws::Http::HttpResponseCode::OK);
  }
}
}  // namespace odbc
}  // namespace timestream
