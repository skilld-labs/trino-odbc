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

#define BOOST_TEST_MODULE TrinoUnitTest
#include <string>

#include <odbc_unit_test_suite.h>
#include "trino/odbc/log.h"
#include "trino/odbc/log_level.h"
#include <ignite/common/include/common/platform_utils.h>
#include <trino/odbc/authentication/auth_type.h>
#include "mock/mock_trino_service.h"
#include "trino/odbc/log.h"
#include <regex>

using trino::odbc::AuthType;
using trino::odbc::MockConnection;
using trino::odbc::MockTrinoService;
using trino::odbc::OdbcUnitTestSuite;
using trino::odbc::config::Configuration;
using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct ConnectionUnitTestSuiteFixture : OdbcUnitTestSuite {
  ConnectionUnitTestSuiteFixture() : OdbcUnitTestSuite() {
  }

  /**
   * Destructor.
   */
  ~ConnectionUnitTestSuiteFixture() {
  }

  void getLogOptions(Configuration& config) const {
    using ignite::odbc::common::GetEnv;
    using trino::odbc::LogLevel;

    std::string logPath = GetEnv("TRINO_LOG_PATH", "");
    std::string logLevelStr = GetEnv("TRINO_LOG_LEVEL", "2");

    LogLevel::Type logLevel =
        LogLevel::FromString(logLevelStr, LogLevel::Type::UNKNOWN);
    config.SetLogLevel(logLevel);
    config.SetLogPath(logPath);
  }

  bool IsSuccessful() {
    if (!dbc) {
      return false;
    }
    return dbc->GetDiagnosticRecords().IsSuccessful();
  }

  int GetReturnCode() {
    if (!dbc) {
      return SQL_ERROR;
    }
    return dbc->GetDiagnosticRecords().GetReturnCode();
  }

  std::string GetSqlState() {
    if (!dbc) {
      return "";
    }
    return dbc->GetDiagnosticRecords()
        .GetStatusRecord(dbc->GetDiagnosticRecords().GetLastNonRetrieved())
        .GetSqlState();
  }

  void CheckConnectError(Configuration& cfg, const std::string& expectedMsg) {
    std::ostringstream ss;
    std::ostream* original =
        trino::odbc::Logger::GetLoggerInstance()->GetLogStream();
    trino::odbc::Logger::GetLoggerInstance()->SetLogStream(
        static_cast< std::ostream* >(&ss));

    dbc->Establish(cfg);

    trino::odbc::Logger::GetLoggerInstance()->SetLogStream(original);
    std::string logMsg = ss.str();
    std::smatch matches;
    BOOST_REQUIRE_EQUAL(
        std::regex_search(logMsg, matches, std::regex(expectedMsg)), true);
  }
};

BOOST_FIXTURE_TEST_SUITE(ConnectionUnitTestSuite,
                         ConnectionUnitTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestEstablishUsingKey) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::IAM);
  cfg.SetAccessKeyId("AwsTSUnitTestKeyId");
  cfg.SetSecretKey("AwsTSUnitTestSecretKey");
  getLogOptions(cfg);

  dbc->Establish(cfg);

  BOOST_CHECK(IsSuccessful());
}

BOOST_AUTO_TEST_CASE(TestEstablishAuthTypeNotSpecified) {
  trino::odbc::config::Configuration cfg;
  cfg.SetAccessKeyId("AwsTSUnitTestKeyId");
  cfg.SetSecretKey("AwsTSUnitTestSecretKey");

  dbc->Establish(cfg);

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestEstablishUsingKeyNoSecretKey) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::IAM);
  cfg.SetAccessKeyId("AwsTSUnitTestKeyId");
  getLogOptions(cfg);

  dbc->Establish(cfg);

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "01S00");
}

BOOST_AUTO_TEST_CASE(TestEstablishUsingKeyInvalidLogin) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::IAM);
  cfg.SetAccessKeyId("InvalidLogin");
  cfg.SetSecretKey("AwsTSUnitTestSecretKey");
  getLogOptions(cfg);

  dbc->Establish(cfg);

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestEstablishusingKeyInvalidSecretKey) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::IAM);
  cfg.SetAccessKeyId("AwsTSUnitTestKeyId");
  cfg.SetSecretKey("InvalidSecretKey");
  getLogOptions(cfg);

  dbc->Establish(cfg);

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestEstablishReconnect) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::IAM);
  cfg.SetAccessKeyId("AwsTSUnitTestKeyId");
  cfg.SetSecretKey("AwsTSUnitTestSecretKey");
  getLogOptions(cfg);

  dbc->Establish(cfg);

  BOOST_CHECK(IsSuccessful());

  dbc->Establish(cfg);
  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08002");
}

