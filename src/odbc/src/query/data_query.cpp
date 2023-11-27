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

#include "trino/odbc/query/data_query.h"

#include "trino/odbc/connection.h"
#include "trino/odbc/log.h"
#include "ignite/odbc/odbc_error.h"

/*@*/
#include <aws/trino-query/model/Type.h>
#include <aws/trino-query/model/CancelQueryRequest.h>

namespace trino {
namespace odbc {
namespace query {
DataQuery::DataQuery(diagnostic::DiagnosableAdapter& diag,
                     Connection& connection, const std::string& sql)
    : Query(diag, trino::odbc::query::QueryType::DATA),
      connection_(connection),
      sql_(sql),
      resultMetaAvailable_(false),
      resultMeta_(),
      request_(),
      result_(nullptr),
      cursor_(nullptr),
      queryClient_(connection.GetQueryClient()),
      hasAsyncFetch(false),
      rowCounter(0) {
  // No-op.
}

DataQuery::~DataQuery() {
  LOG_DEBUG_MSG("~DataQuery is called");

  if (result_.get())
    InternalClose();
}

SqlResult::Type DataQuery::Execute() {
  LOG_DEBUG_MSG("Execute is called");

  if (result_.get())
    InternalClose();

  SqlResult::Type retval = MakeRequestExecute();

  LOG_DEBUG_MSG("retval is " << retval);
  return retval;
}

SqlResult::Type DataQuery::Cancel() {
  LOG_DEBUG_MSG("Cancel is called");

  if (hasAsyncFetch) {
    if (!result_) {
      LOG_ERROR_MSG("no result found");
      diag.AddStatusRecord(SqlState::SHY000_GENERAL_ERROR,
                           "query is not executed");
      return SqlResult::AI_ERROR;
    }

    // Try to cancel current query
/*@*/
    Aws::TrinoQuery::Model::CancelQueryRequest cancel_request;
    cancel_request.SetQueryId(result_->GetQueryId());

    auto outcome = connection_.GetQueryClient()->CancelQuery(cancel_request);
    std::string message("");
    if (outcome.IsSuccess()) {
      message = "Query ID: " + cancel_request.GetQueryId() + " is cancelled."
                + outcome.GetResult().GetCancellationMessage();
    } else {
      message = "Query ID: " + cancel_request.GetQueryId() + " can't cancel."
                + outcome.GetError().GetMessage();
      // ValidationException is an exception that the query is finished and
      // cancel does not work, it should not be counted as error
      if (outcome.GetError().GetExceptionName() != "ValidationException") {
        LOG_ERROR_MSG(message.c_str());
        diag.AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, message);
        return SqlResult::AI_ERROR;
      }
    }
    LOG_DEBUG_MSG(message.c_str());
  }

  InternalClose();

