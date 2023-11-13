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

#ifdef _WIN32
#include "trino/odbc/system/ui/dsn_configuration_window.h"
#endif

#include <trino/odbc/utils.h>
#include <trino/odbc/config/configuration.h>
#include <trino/odbc/config/connection_string_parser.h>
#include <trino/odbc/authentication/auth_type.h>
#include <trino/odbc/log.h>
#include <trino/odbc/log_level.h>
#include <ignite/odbc/odbc_error.h>

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <set>

#include "trino/odbc/diagnostic/diagnostic_record_storage.h"

using namespace trino::odbc;
using namespace trino::odbc::config;
using trino::odbc::common::EncodeURIComponent;
using namespace boost::unit_test;

namespace {
const std::string testDriverName = "Test Driver";
const std::string testDsn = "Test DSN";
const std::string testUid = "testUid";
const std::string testPwd = "testPwd";
const std::string testAccessKeyId = "testAccessKeyId";
const std::string testSecretKey = "testSecretKey";
const std::string testSessionToken = "testSessionToken";
const AuthType::Type testAuthType = AuthType::FromString("okta");
const std::string testProfileName = "test-profile";
const int32_t testReqTimeoutMS = 300;
const int32_t testConnectionTimeoutMS = 500;
const int32_t testMaxRetryCountClient = 3;
const int32_t testMaxConnections = 15;
const std::string testEndpoint = "testEndpoint";
const std::string testRegion = "testRegion";
const std::string testIdPHost = "testIdPHost";
const std::string testIdPUserName = "testIdPUserName";
const std::string testIdPPassword = "testIdPPassword";
const std::string testIdPArn = "testIdPArn";
const std::string testOktaAppId = "testOktaAppId";
const std::string testRoleArn = "testRoleArn";
const std::string testAADAppId = "testAADAppId";
const std::string testAADClientSecret = "testAADClientSecret";
const std::string testAADTenant = "testAADTenant";
}  // namespace

const char* BoolToStr(bool val, bool lowerCase = true) {
  if (lowerCase)
    return val ? "true" : "false";

  return val ? "TRUE" : "FALSE";
}

void ParseValidDsnString(const std::string& dsnStr, Configuration& cfg) {
  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  BOOST_CHECK_NO_THROW(parser.ParseConfigAttributes(dsnStr.c_str(), &diag));

  if (diag.GetStatusRecordsNumber() != 0)
    BOOST_FAIL(diag.GetStatusRecord(1).GetMessageText());
}

void ParseValidConnectString(const std::string& connectStr,
                             Configuration& cfg) {
  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectStr, &diag));

  if (diag.GetStatusRecordsNumber() != 0)
    BOOST_FAIL(diag.GetStatusRecord(1).GetMessageText());
}

void ParseConnectStringWithError(const std::string& connectStr,
                                 Configuration& cfg) {
  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectStr, &diag));

  BOOST_CHECK_NE(diag.GetStatusRecordsNumber(), 0);
}

void CheckValidAuthType(const char* connectStr, AuthType::Type authType) {
  Configuration cfg;

  ParseValidConnectString(connectStr, cfg);

  BOOST_CHECK(cfg.GetAuthType() == authType);
}

void CheckInvalidAuthType(const char* connectStr) {
  Configuration cfg;

  ParseConnectStringWithError(connectStr, cfg);

  BOOST_CHECK(cfg.GetAuthType() == Configuration::DefaultValue::authType);
}

void CheckValidLogLevel(const char* connectStr, LogLevel::Type loglevel) {
  Configuration cfg;

  ParseValidConnectString(connectStr, cfg);

  BOOST_CHECK(cfg.GetLogLevel() == loglevel);
}

void CheckInvalidLogLevel(const char* connectStr) {
  Configuration cfg;

  ParseConnectStringWithError(connectStr, cfg);

  BOOST_CHECK(cfg.GetLogLevel() == Configuration::DefaultValue::logLevel);
}