BOOST_AUTO_TEST_CASE(TestRelease) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::IAM);
  cfg.SetAccessKeyId("AwsTSUnitTestKeyId");
  cfg.SetSecretKey("AwsTSUnitTestSecretKey");
  getLogOptions(cfg);

  dbc->Establish(cfg);

  BOOST_CHECK(IsSuccessful());

  dbc->Release();

  BOOST_CHECK(IsSuccessful());
  // release again, error is expected
  dbc->Release();
  BOOST_CHECK_EQUAL(GetSqlState(), "08003");
}

BOOST_AUTO_TEST_CASE(TestDeregister) {
  // This will remove dbc from env, any test that
  // needs env should be put ahead of this testcase
  dbc->Deregister();
  BOOST_CHECK_EQUAL(env->ConnectionsNum(), 0);
}

BOOST_AUTO_TEST_CASE(TestEstablishUsingAAD) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::AAD);
  cfg.SetIdPUserName("aad_valid_user");
  cfg.SetIdPPassword("aad_password");
  cfg.SetAADAppId("aad_valid_app_id");
  cfg.SetAADTenant("aad_valid_tenant");
  cfg.SetAADClientSecret("aad_valid_client_secret");
  cfg.SetRoleArn("arn:role:checkSAMLAssertion");
  cfg.SetIdPArn("arn:idp");

  getLogOptions(cfg);

  dbc->Establish(cfg);

  // verify SQL_USER_NAME is set correctly after connect
  SQLWCHAR userName[16];
  short buflen = 16 * sizeof(SQLWCHAR);
  short reslen;
  dbc->GetInfo().GetInfo(SQL_USER_NAME, userName, buflen, &reslen);

  BOOST_CHECK(trino::odbc::utility::SqlWcharToString(userName)
              == "aad_valid_user");
  BOOST_CHECK(IsSuccessful());
}

BOOST_AUTO_TEST_CASE(TestAADWrongAccessToken) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::AAD);
  cfg.SetIdPUserName("aad_wrong_access_token");
  cfg.SetIdPPassword("aad_password");
  cfg.SetAADAppId("aad_valid_app_id");
  cfg.SetAADTenant("aad_valid_tenant");
  cfg.SetAADClientSecret("aad_valid_client_secret");
  cfg.SetRoleArn("arn:role:checkSAMLAssertion");
  cfg.SetIdPArn("arn:idp");

  getLogOptions(cfg);

  CheckConnectError(cfg, "Failed to fetch credentials");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestAADEmptyAccessToken) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::AAD);
  cfg.SetIdPUserName("aad_empty_access_token");
  cfg.SetIdPPassword("aad_password");
  cfg.SetAADAppId("aad_valid_app_id");
  cfg.SetAADTenant("aad_valid_tenant");
  cfg.SetAADClientSecret("aad_valid_client_secret");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  getLogOptions(cfg);

  CheckConnectError(cfg, "Failed to get SAML asseration");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestAADNoAccessToken) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::AAD);
  cfg.SetIdPUserName("aad_no_access_token");
  cfg.SetIdPPassword("aad_password");
  cfg.SetAADAppId("aad_valid_app_id");
  cfg.SetAADTenant("aad_valid_tenant");
  cfg.SetAADClientSecret("aad_valid_client_secret");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  getLogOptions(cfg);

  CheckConnectError(
      cfg,
      "Unable to extract the access token from the Azure AD response body");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestAADFailAccessToken) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::AAD);
  cfg.SetIdPUserName("aad_fail_access_token");
  cfg.SetIdPPassword("aad_password");
  cfg.SetAADAppId("aad_valid_app_id");
  cfg.SetAADTenant("aad_valid_tenant");
  cfg.SetAADClientSecret("aad_valid_client_secret");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  getLogOptions(cfg);

  CheckConnectError(
      cfg, "Request to Azure Active Directory for access token failed");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestAADInvalidTenant) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::AAD);
  cfg.SetIdPUserName("aad_valid_user");
  cfg.SetIdPPassword("aad_password");
  cfg.SetAADAppId("aad_valid_app_id");
  cfg.SetAADTenant("aad_invalid_tenant");
  cfg.SetAADClientSecret("aad_valid_client_secret");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  getLogOptions(cfg);

  CheckConnectError(
      cfg, "Request to Azure Active Directory for access token failed");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestAADClientError) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::AAD);
  cfg.SetIdPUserName("aad_client_error");
  cfg.SetIdPPassword("aad_password");
  cfg.SetAADAppId("aad_valid_app_id");
  cfg.SetAADTenant("aad_valid_tenant");
  cfg.SetAADClientSecret("aad_valid_client_secret");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  getLogOptions(cfg);

  CheckConnectError(cfg, "Client error: 'Network connection error'.");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestAADSessionTokenInvalidRspBody) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::AAD);
  cfg.SetIdPUserName("aad_invalid_rsp_body");
  cfg.SetIdPPassword("aad_password");
  cfg.SetAADAppId("aad_valid_app_id");
  cfg.SetAADTenant("aad_valid_tenant");
  cfg.SetAADClientSecret("aad_valid_client_secret");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  getLogOptions(cfg);

  CheckConnectError(cfg, "Error parsing response body. Failed to parse JSON.");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestEstablishUsingOkta) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::OKTA);
  cfg.SetIdPHost("okta-host");
  cfg.SetIdPUserName("okta_valid_user");
  cfg.SetIdPPassword("okta_password");
  cfg.SetOktaAppId("okta_valid_app_id");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  getLogOptions(cfg);

  dbc->Establish(cfg);

  // verify SAMLResponse number character references are decoded correctly
  std::string errInfo;
  BOOST_CHECK_EQUAL(
      dbc->GetSAMLCredentialsProvider()->GetSAMLAssertion(errInfo),
      "TUw6Mi4wOmF0dHJuYW1lLWZvcm1hdDpiYXNpYyI+"
      "aGVtYS1pbnN0YW5jZSIgeHNpOnR5cGU9InhzOnN0cmluZyI+");

  // verify SQL_USER_NAME is set correctly after connect
  SQLWCHAR userName[16];
  short buflen = 16 * sizeof(SQLWCHAR);
  short reslen;
  dbc->GetInfo().GetInfo(SQL_USER_NAME, userName, buflen, &reslen);

  BOOST_CHECK(trino::odbc::utility::SqlWcharToString(userName)
              == "okta_valid_user");
  BOOST_CHECK(IsSuccessful());
}

