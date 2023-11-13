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

#ifndef ODBC_TEST_ODBC_UNIT_TEST_SUITE
#define ODBC_TEST_ODBC_UNIT_TEST_SUITE

#include <boost/test/unit_test.hpp>

#include <mock/mock_environment.h>
#include <mock/mock_connection.h>
#include <mock/mock_statement.h>
#include "mock/mock_trino_service.h"

#ifndef BOOST_TEST_CONTEXT
#define BOOST_TEST_CONTEXT(...)
#endif

#ifndef BOOST_TEST_INFO
#define BOOST_TEST_INFO(...)
#endif

namespace trino {
namespace odbc {
/**
 * Test setup fixture.
 */
struct OdbcUnitTestSuite {
  /**
   * Constructor.
   */
  OdbcUnitTestSuite();

  /**
   * Destructor.
   */
  virtual ~OdbcUnitTestSuite();

  /** ODBC Environment. */
  MockEnvironment* env;

  /** ODBC Connect. */
  MockConnection* dbc;

  /** ODBC Statement. */
  MockStatement* stmt;
};
}  // namespace odbc
}  // namespace trino

#endif  // ODBC_TEST_ODBC_UNIT_TEST_SUITE