  return SqlResult::AI_SUCCESS;
}

const meta::ColumnMetaVector* DataQuery::GetMeta() {
  LOG_DEBUG_MSG("GetMeta is called");

  if (!resultMetaAvailable_) {
    MakeRequestResultsetMeta();

    if (!resultMetaAvailable_) {
      LOG_ERROR_MSG("Returning nullptr due to no available result meta");

      return nullptr;
    }
  }

  return &resultMeta_;
}

/**
 * Fetch one page asynchronously. It will be
 * executed in an asynchronous thread.
 *
 * @return void.
 */
/*@*/
void AsyncFetchOnePage(
    const std::shared_ptr< Aws::TrinoQuery::TrinoQueryClient > client,
    const QueryRequest& request, DataQueryContext& context_) {
  LOG_DEBUG_MSG("AsyncFetchOnePage is called");
  Aws::TrinoQuery::Model::QueryOutcome result;
  result = client->Query(request);

  std::unique_lock< std::mutex > locker(context_.mutex_);
  context_.cv_.wait(locker, [&]() {
    // This thread could only continue when context_.queue_ is empty
    // or the main thread is exiting.
    return context_.queue_.empty() || context_.isClosing_;
  });

  if (context_.queue_.empty()) {
    LOG_DEBUG_MSG("Result queue is empty");
    // context_.queue_ hold one element at most
    context_.queue_.push(result);
    context_.cv_.notify_one();
  }
}

/*@*/
SqlResult::Type DataQuery::SwitchCursor() {
  LOG_DEBUG_MSG("SwitchCursor is called");
  std::unique_lock< std::mutex > locker(context_.mutex_);
  context_.cv_.wait(locker, [&]() { return !context_.queue_.empty(); });
  Aws::TrinoQuery::Model::QueryOutcome outcome = context_.queue_.front();
  context_.queue_.pop();
  locker.unlock();

  if (!outcome.IsSuccess()) {
    auto& error = outcome.GetError();
    LOG_ERROR_MSG("ERROR: " << error.GetExceptionName() << ": "
                            << error.GetMessage() << ", for query " << sql_
                            << ", number of rows fetched: " << rowCounter);
    cursor_.reset();
    hasAsyncFetch = false;  // no async fetch any more
    return SqlResult::Type::AI_ERROR;
  }

  result_ = std::make_shared< QueryResult >(outcome.GetResult());
/*@*/
  const Aws::Vector< Row >& rows = outcome.GetResult().GetRows();
  const Aws::String& token = outcome.GetResult().GetNextToken();
  if (rows.empty()) {
    LOG_INFO_MSG(
        "Data fetching is finished, number of rows fetched: " << rowCounter);
    return SqlResult::AI_NO_DATA;
  }

  // switch to rows in next page
  cursor_.reset(new TrinoCursor(rows, resultMeta_));
  cursor_->Increment();  // The cursor_ needs to be incremented before using it
                         // for the first time

  if (token.empty()) {
    hasAsyncFetch = false;  // no async fetch any more
    LOG_INFO_MSG(
        "Data fetching is finished, number of rows fetched: " << rowCounter);
  } else {
    if (!threads_.empty()) {
      std::thread& itr = threads_.front();
      // wait for the last thread to end. The join() should be done before the
      // thread is popped from the queue. The thread could not be joined after
      // it is popped from the queue, as it could cause crash.
      LOG_DEBUG_MSG("Waiting for thread " << itr.get_id() << " to end");
      if (itr.joinable()) {
        itr.join();
      }
      threads_.pop();
    } else {
      LOG_DEBUG_MSG("The threads queue is empty");
    }

    request_.SetNextToken(token);
    std::thread next(AsyncFetchOnePage, queryClient_, std::ref(request_),
                     std::ref(context_));
    LOG_DEBUG_MSG("New thread " << next.get_id() << " is started");
    addThreads(next);
  }

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::FetchNextRow(app::ColumnBindingMap& columnBindings) {
  LOG_DEBUG_MSG("FetchNextRow is called");
  if (!cursor_) {
    diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                         "Cursor does not point to any data.",
                         trino::odbc::LogLevel::Type::WARNING_LEVEL);
    return SqlResult::AI_NO_DATA;
  }

  if (!cursor_->Increment()) {
    if (hasAsyncFetch) {
      SqlResult::Type result = SwitchCursor();
      if (result != SqlResult::AI_SUCCESS) {
        diag.AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE,
                             "Invalid cursor state.",
                             trino::odbc::LogLevel::Type::WARNING_LEVEL);
        return result;
      }
    } else {
      LOG_INFO_MSG(
          "Exit due to cursor has reached the "
          "end.");
      return SqlResult::AI_NO_DATA;
    }
  }

  for (uint32_t i = 1; i < cursor_->GetColumnSize() + 1; ++i) {
    app::ColumnBindingMap::iterator it = columnBindings.find(i);

    if (it == columnBindings.end())
      continue;
  
    app::ConversionResult::Type convRes =
        cursor_->ReadColumnToBuffer(i, it->second);

    SqlResult::Type result = ProcessConversionResult(convRes, 0, i);

    if (result == SqlResult::AI_ERROR) {
      LOG_ERROR_MSG("Exit due to data reading error");
      return SqlResult::AI_ERROR;
    }
  }

  rowCounter++;
  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::GetColumn(uint16_t columnIdx,
                                     app::ApplicationDataBuffer& buffer) {
  LOG_DEBUG_MSG("GetColumn is called");

  if (!cursor_) {
    diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                         "Cursor does not point to any data.",
                         trino::odbc::LogLevel::Type::WARNING_LEVEL);

    return SqlResult::AI_NO_DATA;
  }

  if (!cursor_->HasData()) {
    diag.AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE,
                         "Cursor has reached end of the result set.");

    return SqlResult::AI_ERROR;
  }

  app::ConversionResult::Type convRes =
      cursor_->ReadColumnToBuffer(columnIdx, buffer);

  SqlResult::Type result = ProcessConversionResult(convRes, 0, columnIdx);

  LOG_DEBUG_MSG("result is " << result);
  return result;
}

SqlResult::Type DataQuery::Close() {
  LOG_DEBUG_MSG("Close is called");

  SqlResult::Type retval = InternalClose();

  LOG_DEBUG_MSG("retval is " << retval);
  return retval;
}

SqlResult::Type DataQuery::InternalClose() {
  LOG_DEBUG_MSG("InternalClose is called");

  // stop all asynchronous threads
  context_.isClosing_ = true;
  while (!threads_.empty()) {
    std::thread& itr = threads_.front();
    // wait for the last thread to end. The join() should be done before the
    // thread is popped from the queue. The thread could not be joined after
    // it is popped from the queue, as it could cause crash.
    if (itr.joinable()) {
      itr.join();
    }
    threads_.pop();
  }

  result_.reset();
  cursor_.reset();

  return SqlResult::AI_SUCCESS;
}