void CheckValidBoolValue(const std::string& connectStr, const std::string& key,
                         bool val) {
  Configuration cfg;

  ParseValidConnectString(connectStr, cfg);

  Configuration::ArgumentMap map;
  cfg.ToMap(map);

  std::string expected = val ? "true" : "false";

  BOOST_CHECK_EQUAL(map[key], expected);
}

void CheckInvalidBoolValue(const std::string& connectStr,
                           const std::string& key) {
  Configuration cfg;

  ParseConnectStringWithError(connectStr, cfg);

  Configuration::ArgumentMap map;
  cfg.ToMap(map);

  BOOST_CHECK(map[key].empty());
}

void CheckConnectionConfig(const Configuration& cfg) {
  BOOST_CHECK_EQUAL(cfg.GetDriver(), testDriverName);
  BOOST_CHECK_EQUAL(cfg.GetUid(), testUid);
  BOOST_CHECK_EQUAL(cfg.GetPwd(), testPwd);
  BOOST_CHECK_EQUAL(cfg.GetAccessKeyId(), testAccessKeyId);
  BOOST_CHECK_EQUAL(cfg.GetSecretKey(), testSecretKey);
  BOOST_CHECK_EQUAL(cfg.GetSessionToken(), testSessionToken);
  BOOST_CHECK_EQUAL(cfg.GetProfileName(), testProfileName);
  BOOST_CHECK_EQUAL(cfg.GetReqTimeout(), testReqTimeoutMS);
  BOOST_CHECK_EQUAL(cfg.GetConnectionTimeout(), testConnectionTimeoutMS);
  BOOST_CHECK_EQUAL(cfg.GetMaxRetryCountClient(), testMaxRetryCountClient);
  BOOST_CHECK_EQUAL(cfg.GetMaxConnections(), testMaxConnections);
  BOOST_CHECK_EQUAL(cfg.GetEndpoint(), testEndpoint);
  BOOST_CHECK_EQUAL(cfg.GetRegion(), testRegion);
  BOOST_CHECK(cfg.GetAuthType() == testAuthType);
  BOOST_CHECK_EQUAL(cfg.GetIdPHost(), testIdPHost);
  BOOST_CHECK_EQUAL(cfg.GetIdPUserName(), testIdPUserName);
  BOOST_CHECK_EQUAL(cfg.GetIdPPassword(), testIdPPassword);
  BOOST_CHECK_EQUAL(cfg.GetIdPArn(), testIdPArn);
  BOOST_CHECK_EQUAL(cfg.GetOktaAppId(), testOktaAppId);
  BOOST_CHECK_EQUAL(cfg.GetRoleArn(), testRoleArn);
  BOOST_CHECK_EQUAL(cfg.GetAADAppId(), testAADAppId);
  BOOST_CHECK_EQUAL(cfg.GetAADClientSecret(), testAADClientSecret);
  BOOST_CHECK_EQUAL(cfg.GetAADTenant(), testAADTenant);
  BOOST_CHECK(cfg.GetLogLevel() == Logger::GetLoggerInstance()->GetLogLevel());
  BOOST_CHECK(cfg.GetLogPath() == Logger::GetLoggerInstance()->GetLogPath());
  BOOST_CHECK(!cfg.IsDsnSet());

  // the expected string is in alphabetical order
  std::stringstream constructor;
  constructor << "aadapplicationid=" << testAADAppId << ';'
              << "aadclientsecret=" << testAADClientSecret << ';'
              << "aadtenant=" << testAADTenant << ';'
              << "accesskeyid=" << testAccessKeyId << ';'
              << "auth=" << AuthType::ToString(testAuthType) << ';'
              << "connectiontimeout=" << testConnectionTimeoutMS << ';'
              << "driver={" << testDriverName << "};"
              << "endpointoverride=" << testEndpoint << ';'
              << "idparn=" << testIdPArn << ';' << "idphost=" << testIdPHost
              << ';' << "idppassword=" << testIdPPassword << ';'
              << "idpusername=" << testIdPUserName << ';' << "loglevel="
              << LogLevel::ToString(Logger::GetLoggerInstance()->GetLogLevel())
              << ';'
              << "logoutput=" << Logger::GetLoggerInstance()->GetLogPath()
              << ';' << "maxconnections=" << testMaxConnections << ';'
              << "maxretrycountclient=" << testMaxRetryCountClient << ';'
              << "oktaapplicationid=" << testOktaAppId << ';'
              << "profilename=" << testProfileName << ";"
              << "pwd=" << testPwd << ';' << "region=" << testRegion << ';'
              << "requesttimeout=" << testReqTimeoutMS << ';'
              << "rolearn=" << testRoleArn << ';'
              << "secretkey=" << testSecretKey << ';'
              << "sessiontoken=" << testSessionToken << ';' << "uid=" << testUid
              << ';';
  const std::string& expectedStr = constructor.str();

  BOOST_CHECK_EQUAL(trino::odbc::common::ToLower(cfg.ToConnectString()),
                    trino::odbc::common::ToLower(expectedStr));
}

