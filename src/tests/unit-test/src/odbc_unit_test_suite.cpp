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
#include <fstream>
#include <odbc_unit_test_suite.h>

#include <mock/mock_environment.h>
#include <mock/mock_connection.h>

using namespace boost::unit_test;

/**
 * Test setup config for test results
 */
struct OdbcUnitTestConfig {
  OdbcUnitTestConfig() : test_log("odbc_unit_test_result.xml") {
    unit_test_log.set_stream(test_log);
    unit_test_log.set_format(OF_JUNIT);
  }
  ~OdbcUnitTestConfig() {
    unit_test_log.set_stream(std::cout);
  }

  std::ofstream test_log;
};

BOOST_GLOBAL_FIXTURE(OdbcUnitTestConfig);

namespace timestream {
namespace odbc {
OdbcUnitTestSuite::OdbcUnitTestSuite()
    : env(new MockEnvironment()), dbc(nullptr), stmt(nullptr) {
  dbc = static_cast< MockConnection* >(env->CreateConnection());

  // MockTimestreamService is singleton
  MockTimestreamService::CreateMockTimestreamService();

  // setup credentials in MockTimestreamService
  MockTimestreamService::GetInstance()->AddCredential("AwsTSUnitTestKeyId",
                                                      "AwsTSUnitTestSecretKey");
}

OdbcUnitTestSuite::~OdbcUnitTestSuite() {
  if (stmt) {
    delete stmt;
    stmt = nullptr;
  }

  if (dbc) {
    env->DeregisterConnection(dbc);
    delete dbc;
    dbc = nullptr;
  }

  if (env) {
    delete env;
    env = nullptr;
  }

  // clear the credentials for this test
  MockTimestreamService::GetInstance()->RemoveCredential("AwsTSUnitTestKeyId");

  // destory the singleton to avoid memory leak
  MockTimestreamService::DestoryMockTimestreamService();
}

}  // namespace odbc
}  // namespace timestream