bool DataQuery::DataAvailable() const {
  return cursor_ != nullptr;
}

int64_t DataQuery::AffectedRows() const {
  // Return zero by default, since number of affected rows is non-zero only
  // if we're executing update statements, which is not supported
  return 0;
}

int64_t DataQuery::RowNumber() const {
  if (!cursor_ || !cursor_->HasData()) {
    diag.AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                         "Cursor does not point to any data.",
                         trino::odbc::LogLevel::Type::WARNING_LEVEL);

    LOG_DEBUG_MSG("Row number returned is 0.");

    return 0;
  }
  LOG_DEBUG_MSG("Row number returned: " << rowCounter);
  return int64_t(rowCounter);
}

SqlResult::Type DataQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}

SqlResult::Type DataQuery::MakeRequestExecute() {
  // This function is called by Execute() and does the actual querying
  LOG_DEBUG_MSG("MakeRequestExecute is called");

  LOG_INFO_MSG("sql query: " << sql_);
  request_.SetQueryString(sql_);
  if (connection_.GetConfiguration().IsMaxRowPerPageSet()) {
    LOG_DEBUG_MSG("MaxRowPerPage is set to "
                  << connection_.GetConfiguration().GetMaxRowPerPage());
    request_.SetMaxRows(connection_.GetConfiguration().GetMaxRowPerPage());
  }

  do {
/*@*/
    Aws::TrinoQuery::Model::QueryOutcome outcome =
        connection_.GetQueryClient()->Query(request_);

    if (!outcome.IsSuccess()) {
      auto error = outcome.GetError();
      LOG_ERROR_MSG("ERROR: " << error.GetExceptionName() << ": "
                              << error.GetMessage() << " for query " << sql_);

      diag.AddStatusRecord(
          SqlState::SHY000_GENERAL_ERROR,
          "AWS API Failure: Failed to execute query \"" + sql_ + "\"");
      InternalClose();
      return SqlResult::AI_ERROR;
    }

    // outcome is successful, update result_
    result_ = std::make_shared< QueryResult >(outcome.GetResult());
    if (result_->GetRows().empty()) {
      if (result_->GetNextToken().empty()) {
        // result is empty
        LOG_DEBUG_MSG("QueryResult is empty, returning no data");
        return SqlResult::AI_NO_DATA;
      }
      request_.SetNextToken(result_->GetNextToken());
      continue;
    } else {
      break;
    }
  } while (true);

  cursor_.reset(new TrinoCursor(result_->GetRows(), resultMeta_));

  if (!result_->GetNextToken().empty()) {
    LOG_DEBUG_MSG(
        "Next token is not empty, starting async thread to fetch next page");
    request_.SetNextToken(result_->GetNextToken());
    std::thread next(AsyncFetchOnePage, queryClient_, std::ref(request_),
                     std::ref(context_));
    addThreads(next);
    hasAsyncFetch = true;
  }

  SqlResult::Type retval = MakeRequestFetch();
  return retval;
}

SqlResult::Type DataQuery::MakeRequestFetch() {
  LOG_DEBUG_MSG("MakeRequestFetch is called");

  if (!result_.get()) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "result_ is a null pointer");
    return SqlResult::AI_ERROR;
  }

/*@*/
  const Aws::Vector< ColumnInfo >& columnInfo = result_->GetColumnInfo();

  if (!resultMetaAvailable_) {
    ReadColumnMetadataVector(columnInfo);
  }

  SqlResult::Type retval = SqlResult::AI_SUCCESS;

  if (result_->GetRows().empty()) {
    retval = SqlResult::AI_NO_DATA;
  } else {
    LOG_DEBUG_MSG("Result has " << result_->GetRows().size() << " rows");
    cursor_.reset(new TrinoCursor(result_->GetRows(), resultMeta_));
  }

  LOG_DEBUG_MSG("retval is " << retval);

  return retval;
}

SqlResult::Type DataQuery::MakeRequestResultsetMeta() {
  LOG_DEBUG_MSG("MakeRequestResultsetMeta is called");

  QueryRequest request;
  request.SetQueryString(sql_);

/*@*/
  Aws::TrinoQuery::Model::QueryOutcome outcome =
      connection_.GetQueryClient()->Query(request);

  if (!outcome.IsSuccess()) {
    auto const error = outcome.GetError();

    diag.AddStatusRecord(SqlState::SHY000_GENERAL_ERROR,
                         "AWS API ERROR: " + error.GetExceptionName() + ": "
                             + error.GetMessage() + " for query " + sql_);

    InternalClose();
    return SqlResult::AI_ERROR;
  }
  // outcome is successful
  QueryResult result = outcome.GetResult();
  const Aws::Vector< ColumnInfo >& columnInfo = result.GetColumnInfo();

/*@*/
  ReadColumnMetadataVector(columnInfo);

  return SqlResult::AI_SUCCESS;
}

