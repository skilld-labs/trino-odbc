/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modifications Copyright Amazon.com, Inc. or its affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#define BOOST_TEST_MODULE TimestreamTest
#ifdef _WIN32
#include <Windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <string>
#include <thread>

#include "odbc_test_suite.h"
#include "test_utils.h"
#include "timestream/odbc/utility.h"
#include <ignite/common/include/common/platform_utils.h>

#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/bind.hpp>

using boost::unit_test::precondition;
using timestream::odbc::AuthType;
using timestream::odbc::OdbcTestSuite;
using timestream::odbc::utility::CheckEnvVarSetToTrue;
using timestream_test::GetOdbcErrorMessage;
using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct ConnectionTestSuiteFixture : OdbcTestSuite {
  using OdbcTestSuite::OdbcTestSuite;

  void connect() {
    std::string connectionString;
    CreateDsnConnectionStringForAWS(connectionString);
    Connect(connectionString);
    Disconnect();
  }

  void connectForMultiThread(bool& result) {
    std::string connectionString;
    CreateDsnConnectionStringForAWS(connectionString);
    Connect(connectionString, 3, result);
    Disconnect();
  }
};

BOOST_FIXTURE_TEST_SUITE(ConnectionTestSuite, ConnectionTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestSQLConnection) {
  std::string dsn = "TestConnectionDSN";
  std::string connectionString;
  CreateDsnConnectionStringForAWS(connectionString);

  std::string username;
  std::string password;
  WriteDsnConfiguration(dsn, connectionString, username, password);
  Connect(dsn, username, password);

  Disconnect();

  DeleteDsnConfiguration(dsn);
}

BOOST_AUTO_TEST_CASE(TestDriverConnection) {
  std::string connectionString;
  CreateDsnConnectionStringForAWS(connectionString);

  Connect(connectionString);

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionWithEndpoint) {
  std::string connectionString;
  std::string misc(
      "EndpointOverride=query.timestream.us-west-2.amazonaws.com;");

  CreateDsnConnectionStringForAWS(connectionString, "", "", misc);

  Connect(connectionString);

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionWithSpacesEndpoint) {
  std::string connectionString;
  std::string misc("EndpointOverride= \t;");

  CreateDsnConnectionStringForAWS(connectionString, "", "", misc);

  Connect(connectionString);

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingDupCredString) {
  // Test passing both uid/pwd and accessKeyId/secretKey in the connection
  // string
  std::string dsn = "TestConnectionDSN";
  std::string connectionString;
  std::string accessKeyId;
  std::string secretKey;
  GetIAMCredentials(accessKeyId, secretKey);

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM,
                                   accessKeyId, secretKey, true, accessKeyId,
                                   secretKey);

  std::string username;
  std::string password;
  WriteDsnConfiguration(dsn, connectionString, username, password);
  Connect(dsn, username, password);

  Disconnect();

  DeleteDsnConfiguration(dsn);
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionUsingDupCredString) {
  // Test passing both uid/pwd and accessKeyId/secretKey with correct
  // credentials in the connection string

  std::string connectionString;
  std::string accessKeyId;
  std::string secretKey;
  GetIAMCredentials(accessKeyId, secretKey);

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM,
                                   accessKeyId, secretKey, true, accessKeyId,
                                   secretKey);

  Connect(connectionString);

  Disconnect();
}