void CheckDsnConfig(const Configuration& cfg) {
  // since setting logger path/level will change the logger settings,
  // we will not change the logger path/level in configuration_test,
  // which means it is possible for the logger path/level to be equivalent to
  // the default values. Therefore, there will be no boost check for logger
  // path/level.
  BOOST_CHECK_EQUAL(cfg.GetDriver(), testDriverName);
  BOOST_CHECK_EQUAL(cfg.GetDsn(), testDsn);
  BOOST_CHECK_EQUAL(cfg.GetUid(), Configuration::DefaultValue::uid);
  BOOST_CHECK_EQUAL(cfg.GetPwd(), Configuration::DefaultValue::pwd);
  BOOST_CHECK_EQUAL(cfg.GetAccessKeyId(),
                    Configuration::DefaultValue::accessKeyId);
  BOOST_CHECK_EQUAL(cfg.GetSecretKey(), Configuration::DefaultValue::secretKey);
  BOOST_CHECK_EQUAL(cfg.GetSessionToken(),
                    Configuration::DefaultValue::sessionToken);
  BOOST_CHECK_EQUAL(cfg.GetProfileName(),
                    Configuration::DefaultValue::profileName);
  BOOST_CHECK_EQUAL(cfg.GetReqTimeout(),
                    Configuration::DefaultValue::reqTimeout);
  BOOST_CHECK_EQUAL(cfg.GetConnectionTimeout(),
                    Configuration::DefaultValue::connectionTimeout);
  BOOST_CHECK_EQUAL(cfg.GetMaxRetryCountClient(),
                    Configuration::DefaultValue::maxRetryCountClient);
  BOOST_CHECK_EQUAL(cfg.GetMaxConnections(),
                    Configuration::DefaultValue::maxConnections);
  BOOST_CHECK_EQUAL(cfg.GetEndpoint(), Configuration::DefaultValue::endpoint);
  BOOST_CHECK_EQUAL(cfg.GetRegion(), Configuration::DefaultValue::region);
  BOOST_CHECK(cfg.GetAuthType() == Configuration::DefaultValue::authType);
  BOOST_CHECK_EQUAL(cfg.GetIdPHost(), Configuration::DefaultValue::idPHost);
  BOOST_CHECK_EQUAL(cfg.GetIdPUserName(),
                    Configuration::DefaultValue::idPUserName);
  BOOST_CHECK_EQUAL(cfg.GetIdPPassword(),
                    Configuration::DefaultValue::idPPassword);
  BOOST_CHECK_EQUAL(cfg.GetIdPArn(), Configuration::DefaultValue::idPArn);
  BOOST_CHECK_EQUAL(cfg.GetOktaAppId(), Configuration::DefaultValue::oktaAppId);
  BOOST_CHECK_EQUAL(cfg.GetRoleArn(), Configuration::DefaultValue::roleArn);
  BOOST_CHECK_EQUAL(cfg.GetAADAppId(), Configuration::DefaultValue::aadAppId);
  BOOST_CHECK_EQUAL(cfg.GetAADClientSecret(),
                    Configuration::DefaultValue::aadClientSecret);
  BOOST_CHECK_EQUAL(cfg.GetAADTenant(), Configuration::DefaultValue::aadTenant);
}

