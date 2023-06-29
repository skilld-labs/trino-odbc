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

#include <mock/mock_connection.h>
#include <mock/mock_httpclient.h>
#include <mock/mock_statement.h>
#include <mock/mock_stsclient.h>
#include <mock/mock_timestream_query_client.h>

namespace timestream {
namespace odbc {

MockConnection::MockConnection(Environment* env) : Connection(env) {
  // No-op
}

MockConnection::~MockConnection() {
  // No-op
}

SqlResult::Type MockConnection::InternalCreateStatement(
    MockStatement*& statement) {
  statement = new MockStatement(*this);

  if (!statement) {
    AddStatusRecord(SqlState::SHY001_MEMORY_ALLOCATION, "Not enough memory.");

    return SqlResult::AI_ERROR;
  }

  return SqlResult::AI_SUCCESS;
}

std::shared_ptr< Aws::TimestreamQuery::TimestreamQueryClient >
MockConnection::CreateTSQueryClient(
    const Aws::Auth::AWSCredentials& credentials,
    const Aws::Client::ClientConfiguration& clientCfg) {
  return std::static_pointer_cast<
      Aws::TimestreamQuery::TimestreamQueryClient >(
      std::make_shared< timestream::odbc::MockTimestreamQueryClient >(
          credentials, clientCfg));
}

std::shared_ptr< Aws::Http::HttpClient > MockConnection::GetHttpClient() {
  return std::static_pointer_cast< Aws::Http::HttpClient >(
      std::make_shared< timestream::odbc::MockHttpClient >());
}

std::shared_ptr< Aws::STS::STSClient > MockConnection::GetStsClient() {
  return std::static_pointer_cast< Aws::STS::STSClient >(
      std::make_shared< MockSTSClient >());
}

MockStatement* MockConnection::CreateStatement() {
  MockStatement* statement;

  InternalCreateStatement(statement);

  return statement;
}
}  // namespace odbc
}  // namespace timestream