BOOST_AUTO_TEST_CASE(
    TestDriverConnectionUsingDupCredStringWithWrongIAMCredentials) {
  // Test passing uid/pwd with correct credentials and accessKeyId/secretKey
  // with wrong credentials in the connection string. Since uid/pwd take
  // precendence in making a connection, the connection should succeed

  std::string connectionString;
  std::string accessKeyId = "wrongAccessKeyId";
  std::string secretKey = "wrongSecretKey";
  std::string uid;
  std::string pwd;
  GetIAMCredentials(uid, pwd);

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM, uid,
                                   pwd, true, accessKeyId, secretKey);

  Connect(connectionString);

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionUsingDupCredStringWithEmptyUidPwd) {
  // Test passing uid/pwd with empty credentials and accessKeyId/secretKey
  // with correct credentials in the connection string. Since
  // accessKeyId/secretKey should be used if uid/pwd are empty, the connection
  // should succeed

  std::string connectionString;
  std::string uid = "";
  std::string pwd = "";
  std::string accessKeyId;
  std::string secretKey;
  GetIAMCredentials(accessKeyId, secretKey);

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM, uid,
                                   pwd, true, accessKeyId, secretKey);

  Connect(connectionString);

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionUsingDupCredStringWithWrongUidPwd) {
  // Test passing uid/pwd with wrong credentials and accessKeyId/secretKey
  // with correct credentials in the connection string. Since uid/pwd take
  // precendence in making a connection, the connection should fail

  std::string connectionString;
  std::string uid = "wrongUsername";
  std::string pwd = "wrongPassword";
  std::string accessKeyId;
  std::string secretKey;
  GetIAMCredentials(accessKeyId, secretKey);

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM, uid,
                                   pwd, true, accessKeyId, secretKey);

  ExpectConnectionReject(
      connectionString, "08001",
      "Failed to establish connection to Timestream.\nINVALID_ENDPOINT: "
      "Failed to discover endpoint");

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionUsingDupCredStringWithWrongUid) {
  // Test passing uid with wrong value, pwd with correct value, and
  // accessKeyId/secretKey with correct credentials in the connection string.
  // Since uid/pwd take precendence in making a connection, the connection
  // should fail

  std::string connectionString;
  std::string uid = "wrongUsername";
  std::string pwd = GetEnv("AWS_SECRET_ACCESS_KEY");
  std::string accessKeyId;
  std::string secretKey;
  GetIAMCredentials(accessKeyId, secretKey);

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM, uid,
                                   pwd, true, accessKeyId, secretKey);

  ExpectConnectionReject(
      connectionString, "08001",
      "Failed to establish connection to Timestream.\nINVALID_ENDPOINT: "
      "Failed to discover endpoint");

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionUsingDupCredStringWithWrongPwd) {
  // Test passing uid with correct value, pwd with wrong value, and
  // accessKeyId/secretKey with correct credentials in the connection string.
  // Since uid/pwd take precendence in making a connection, the connection
  // should fail

  std::string connectionString;
  std::string uid = GetEnv("AWS_ACCESS_KEY_ID");
  std::string pwd = "wrongPassword";
  std::string accessKeyId;
  std::string secretKey;
  GetIAMCredentials(accessKeyId, secretKey);

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM, uid,
                                   pwd, true, accessKeyId, secretKey);

  ExpectConnectionReject(
      connectionString, "08001",
      "Failed to establish connection to Timestream.\nINVALID_ENDPOINT: "
      "Failed to discover endpoint");

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAAD) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given all correct configuration parameters
    // which are from environment variables by default.
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString);

    Connect(connectionString);

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionGenericConnectionStringUsingAAD) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given all correct configuration parameters
    // which are from environment variables by default. uid/pwd are used in the
    // connection string
    std::string dsn = "TestConnectionDSNAAD";
    std::string connectionString;

    std::string appId = GetEnv("AAD_APP_ID");
    std::string roleArn = GetEnv("AAD_ROLE_ARN");
    std::string idpArn = GetEnv("AAD_IDP_ARN");
    std::string tenantId = GetEnv("AAD_TENANT");
    std::string uid = GetEnv("AAD_USER");
    std::string pwd = GetEnv("AAD_USER_PWD");
    std::string clientSecret = GetEnv("AAD_CLIENT_SECRET");

    std::string logPath = GetEnv("TIMESTREAM_LOG_PATH", "");
    std::string logLevel = GetEnv("TIMESTREAM_LOG_LEVEL", "2");
    std::string region = GetEnv("AWS_REGION", "us-west-2");

    connectionString =
      "driver={Amazon Timestream ODBC Driver};"
      "dsn={" + dsn + "};"
      "auth=" + AuthType::ToString(AuthType::Type::AAD) + ";"
      "region=" + region + ";"
      "logOutput=" + logPath + ";"
      "logLevel=" + logLevel + ";";

    std::string tsAuthentication = 
      "uid=" + uid + ";"
      "pwd=" + pwd + ";"
      "aadApplicationID=" + appId + ";"
      "aadClientSecret=" + clientSecret + ";"
      "aadTenant=" + tenantId + ";"
      "roleARN=" + roleArn + ";"
      "idPARN=" + idpArn + ";";

    connectionString.append(tsAuthentication);

    Connect(connectionString);

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADInvalidUser) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given invalid username (uid)
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, "invalid-user");

    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\n"
        "Request to Azure Active Directory for access token failed.");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADEmptyUser) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given empty username (uid)
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, "");

    ExpectConnectionReject(
        connectionString, "01S00",
        "The following is required to connect:\n"
        "AUTH is \"AAD\" and "
        "UID or IdpUserName, PWD or IdpPassword, and "
        "AADAppId, RoleArn, IdpArn, AADTenant and AADClientSecret");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADInvalidPassword) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given invalid password (pwd)
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, "invalid-password");

    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\n"
        "Request to Azure Active Directory for access token failed.");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADEmptyPassword) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given empty password (pwd)
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, "");

    ExpectConnectionReject(
        connectionString, "01S00",
        "The following is required to connect:\n"
        "AUTH is \"AAD\" and "
        "UID or IdpUserName, PWD or IdpPassword, and "
        "AADAppId, RoleArn, IdpArn, AADTenant and AADClientSecret");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADInvalidAPPId) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given invalid application id
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, nullptr,
                                 "invalid-application-id");

    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\n"
        "Request to Azure Active Directory for access token failed.");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADEmptyAppId) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given empty application id
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, nullptr, "");

    ExpectConnectionReject(
        connectionString, "01S00",
        "The following is required to connect:\n"
        "AUTH is \"AAD\" and "
        "UID or IdpUserName, PWD or IdpPassword, and "
        "AADAppId, RoleArn, IdpArn, AADTenant and AADClientSecret");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADInvalidTenant) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given invalid tenant id
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                 "invalid_tenant_id");

    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\n"
        "Request to Azure Active Directory for access token failed.");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADEmptyTenant) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given empty tenant id
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                 "");

    ExpectConnectionReject(
        connectionString, "01S00",
        "The following is required to connect:\n"
        "AUTH is \"AAD\" and "
        "UID or IdpUserName, PWD or IdpPassword, and "
        "AADAppId, RoleArn, IdpArn, AADTenant and AADClientSecret");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADInvalidClientSecret) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given invalid client secret
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                 nullptr, "invalid-client-secret");

    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\n"
        "Request to Azure Active Directory for access token failed.");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADEmptyClientSecret) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given empty client secret
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                 nullptr, "");

    ExpectConnectionReject(
        connectionString, "01S00",
        "The following is required to connect:\n"
        "AUTH is \"AAD\" and "
        "UID or IdpUserName, PWD or IdpPassword, and "
        "AADAppId, RoleArn, IdpArn, AADTenant and AADClientSecret");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADInvalidRoleArn) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given invalid role arn
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                 nullptr, nullptr, "invalid-role-arn");

    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\n"
        "Failed to fetch credentials, ERROR: ValidationError: 1 validation "
        "error detected: Value 'invalid-role-arn' at 'roleArn' failed to "
        "satisfy constraint: Member must have length greater than or equal to "
        "20");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADEmptyRoleArn) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given empty role arn
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                 nullptr, nullptr, "");

    ExpectConnectionReject(
        connectionString, "01S00",
        "The following is required to connect:\n"
        "AUTH is \"AAD\" and "
        "UID or IdpUserName, PWD or IdpPassword, and "
        "AADAppId, RoleArn, IdpArn, AADTenant and AADClientSecret");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADInvalidIdpArn) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given invalid idp arn
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                 nullptr, nullptr, nullptr, "invalid-idp-arn");

    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\n"
        "Failed to fetch credentials, ERROR: ValidationError: 1 validation "
        "error detected: Value 'invalid-idp-arn' at 'principalArn' failed to "
        "satisfy constraint: Member must have length greater than or equal to "
        "20");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingAADEmptyIdpArn) {
  if (CheckEnvVarSetToTrue("ENABLE_AAD_TEST")) {
    // Test AAD authentication given empty idp arn
    std::string connectionString;

    CreateAADDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                 nullptr, nullptr, nullptr, "");

    ExpectConnectionReject(
        connectionString, "01S00",
        "The following is required to connect:\n"
        "AUTH is \"AAD\" and "
        "UID or IdpUserName, PWD or IdpPassword, and "
        "AADAppId, RoleArn, IdpArn, AADTenant and AADClientSecret");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << " is skipped due to no valid Azure AD account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOkta) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication given all correct configuration parameters
    // which are from environment variables by default.
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString);

    Connect(connectionString);

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaUidPwd) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication given all correct configuration parameters
    // which are from environment variables by default.
    // Using Uid/Pwd instead of idPUsername/idPPassword
    std::string  connectionString =
      "driver={Amazon Timestream ODBC Driver};"
      "dsn={" + Configuration::DefaultValue::dsn + "};"
      "auth=" + AuthType::ToString(AuthType::Type::OKTA) + ";"
      "region=" + GetEnv("AWS_REGION", "us-west-2") + ";"
      "logOutput=" + GetEnv("TIMESTREAM_LOG_PATH", "") + ";"
      "logLevel=" + GetEnv("TIMESTREAM_LOG_LEVEL", "2") + ";";

    std::string tsAuthentication = 
      "idPHost=" + GetEnv("OKTA_HOST") + ";" +
      "Uid=" + GetEnv("OKTA_USER") + ";"
      "Pwd=" + GetEnv("OKTA_USER_PWD") + ";"
      "OktaApplicationID=" +  GetEnv("OKTA_APP_ID") + ";"
      "roleARN=" +  GetEnv("OKTA_ROLE_ARN") + ";"
      "idPARN=" + GetEnv("OKTA_IDP_ARN") + ";";

    connectionString.append(tsAuthentication);
    Connect(connectionString);

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaInvalidHost) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with invalid host
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, "invalid_host");