BOOST_AUTO_TEST_SUITE(ConfigurationTestSuite)

BOOST_AUTO_TEST_CASE(CheckTestValuesNotEqualDefault) {
  BOOST_CHECK_NE(testDriverName, Configuration::DefaultValue::driver);
  BOOST_CHECK_NE(testDsn, Configuration::DefaultValue::dsn);
  BOOST_CHECK_NE(testUid, Configuration::DefaultValue::uid);
  BOOST_CHECK_NE(testPwd, Configuration::DefaultValue::pwd);
  BOOST_CHECK_NE(testAccessKeyId, Configuration::DefaultValue::accessKeyId);
  BOOST_CHECK_NE(testSecretKey, Configuration::DefaultValue::secretKey);
  BOOST_CHECK_NE(testSessionToken, Configuration::DefaultValue::sessionToken);
  BOOST_CHECK_NE(testProfileName, Configuration::DefaultValue::profileName);
  BOOST_CHECK_NE(testReqTimeoutMS, Configuration::DefaultValue::reqTimeout);
  BOOST_CHECK_NE(testConnectionTimeoutMS,
                 Configuration::DefaultValue::connectionTimeout);
  BOOST_CHECK_NE(testMaxRetryCountClient,
                 Configuration::DefaultValue::maxRetryCountClient);
  BOOST_CHECK_NE(testMaxConnections,
                 Configuration::DefaultValue::maxConnections);
  BOOST_CHECK_NE(testEndpoint, Configuration::DefaultValue::endpoint);
  BOOST_CHECK_NE(testRegion, Configuration::DefaultValue::region);
  BOOST_CHECK(testAuthType != Configuration::DefaultValue::authType);
  BOOST_CHECK_NE(testIdPHost, Configuration::DefaultValue::idPHost);
  BOOST_CHECK_NE(testIdPUserName, Configuration::DefaultValue::idPUserName);
  BOOST_CHECK_NE(testIdPPassword, Configuration::DefaultValue::idPPassword);
  BOOST_CHECK_NE(testIdPArn, Configuration::DefaultValue::idPArn);
  BOOST_CHECK_NE(testOktaAppId, Configuration::DefaultValue::oktaAppId);
  BOOST_CHECK_NE(testRoleArn, Configuration::DefaultValue::roleArn);
  BOOST_CHECK_NE(testAADAppId, Configuration::DefaultValue::aadAppId);
  BOOST_CHECK_NE(testAADClientSecret,
                 Configuration::DefaultValue::aadClientSecret);
  BOOST_CHECK_NE(testAADTenant, Configuration::DefaultValue::aadTenant);
}