BOOST_AUTO_TEST_CASE(TestOktaFailToGetSessionToken) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::OKTA);
  cfg.SetIdPHost("okta-host");
  cfg.SetIdPUserName("okta_fail_session_token");
  cfg.SetIdPPassword("okta_password");
  cfg.SetOktaAppId("okta_app_id");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  CheckConnectError(
      cfg,
      "Failed to get Okta session token. Error info: 'Invalid access key id'");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestOktaSessionTokenInvalidRspBody) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::OKTA);
  cfg.SetIdPHost("okta-host");
  cfg.SetIdPUserName("okta_invalid_rsp_body");
  cfg.SetIdPPassword("okta_password");
  cfg.SetOktaAppId("okta_app_id");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  CheckConnectError(cfg, "Error parsing response body. Failed to parse JSON.");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestOktaNoSessionToken) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::OKTA);
  cfg.SetIdPHost("okta-host");
  cfg.SetIdPUserName("okta_no_session_token");
  cfg.SetIdPPassword("okta_password");
  cfg.SetOktaAppId("okta_app_id");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  CheckConnectError(cfg, "No session token in the Okta response body");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestOktaEmptySessionToken) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::OKTA);
  cfg.SetIdPHost("okta-host");
  cfg.SetIdPUserName("okta_empty_session_token");
  cfg.SetIdPPassword("okta_password");
  cfg.SetOktaAppId("okta_app_id");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  CheckConnectError(cfg, "Could not get one time session token for Okta");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestOktaErrorAssertion) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::OKTA);
  cfg.SetIdPHost("okta-host");
  cfg.SetIdPUserName("okta_valid_user");
  cfg.SetIdPPassword("okta_password");
  cfg.SetOktaAppId("okta_error_app_id");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  CheckConnectError(cfg,
                    "Failed to get SAML asseration. Client error: 'Invalid "
                    "query parameter'.");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestOktaAssertionNoSAMLRsp) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::OKTA);
  cfg.SetIdPHost("okta-host");
  cfg.SetIdPUserName("okta_valid_user");
  cfg.SetIdPPassword("okta_password");
  cfg.SetOktaAppId("okta_no_saml_response_app_id");
  cfg.SetRoleArn("arn:role");
  cfg.SetIdPArn("arn:idp");

  CheckConnectError(
      cfg, "Could not extract SAMLResponse from the Okta response body");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_CASE(TestOktaSAMLAssertionNoCredentials) {
  Configuration cfg;
  cfg.SetAuthType(AuthType::Type::OKTA);
  cfg.SetIdPHost("okta-host");
  cfg.SetIdPUserName("okta_valid_user");
  cfg.SetIdPPassword("okta_password");
  cfg.SetOktaAppId("okta_valid_app_id");
  cfg.SetRoleArn("arn:role:nocredentials");
  cfg.SetIdPArn("arn:idp");

  CheckConnectError(cfg, "Failed to fetch credentials.");

  BOOST_CHECK_EQUAL(GetReturnCode(), SQL_ERROR);
  BOOST_CHECK_EQUAL(GetSqlState(), "08001");
}

BOOST_AUTO_TEST_SUITE_END()