#ifdef _WIN32
    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\n"
        "Failed to get Okta session token. Error info: 'Encountered network "
        "error when sending http request'");
#else
    ExpectConnectionReject(connectionString, "08001",
                           "Failed to establish connection to Timestream.\n"
                           "Failed to get Okta session token. Error info: "
                           "'curlCode: 6, Couldn't resolve host name'");
#endif

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaEmptyHost) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with empty host
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, "");

    ExpectConnectionReject(connectionString, "01S00",
                           "The following is required to connect:\n"
                           "AUTH is \"OKTA\" and "
                           "IdpHost, UID or IdpUserName, PWD or IdpPassword, "
                           "OktaAppId, RoleArn and IdpArn");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaInvalidUser) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with invalid user
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, nullptr, "invalid_user");

    ExpectConnectionReject(connectionString, "08001",
                           "Failed to establish connection to Timestream.\n"
                           "Failed to get Okta session token.");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaEmptyUser) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with empty user
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, nullptr, "");

    ExpectConnectionReject(connectionString, "01S00",
                           "The following is required to connect:\n"
                           "AUTH is \"OKTA\" and "
                           "IdpHost, UID or IdpUserName, PWD or IdpPassword, "
                           "OktaAppId, RoleArn and IdpArn");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaInvalidPasswd) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with invalid password
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, nullptr, nullptr,
                                  "invalid_password");

    ExpectConnectionReject(connectionString, "08001",
                           "Failed to establish connection to Timestream.\n"
                           "Failed to get Okta session token.");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaEmptyPassword) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with empty password
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, nullptr, nullptr, "");

    ExpectConnectionReject(connectionString, "01S00",
                           "The following is required to connect:\n"
                           "AUTH is \"OKTA\" and "
                           "IdpHost, UID or IdpUserName, PWD or IdpPassword, "
                           "OktaAppId, RoleArn and IdpArn");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaInvalidAPPId) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with invalid application id
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                  "invalid_app_id");

    ExpectConnectionReject(connectionString, "08001",
                           "Failed to establish connection to Timestream.\n"
                           "Failed to get SAML asseration.");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaEmptyAppId) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with empty application id
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                  "");

    ExpectConnectionReject(connectionString, "01S00",
                           "The following is required to connect:\n"
                           "AUTH is \"OKTA\" and "
                           "IdpHost, UID or IdpUserName, PWD or IdpPassword, "
                           "OktaAppId, RoleArn and IdpArn");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaInvalidRoleArn) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with invalid role arn
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                  nullptr, "invalid_role_arn");

    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\n"
        "Failed to fetch credentials, ERROR: ValidationError: 1 validation "
        "error detected"
        ": Value 'invalid_role_arn' at 'roleArn' failed to satisfy constraint"
        ": Member must have length greater than or equal to 20");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaEmptyRoleArn) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with empty role arn
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                  nullptr, "");

    ExpectConnectionReject(connectionString, "01S00",
                           "The following is required to connect:\n"
                           "AUTH is \"OKTA\" and "
                           "IdpHost, UID or IdpUserName, PWD or IdpPassword, "
                           "OktaAppId, RoleArn and IdpArn");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaInvalidIdpArn) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with invalid IDP arn
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                  nullptr, nullptr, "invalid_idp_arn");

    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\nFailed to fetch "
        "credentials, "
        "ERROR: ValidationError: 1 validation error detected"
        ": Value 'invalid_idp_arn' at 'principalArn' failed to satisfy "
        "constraint"
        ": Member must have length greater than or equal to 20");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingOktaEmptyIdpArn) {
  if (CheckEnvVarSetToTrue("ENABLE_OKTA_TEST")) {
    // Test Okta authentication with empty IDP arn
    std::string connectionString;
    CreateOktaDsnConnectionString(connectionString, nullptr, nullptr, nullptr,
                                  nullptr, nullptr, "");

    ExpectConnectionReject(connectionString, "01S00",
                           "The following is required to connect:\n"
                           "AUTH is \"OKTA\" and "
                           "IdpHost, UID or IdpUserName, PWD or IdpPassword, "
                           "OktaAppId, RoleArn and IdpArn");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid Okta account" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionUsingGenericIAMString) {
  // Test passing only uid/pwd in the connection string
  std::string dsn = "TestConnectionDSN";
  std::string connectionString;
  std::string uid;
  std::string pwd;
  GetIAMCredentials(uid, pwd);

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM, uid,
                                   pwd);

  std::string username;
  std::string password;
  WriteDsnConfiguration(dsn, connectionString, username, password);
  Connect(dsn, username, password);

  Disconnect();

  DeleteDsnConfiguration(dsn);
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionUsingGenericIAMString) {
  // Test passing only uid/pwd in the connection string
  std::string connectionString;
  std::string uid;
  std::string pwd;
  GetIAMCredentials(uid, pwd);

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM, uid,
                                   pwd);

  Connect(connectionString);

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionWithUIDSecretKey) {
  std::string connectionString;
  std::string accessKeyId = GetEnv("AWS_ACCESS_KEY_ID");
  std::string secretKey = GetEnv("AWS_SECRET_ACCESS_KEY");
  std::string sessionToken = GetEnv("AWS_SESSION_TOKEN");
  std::string region = GetEnv("AWS_REGION", "us-west-2");
  std::string logPath = GetEnv("TIMESTREAM_LOG_PATH");
  std::string logLevel = GetEnv("TIMESTREAM_LOG_LEVEL", "2");

  connectionString =
            "driver={Amazon Timestream ODBC Driver};"
            "dsn={" + Configuration::DefaultValue::dsn + "};"
            "auth=" + AuthType::ToString(AuthType::Type::IAM) + ";"
            "secretKey=" + secretKey + ";"
            "uid=" + accessKeyId + ";"
            "sessionToken=" + sessionToken + ";"
            "region=" + region + ";"
            "logOutput=" + logPath + ";"
            "logLevel=" + logLevel + ";";

  Connect(connectionString);

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionWithAccessKeyIdPWD) {
  std::string connectionString;
  std::string accessKeyId = GetEnv("AWS_ACCESS_KEY_ID");
  std::string secretKey = GetEnv("AWS_SECRET_ACCESS_KEY");
  std::string sessionToken = GetEnv("AWS_SESSION_TOKEN");
  std::string region = GetEnv("AWS_REGION", "us-west-2");
  std::string logPath = GetEnv("TIMESTREAM_LOG_PATH");
  std::string logLevel = GetEnv("TIMESTREAM_LOG_LEVEL", "2");

  connectionString =
            "driver={Amazon Timestream ODBC Driver};"
            "dsn={" + Configuration::DefaultValue::dsn + "};"
            "auth=" + AuthType::ToString(AuthType::Type::IAM) + ";"
            "pwd=" + secretKey + ";"
            "accessKeyId=" + accessKeyId + ";"
            "sessionToken=" + sessionToken + ";"
            "region=" + region + ";"
            "logOutput=" + logPath + ";"
            "logLevel=" + logLevel + ";";

  Connect(connectionString);

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionUsingProfile) {
  if (CheckEnvVarSetToTrue("ENABLE_PROFILE_TEST")) {
    const std::string profile = "test-profile";
    std::string connectionString;
    CreateDsnConnectionStringForAWS(connectionString,
                                    AuthType::Type::AWS_PROFILE, profile);
    Connect(connectionString);
    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid IAM test profile" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestConnectionUsingIncompleteProfile) {
  if (CheckEnvVarSetToTrue("ENABLE_PROFILE_TEST")) {
    const std::string profile = "incomplete-profile";

    std::string connectionString;
    CreateDsnConnectionStringForAWS(connectionString,
                                    AuthType::Type::AWS_PROFILE, profile);

    ExpectConnectionReject(
        connectionString, "08001",
        "Failed to establish connection to Timestream.\nINVALID_ENDPOINT: "
        "Failed to discover endpoint");

    Disconnect();
  } else {
    std::cout << boost::unit_test::framework::current_test_case().p_name
              << "  is skipped due to no valid IAM test profile" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(TestConnectionUsingNonExistProfile) {
  const std::string profile = "nonexist-profile";
  std::string connectionString;
  CreateDsnConnectionStringForAWS(connectionString, AuthType::Type::AWS_PROFILE,
                                  profile);

  ExpectConnectionReject(connectionString, "08001",
                         "Failed to establish connection to Timestream.\n"
                         "Empty or expired credentials");

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionUsingEmptyProfile) {
  const std::string profile = "";
  std::string connectionString;
  CreateDsnConnectionStringForAWS(connectionString, AuthType::Type::AWS_PROFILE,
                                  profile);

  ExpectConnectionReject(connectionString, "08001",
                         "Failed to establish connection to Timestream.\n"
                         "Empty or expired credentials");
  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionConcurrency) {
  ConnectionTestSuiteFixture testConn[10];
  bool result[10] = {false};

  std::vector< std::thread > threads;

  for (int i = 0; i < 10; ++i)
    threads.push_back(
        std::thread(&ConnectionTestSuiteFixture::connectForMultiThread,
                    testConn[i], std::ref(result[i])));

  for (auto& th : threads)
    th.join();

  // verify the thread execution result
  // boost macro could be used as multi-thread execution is finished
  for (int i = 0; i < 10; i++) {
    BOOST_REQUIRE_EQUAL(result[i], true);
  }
}

BOOST_AUTO_TEST_CASE(TestConnectionOnlyConnect) {
  std::string connectionString;
  CreateDsnConnectionStringForAWS(connectionString);
  Connect(connectionString);
}

BOOST_AUTO_TEST_CASE(TestConnectionOnlyDisconnect) {
  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionIncompleteBasicProperties) {
  const std::string dsn = "IncompleteBasicProperties";
  std::string connectionString =
      "driver={Amazon Timestream ODBC Driver};"
      "auth=IAM;"
      "accessKeyId=key;";

  std::string username;
  std::string password;
  WriteDsnConfiguration(dsn, connectionString, username, password);
  ExpectConnectionReject(dsn, username, password, "01S00",
                         "The following is required to connect:\n"
                         "AUTH is \"IAM\" and "
                         "UID and PWD or "
                         "AccessKeyId and Secretkey");

  Disconnect();

  DeleteDsnConfiguration(dsn);
}

BOOST_AUTO_TEST_CASE(TestSQLDriverConnectionIncompleteBasicProperties) {
  std::string connectionString =
      "driver={Amazon Timestream ODBC Driver};"
      "auth=IAM;"
      "accessKeyId=key;";
  ExpectConnectionReject(connectionString, "01S00",
                         "The following is required to connect:\n"
                         "AUTH is \"IAM\" and "
                         "UID and PWD or "
                         "AccessKeyId and Secretkey");

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionInvalidUser) {
  const std::string dsn = "InvalidUser";
  std::string connectionString;
  CreateDsnConnectionStringForAWS(connectionString, "", "invaliduser");

  std::string username;
  std::string password;
  WriteDsnConfiguration(dsn, connectionString, username, password);
  ExpectConnectionReject(dsn, username, password, "08001",
                         "Failed to establish connection to "
                         "Timestream.\nINVALID_ENDPOINT: Failed to discover");

  Disconnect();

  DeleteDsnConfiguration(dsn);
}

BOOST_AUTO_TEST_CASE(TestSQLDriverConnectionInvalidUser) {
  std::string connectionString;
  CreateDsnConnectionStringForAWS(connectionString, "", "invaliduser");

  ExpectConnectionReject(connectionString, "08001",
                         "Failed to establish connection to "
                         "Timestream.\nINVALID_ENDPOINT: Failed to discover");

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestSQLConnectionInvalidUserUsingGenericIAMString) {
  // Test passing only uid/pwd in the connection string
  const std::string dsn = "InvalidUser";
  std::string connectionString;
  std::string uid = GetEnv("AWS_ACCESS_KEY_ID");

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM, uid,
                                   "invaliduser");

  std::string username;
  std::string password;
  WriteDsnConfiguration(dsn, connectionString, username, password);
  ExpectConnectionReject(dsn, username, password, "08001",
                         "Failed to establish connection to "
                         "Timestream.\nINVALID_ENDPOINT: Failed to discover");

  Disconnect();

  DeleteDsnConfiguration(dsn);
}

BOOST_AUTO_TEST_CASE(TestSQLDriverConnectionInvalidUserUsingGenericIAMString) {
  // Test passing only uid/pwd in the connection string
  std::string connectionString;
  std::string uid = GetEnv("AWS_ACCESS_KEY_ID");

  CreateGenericDsnConnectionString(connectionString, AuthType::Type::IAM, uid,
                                   "invaliduser");

  ExpectConnectionReject(connectionString, "08001",
                         "Failed to establish connection to "
                         "Timestream.\nINVALID_ENDPOINT: Failed to discover");

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestDriverConnectionMiscAttributes) {
  std::string connectionString;
  std::string misc(
      "RequestTimeout=10000;ConnectionTimeout=10000;MaxRetryCountClient=5;"
      "MaxConnections=25");
  CreateDsnConnectionStringForAWS(connectionString, "", "", misc);

  Connect(connectionString);

  Disconnect();
}

BOOST_AUTO_TEST_SUITE_END()