BOOST_AUTO_TEST_CASE(TestConnectStringUppercase) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "UID=" << testUid << ';' << "PWD=" << testPwd << ';'
              << "ACCESSKEYID=" << testAccessKeyId << ';'
              << "SECRETKEY=" << testSecretKey << ';'
              << "SESSIONTOKEN=" << testSessionToken << ';' << "LOGLEVEL="
              << LogLevel::ToString(Logger::GetLoggerInstance()->GetLogLevel())
              << ';'
              << "LOGOUTPUT=" << Logger::GetLoggerInstance()->GetLogPath()
              << ';' << "AUTH=" << AuthType::ToString(testAuthType) << ';'
              << "PROFILENAME=" << testProfileName << ';'
              << "REQUESTTIMEOUT=" << testReqTimeoutMS << ';'
              << "CONNECTIONTIMEOUT=" << testConnectionTimeoutMS << ';'
              << "MAXRETRYCOUNTCLIENT=" << testMaxRetryCountClient << ';'
              << "MAXCONNECTIONS=" << testMaxConnections << ';'
              << "ENDPOINTOVERRIDE=" << testEndpoint << ';'
              << "REGION=" << testRegion << ';' << "IDPHOST=" << testIdPHost
              << ';' << "IDPUSERNAME=" << testIdPUserName << ';'
              << "IDPPASSWORD=" << testIdPPassword << ';'
              << "IDPARN=" << testIdPArn << ';'
              << "OKTAAPPLICATIONID=" << testOktaAppId << ';'
              << "ROLEARN=" << testRoleArn << ';'
              << "AADAPPLICATIONID=" << testAADAppId << ';'
              << "AADCLIENTSECRET=" << testAADClientSecret << ';'
              << "AADTENANT=" << testAADTenant << ';' << "DRIVER={"
              << testDriverName << "};";

  const std::string& connectStr = constructor.str();

  ParseValidConnectString(connectStr, cfg);

  CheckConnectionConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringLowercase) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "uid=" << testUid << ';' << "pwd=" << testPwd << ';'
              << "accesskeyid=" << testAccessKeyId << ';'
              << "secretkey=" << testSecretKey << ';'
              << "sessiontoken=" << testSessionToken << ';' << "loglevel="
              << LogLevel::ToString(Logger::GetLoggerInstance()->GetLogLevel())
              << ';'
              << "logoutput=" << Logger::GetLoggerInstance()->GetLogPath()
              << ';' << "auth=" << AuthType::ToString(testAuthType) << ';'
              << "profilename=" << testProfileName << ';'
              << "requesttimeout=" << testReqTimeoutMS << ';'
              << "connectiontimeout=" << testConnectionTimeoutMS << ';'
              << "maxretrycountclient=" << testMaxRetryCountClient << ';'
              << "maxconnections=" << testMaxConnections << ';'
              << "endpointoverride=" << testEndpoint << ';'
              << "region=" << testRegion << ';' << "idphost=" << testIdPHost
              << ';' << "idpusername=" << testIdPUserName << ';'
              << "idppassword=" << testIdPPassword << ';'
              << "idparn=" << testIdPArn << ';'
              << "oktaapplicationid=" << testOktaAppId << ';'
              << "rolearn=" << testRoleArn << ';'
              << "aadapplicationid=" << testAADAppId << ';'
              << "aadclientsecret=" << testAADClientSecret << ';'
              << "aadtenant=" << testAADTenant << ';' << "driver={"
              << testDriverName << "};";

  const std::string& connectStr = constructor.str();

  ParseValidConnectString(connectStr, cfg);

  CheckConnectionConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringZeroTerminated) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "uid=" << testUid << ';' << "pwd=" << testPwd << ';'
              << "accesskeyid=" << testAccessKeyId << ';'
              << "secretkey=" << testSecretKey << ';'
              << "sessiontoken=" << testSessionToken << ';' << "loglevel="
              << LogLevel::ToString(Logger::GetLoggerInstance()->GetLogLevel())
              << ';'
              << "logoutput=" << Logger::GetLoggerInstance()->GetLogPath()
              << ';' << "auth=" << AuthType::ToString(testAuthType) << ';'
              << "profilename=" << testProfileName << ';'
              << "requesttimeout=" << testReqTimeoutMS << ';'
              << "connectiontimeout=" << testConnectionTimeoutMS << ';'
              << "maxretrycountclient=" << testMaxRetryCountClient << ';'
              << "maxconnections=" << testMaxConnections << ';'
              << "endpointoverride=" << testEndpoint << ';'
              << "region=" << testRegion << ';' << "idphost=" << testIdPHost
              << ';' << "idpusername=" << testIdPUserName << ';'
              << "idppassword=" << testIdPPassword << ';'
              << "idparn=" << testIdPArn << ';'
              << "oktaapplicationid=" << testOktaAppId << ';'
              << "rolearn=" << testRoleArn << ';'
              << "aadapplicationid=" << testAADAppId << ';'
              << "aadclientsecret=" << testAADClientSecret << ';'
              << "aadtenant=" << testAADTenant << ';' << "driver={"
              << testDriverName << "};";

  std::string connectStr = constructor.str();

  connectStr.push_back(0);

  ParseValidConnectString(connectStr, cfg);

  CheckConnectionConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringMixed) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "Uid=" << testUid << ';' << "Pwd=" << testPwd << ';'
              << "AccessKeyId=" << testAccessKeyId << ';'
              << "SecretKey=" << testSecretKey << ';'
              << "SessionToken=" << testSessionToken << ';' << "LogLevel="
              << LogLevel::ToString(Logger::GetLoggerInstance()->GetLogLevel())
              << ';'
              << "LogOutput=" << Logger::GetLoggerInstance()->GetLogPath()
              << ';' << "Auth=" << AuthType::ToString(testAuthType) << ';'
              << "ProfileName=" << testProfileName << ';'
              << "RequestTimeout=" << testReqTimeoutMS << ';'
              << "ConnectionTimeout=" << testConnectionTimeoutMS << ';'
              << "MaxRetryCountClient=" << testMaxRetryCountClient << ';'
              << "MaxConnections=" << testMaxConnections << ';'
              << "EndpointOverride=" << testEndpoint << ';'
              << "Region=" << testRegion << ';' << "IdPHost=" << testIdPHost
              << ';' << "IdPUserName=" << testIdPUserName << ';'
              << "IdPPassword=" << testIdPPassword << ';'
              << "IdPArn=" << testIdPArn << ';'
              << "OktaApplicationID=" << testOktaAppId << ';'
              << "RoleArn=" << testRoleArn << ';'
              << "AADApplicationID=" << testAADAppId << ';'
              << "AADClientSecret=" << testAADClientSecret << ';'
              << "AADTenant=" << testAADTenant << ';' << "Driver={"
              << testDriverName << "};";

  const std::string& connectStr = constructor.str();

  ParseValidConnectString(connectStr, cfg);

  CheckConnectionConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringWhiteSpaces) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "UID=      " << testUid << "      ;"
              << "PWD   =  " << testPwd << ";  "
              << "ACCESSKEYID =" << testAccessKeyId << ';'
              << "SECRETKEY=" << testSecretKey << ';'
              << "SESSIONTOKEN=" << testSessionToken << ';' << "  LOGLEVEL ="
              << LogLevel::ToString(Logger::GetLoggerInstance()->GetLogLevel())
              << "  ; "
              << "LOGOUTPUT=  " << Logger::GetLoggerInstance()->GetLogPath()
              << " ;"
              << " AUTH=" << AuthType::ToString(testAuthType) << ';'
              << "     PROFILENAME  = " << testProfileName << "    ; "
              << "  REQUESTTIMEOUT=" << testReqTimeoutMS << "  ;  "
              << "CONNECTIONTIMEOUT=  " << testConnectionTimeoutMS << ";  "
              << "MAXRETRYCOUNTCLIENT=  " << testMaxRetryCountClient << " ;"
              << "MAXCONNECTIONS=  " << testMaxConnections << "  ; "
              << "ENDPOINTOVERRIDE=" << testEndpoint << "  ; "
              << "REGION=" << testRegion << "  ; "
              << "IDPHOST=" << testIdPHost << ";  "
              << "IDPUSERNAME=" << testIdPUserName << ";  "
              << "IDPPASSWORD=" << testIdPPassword << "  ; "
              << "IDPARN=" << testIdPArn << " ;   "
              << "OKTAAPPLICATIONID=" << testOktaAppId << "  ;  "
              << "ROLEARN=" << testRoleArn << ";  "
              << "AADAPPLICATIONID=" << testAADAppId << ";  "
              << "AADCLIENTSECRET=" << testAADClientSecret << "  ; "
              << "AADTENANT=" << testAADTenant << "    ;"
              << "DRIVER = {" << testDriverName << "};";

  const std::string& connectStr = constructor.str();

  ParseValidConnectString(connectStr, cfg);

  CheckConnectionConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringInvalidAuthType) {
  CheckInvalidAuthType("auth=tableau;");
  CheckInvalidAuthType("auth=aat;");
}