void DataQuery::ReadColumnMetadataVector(
    const Aws::Vector< ColumnInfo >& trinoVector) {
  LOG_DEBUG_MSG("ReadColumnMetadataVector is called");

  using trino::odbc::meta::ColumnMeta;
  resultMeta_.clear();

  if (trinoVector.empty()) {
    LOG_ERROR_MSG("Exit due to column vector is empty");

    return;
  }

/*@*/
  for (ColumnInfo trinoMetadata : trinoVector) {
    resultMeta_.emplace_back(ColumnMeta());
    resultMeta_.back().ReadMetadata(trinoMetadata);
  }
  resultMetaAvailable_ = true;
}

SqlResult::Type DataQuery::ProcessConversionResult(
    app::ConversionResult::Type convRes, int32_t rowIdx, int32_t columnIdx) {
  LOG_DEBUG_MSG("ProcessConversionResult is called with convRes is "
                << static_cast< int >(convRes));

  switch (convRes) {
    case app::ConversionResult::Type::AI_SUCCESS: {
      return SqlResult::AI_SUCCESS;
    }

    case app::ConversionResult::Type::AI_NO_DATA: {
      return SqlResult::AI_NO_DATA;
    }

    case app::ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED: {
      diag.AddStatusRecord(
          SqlState::S01004_DATA_TRUNCATED,
          "Buffer is too small for the column data. Truncated from the right.",
          trino::odbc::LogLevel::Type::WARNING_LEVEL, rowIdx, columnIdx);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::Type::AI_FRACTIONAL_TRUNCATED: {
      diag.AddStatusRecord(
          SqlState::S01S07_FRACTIONAL_TRUNCATION,
          "Buffer is too small for the column data. Fraction truncated.",
          trino::odbc::LogLevel::Type::WARNING_LEVEL, rowIdx, columnIdx);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::Type::AI_INDICATOR_NEEDED: {
      diag.AddStatusRecord(
          SqlState::S22002_INDICATOR_NEEDED,
          "Indicator is needed but not suplied for the column buffer.",
          trino::odbc::LogLevel::Type::WARNING_LEVEL, rowIdx, columnIdx);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::Type::AI_UNSUPPORTED_CONVERSION: {
      diag.AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                           "Data conversion is not supported.",
                           trino::odbc::LogLevel::Type::WARNING_LEVEL,
                           rowIdx, columnIdx);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::Type::AI_FAILURE:
      LOG_DEBUG_MSG("parameter: convRes: AI_FAILURE");
    default: {
      diag.AddStatusRecord(
          SqlState::S01S01_ERROR_IN_ROW, "Can not retrieve row column.",
          trino::odbc::LogLevel::Type::WARNING_LEVEL, rowIdx, columnIdx);
      break;
    }
  }

  return SqlResult::AI_ERROR;
}

void DataQuery::SetResultsetMeta(const meta::ColumnMetaVector& value) {
  LOG_DEBUG_MSG("SetResultsetMeta is called");

  // assign `resultMeta_` with contents of `value`
  resultMeta_.assign(value.begin(), value.end());
  resultMetaAvailable_ = true;

  // the nested forloops are for logging purposes
  for (size_t i = 0; i < resultMeta_.size(); ++i) {
    meta::ColumnMeta& meta = resultMeta_.at(i);
    if (meta.GetDataType()) {
      LOG_DEBUG_MSG(
          "\n[" << i << "] SchemaName:     "
                << meta.GetSchemaName().get_value_or("") << "\n[" << i
                << "] TypeName:       " << meta.GetTableName().get_value_or("")
                << "\n[" << i
                << "] ColumnName:     " << meta.GetColumnName().get_value_or("")
                << "\n[" << i << "] ColumnType:     "
                << static_cast< int32_t >(*meta.GetDataType()));
    } else {
      LOG_DEBUG_MSG(
          "\n[" << i << "] SchemaName:     "
                << meta.GetSchemaName().get_value_or("") << "\n[" << i
                << "] TypeName:       " << meta.GetTableName().get_value_or("")
                << "\n[" << i
                << "] ColumnName:     " << meta.GetColumnName().get_value_or("")
                << "\n[" << i << "] ColumnType: not available");
    }
  }
}
}  // namespace query
}  // namespace odbc
}  // namespace trino