BOOST_AUTO_TEST_CASE(TestConnectStringValidAuthType) {
  CheckValidAuthType("auth=password;", AuthType::Type::PASSWORD);
  CheckValidAuthType("auth=iam;", AuthType::Type::IAM);
  CheckValidAuthType("auth=aad;", AuthType::Type::AAD);
  CheckValidAuthType("auth=okta;", AuthType::Type::OKTA);
}

BOOST_AUTO_TEST_CASE(TestConnectStringInvalidLogLevel) {
  CheckInvalidLogLevel("loglevel=debug;");
  CheckInvalidLogLevel("loglevel=off;");
  CheckInvalidLogLevel("loglevel=5;");
  CheckInvalidLogLevel("loglevel=6;");
  CheckInvalidLogLevel("loglevel=-1;");
}

BOOST_AUTO_TEST_CASE(TestConnectStringValidLogLevel) {
  CheckValidLogLevel("loglevel=4;", LogLevel::Type::DEBUG_LEVEL);
  CheckValidLogLevel("loglevel=3;", LogLevel::Type::INFO_LEVEL);
  CheckValidLogLevel("loglevel=2;", LogLevel::Type::WARNING_LEVEL);
  CheckValidLogLevel("loglevel=1;", LogLevel::Type::ERROR_LEVEL);
  CheckValidLogLevel("loglevel=0;", LogLevel::Type::OFF);
}

BOOST_AUTO_TEST_CASE(TestDsnStringUppercase) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "DRIVER=" << testDriverName << '\0' << "DSN={" << testDsn
              << "}" << '\0' << '\0';

  const std::string& configStr = constructor.str();

  ParseValidDsnString(configStr, cfg);

  CheckDsnConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestDsnStringLowercase) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "driver=" << testDriverName << '\0' << "dsn={" << testDsn
              << "}" << '\0' << '\0';

  const std::string& configStr = constructor.str();

  ParseValidDsnString(configStr, cfg);

  CheckDsnConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestDsnStringMixed) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "Driver=" << testDriverName << '\0' << "Dsn={" << testDsn
              << "}" << '\0' << '\0';

  const std::string& configStr = constructor.str();

  ParseValidDsnString(configStr, cfg);

  CheckDsnConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestDsnStringWhitespaces) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << " DRIVER =  " << testDriverName << "\r\n"
              << '\0' << "DSN= {" << testDsn << "} \n"
              << '\0' << '\0';

  const std::string& configStr = constructor.str();

  ParseValidDsnString(configStr, cfg);

  CheckDsnConfig(cfg);
}

#ifdef _WIN32
BOOST_AUTO_TEST_CASE(TestParseDriverVersion) {
  using trino::odbc::system::ui::DsnConfigurationWindow;

  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("02.00.0000"), L"V.2.0.0");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("02.01.0000"), L"V.2.1.0");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("02.10.0000"),
      L"V.2.10.0");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("12.00.0000"),
      L"V.12.0.0");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("02.01.1000"),
      L"V.2.1.1000");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("02.01.0100"),
      L"V.2.1.100");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("02.10.0010"),
      L"V.2.10.10");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("02.01.0200"),
      L"V.2.1.200");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("02.01.0201"),
      L"V.2.1.201");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("02.10.1001"),
      L"V.2.10.1001");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("12.10.0001"),
      L"V.12.10.1");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("08.01.0001"), L"V.8.1.1");
  BOOST_CHECK_EQUAL(
      DsnConfigurationWindow::GetParsedDriverVersion("88.88.8888"),
      L"V.88.88.8888");
}
#endif

BOOST_AUTO_TEST_SUITE_END()
