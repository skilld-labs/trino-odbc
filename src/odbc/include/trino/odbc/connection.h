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

#ifndef _TRINO_ODBC_CONNECTION
#define _TRINO_ODBC_CONNECTION

#include <stdint.h>

#include <vector>

#include "trino/odbc/config/configuration.h"
#include "trino/odbc/config/connection_info.h"
#include "trino/odbc/diagnostic/diagnosable_adapter.h"
#include "trino/odbc/log.h"
#include "trino/odbc/ignite_error.h"
#include "ignite/odbc/odbc_error.h"
#include "trino/odbc/authentication/saml.h"
#include "trino/odbc/descriptor.h"

/*#*/
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/trino-query/TrinoQueryClient.h>
#include "aws/sts/STSClient.h"

namespace trino {
namespace odbc {
class Environment;
class Statement;

/**
 * Statement attributes that could be set by ODBC2 SQLSetConnectOption.
 * These attributes will be passed to statement when a statement is created.
 */
struct StatementAttributes {
  SqlUlen bindType;
  SqlUlen concurrency;
  SqlUlen cursorType;
  SqlUlen retrievData;
  SqlUlen rowsetSize;
};

/**
 * ODBC node connection.
 */
class IGNITE_IMPORT_EXPORT Connection : public diagnostic::DiagnosableAdapter {
  friend class Environment;

 public:
  /**
   * Operation with timeout result.
   */
  struct OperationResult {
    enum T { SUCCESS, FAIL, TIMEOUT };
  };

  /** Default connection timeout in seconds. */
  enum { DEFAULT_CONNECT_TIMEOUT = 5 };

  /**
   * Destructor.
   */
  ~Connection();

  /**
   * Get connection info.
   *
   * @return Connection info.
   */
  const config::ConnectionInfo& GetInfo() const;

  /**
   * Get info of any type.
   *
   * @param type Info type.
   * @param buf Result buffer pointer.
   * @param buflen Result buffer length.
   * @param reslen Result value length pointer.
   */
  void GetInfo(config::ConnectionInfo::InfoType type, void* buf, short buflen,
               short* reslen);

  /**
   * Gets the native SQL statement.
   *
   * @param inQuery SQL query to translate to native SQL.
   * @param inQueryLen Length of the SQL query.
   * @param outQueryBuffer Buffer to place the native SQL.
   * @param outQueryBufferLen Length of the output buffer for the native SQL
   * including the null terminating character.
   * @param outQueryLen Actual or required length of the output buffer for the
   * native SQL NOT including the null terminating character.
   */
  template < typename CharT >
  inline void NativeSql(const CharT* inQuery, int64_t inQueryLen,
                        CharT* outQueryBuffer, int64_t outQueryBufferLen,
                        int64_t* outQueryLen) {
    IGNITE_ODBC_API_CALL(InternalNativeSql(inQuery, inQueryLen, outQueryBuffer,
                                           outQueryBufferLen, outQueryLen));
  }

  /**
   * Establish connection to ODBC server.
   *
   * @param connectStr Connection string.
   * @param parentWindow Parent window pointer.
   */
  void Establish(const std::string& connectStr, void* parentWindow);

  /**
   * Establish connection to ODBC server.
   *
   * @param cfg Configuration.
   */
  void Establish(const config::Configuration& cfg);

  /**
   * Release established connection.
   *
   * @return Operation result.
   */
  void Release();

  /**
   * Deregister self from the parent.
   */
  void Deregister();

  /**
   * Get the Trino query client.
   *
   * @return Shared Pointer to Trino query client.
   */
  std::shared_ptr< Aws::TrinoQuery::TrinoQueryClient > /*#*/
  GetQueryClient() const;

  /**
   * Create statement associated with the connection.
   *
   * @return Pointer to valid instance on success and NULL on failure.
   */
  Statement* CreateStatement();

  /**
   * Get configuration.
   *
   * @return Connection configuration.
   */
  const config::Configuration& GetConfiguration() const;

  /**
   * Is auto commit.
   *
   * @return @c true if the auto commit is enabled.
   */
  bool IsAutoCommit() const;

  /**
   * Create diagnostic record associated with the Connection instance.
   *
   * @param sqlState SQL state.
   * @param message Message.
   * @param rowNum Associated row number.
   * @param columnNum Associated column number.
   * @return DiagnosticRecord associated with the instance.
   */
  static diagnostic::DiagnosticRecord CreateStatusRecord(
      SqlState::Type sqlState, const std::string& message, int32_t rowNum = 0,
      int32_t columnNum = 0);

  /**
   * Get connection attribute.
   *
   * @param attr Attribute type.
   * @param buf Buffer for value.
   * @param bufLen Buffer length.
   * @param valueLen Resulting value length.
   */
  void GetAttribute(int attr, void* buf, SQLINTEGER bufLen,
                    SQLINTEGER* valueLen);

  /**
   * Set connection attribute.
   *
   * @param attr Attribute type.
   * @param value Value pointer.
   * @param valueLen Value length.
   */
  void SetAttribute(int attr, void* value, SQLINTEGER valueLen);

#if defined(__APPLE__)
  /**
   * Set function supportability.
   *
   * @param funcId Function Id.
   * @param valueBuf Value array or value.
   */
  void GetFunctions(SQLUSMALLINT funcId, SQLUSMALLINT* valueBuf);
#endif

  /**
   * Return the ODBC Version from env_
   */
  int32_t GetEnvODBCVer();

  /**
   * Get metadataID_.
   *
   * @return metadataID_ value.
   */
  bool GetMetadataID() {
    return metadataID_;
  }

  /**
   * Get TrinoSAMLCredentialsProvider.
   *
   * @return samlCredProvider_ value.
   */
  std::shared_ptr< TrinoSAMLCredentialsProvider >
  GetSAMLCredentialsProvider() {
    return samlCredProvider_;
  }

  /**
   * Create a descriptor.
   *
   * @return New created descriptor pointer.
   */
  Descriptor* CreateDescriptor();

  /**
   * Get trino Log Level from string
   *
   * @param trinoLogLvl trino Log Level String
   *
   * @return trinoLogLevel trino Log Level, default is Warn.
   */
  static Aws::Utils::Logging::LogLevel GetAWSLogLevelFromString(std::string trinoLogLvl); /*#*/

  /**
   * Get cursor name for a statement
   *
   * @param stmt Statement pointer
   *
   * @return cursor name for a statement
   */
  std::string GetCursorName(const Statement* stmt);

  /**
   * Add a cursor name for a statement
   *
   * @param stmt Statement pointer
   * @param cursorName Cursor name
   *
   * @return operation result
   */
  SqlResult::Type AddCursorName(const Statement* stmt, std::string& cursorName);

  /**
   * Remove a cursor name for a statement
   *
   * @param stmt Statement pointer
   */
  void RemoveCursorName(const Statement* stmt);

  /**
   * Check if a cursor name exists for a connection
   *
   * @param cursorName Cursor name
   *
   * @return true if the cursor name exists for a connection.
   */
  bool CursorNameExists(std::string& cursorName);

  /**
   * Set a statement attribute
   *
   * @param option Statement option
   * @param value  Option value
   */
  void SetStmtAttribute(SQLUSMALLINT option, SQLULEN value);

  /**
   * Set a connection option
   *
   * @param option Connection option
   * @param value  Option value
   */
  void SetConnectOption(SQLUSMALLINT option, SQLULEN value);

  /**
   * Get a connection option value
   *
   * @param option Connection option
   * @param value  Option value to be returned
   */
  void GetConnectOption(SQLUSMALLINT option, SQLPOINTER value);

 protected:
  /**
   * Constructor.
   */
  Connection();

  /**
   * Create TrinoQueryClient object.
   *
   * @param credentials trino credentials.
   * @param clientCfg trino client configuration.
   * @return a shared_ptr to created TrinoQueryClient object.
   */
  virtual std::shared_ptr< Aws::TrinoQuery::TrinoQueryClient > /*#*/
  CreateTrinoQueryClient(const Aws::Auth::AWSCredentials& credentials,
                      const Aws::Client::ClientConfiguration& clientCfg); /*@*/

  /**
   * Create trino HttpClient object.
   *
   * @return a shared_ptr to created HttpClient object.
   */
  virtual std::shared_ptr< Aws::Http::HttpClient > GetHttpClient(); /*#*/

  /**
   * Create trino STSClient object.
   *
   * @return a shared_ptr to created STSClient object.
   */
  virtual std::shared_ptr< Aws::STS::STSClient > GetStsClient(); /*#*/

  /**
   * Create statement associated with the connection.
   * Internal call.
   *
   * @param statement Pointer to valid instance on success or NULL on failure.
   * @return Operation result.
   */
  virtual SqlResult::Type InternalCreateStatement(Statement*& statement);

 private:
  IGNITE_NO_COPY_ASSIGNMENT(Connection);

  /**
   * Establish connection to ODBC server.
   * Internal call.
   *
   * @param connectStr Connection string.
   * @param parentWindow Parent window.
   * @return Operation result.
   */
  SqlResult::Type InternalEstablish(const std::string& connectStr,
                                    void* parentWindow);

  /**
   * Establish connection to ODBC server.
   * Internal call.
   *
   * @param cfg Configuration.
   * @return Operation result.
   */
  SqlResult::Type InternalEstablish(const config::Configuration& cfg);

  /**
   * Release established connection.
   * Internal call.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalRelease();

  /**
   * Close connection.
   */
  void Close();

  /**
   * Get info of any type.
   * Internal call.
   *
   * @param type Info type.
   * @param buf Result buffer pointer.
   * @param buflen Result buffer length.
   * @param reslen Result value length pointer.
   * @return Operation result.
   */
  SqlResult::Type InternalGetInfo(config::ConnectionInfo::InfoType type,
                                  void* buf, short buflen, short* reslen);

  /**
   * Gets the native SQL statement.
   *
   * @param inQuery SQL query to translate to native SQL.
   * @param inQueryLen Length of the SQL query.
   * @param outQueryBuffer Buffer to place the native SQL.
   * @param outQueryBufferLen Length of the output buffer for the native SQL
   * including the null terminating character.
   * @param outQueryLen Actual or required length of the output buffer for the
   * native SQL NOT including the null terminating character.
   * @return Operation result.
   */
  template < typename CharT >
  inline SqlResult::Type InternalNativeSql(const CharT* inQuery,
                                           int64_t inQueryLen,
                                           CharT* outQueryBuffer,
                                           int64_t outQueryBufferLen,
                                           int64_t* outQueryLen) {
    bool isTruncated = false;
    if (!inQuery) {
      AddStatusRecord(SqlState::SHY009_INVALID_USE_OF_NULL_POINTER,
                      "The InStatementText argument must not NULL");
      return SqlResult::AI_ERROR;
    }

    SQLINTEGER actualBufferLen = 0;
    if (outQueryBuffer) {
      if (outQueryBufferLen <= 0) {
        AddStatusRecord(SqlState::SHY090_INVALID_STRING_OR_BUFFER_LENGTH,
                        "The BufferLength argument must be greater than zero");
        return SqlResult::AI_ERROR;
      }

      if (inQueryLen == SQL_NTS) {
        for (; inQuery[actualBufferLen] != 0; actualBufferLen++) {
          if ((actualBufferLen + 1) >= outQueryBufferLen) {
            isTruncated = true;
            break;
          }
          outQueryBuffer[actualBufferLen] = inQuery[actualBufferLen];
        }
      } else if (inQueryLen >= 0) {
        for (; actualBufferLen < inQueryLen; actualBufferLen++) {
          if ((actualBufferLen + 1) >= outQueryBufferLen) {
            isTruncated = true;
            break;
          }
          outQueryBuffer[actualBufferLen] = inQuery[actualBufferLen];
        }
      } else {
        AddStatusRecord(
            SqlState::SHY090_INVALID_STRING_OR_BUFFER_LENGTH,
            "The argument TextLength1 was less than 0, but not equal to "
            "SQL_NTS");
        return SqlResult::AI_ERROR;
      }
      outQueryBuffer[actualBufferLen] = 0;
    } else {
      // Get the required length
      if (inQueryLen == SQL_NTS) {
        for (; inQuery[actualBufferLen] != 0; actualBufferLen++) {
        }
      } else if (inQueryLen > 0) {
        actualBufferLen = inQueryLen;
      }
    }

    if (outQueryLen)
      *outQueryLen = actualBufferLen;

    if (isTruncated) {
      AddStatusRecord(
          SqlState::S01004_DATA_TRUNCATED,
          "Buffer is too small for the data. Truncated from the right.");
      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    return SqlResult::AI_SUCCESS;
  }

  /**
   * Get connection attribute.
   * Internal call.
   *
   * @param attr Attribute type.
   * @param buf Buffer for value.
   * @param bufLen Buffer length.
   * @param valueLen Resulting value length.
   * @return Operation result.
   */
  SqlResult::Type InternalGetAttribute(int attr, void* buf, SQLINTEGER bufLen,
                                       SQLINTEGER* valueLen);

  /**
   * Set connection attribute.
   * Internal call.
   *
   * @param attr Attribute type.
   * @param value Value pointer.
   * @param valueLen Value length.
   * @return Operation result.
   */
  SqlResult::Type InternalSetAttribute(int attr, void* value,
                                       SQLINTEGER valueLen);

  /**
   * Set a statement attribute
   * Internal call.
   *
   * @param option Statement option
   * @param value  Option value
   * @return Operation result.
   */
  SqlResult::Type InternalSetStmtAttribute(SQLUSMALLINT option, SQLULEN value);

  /**
   * Set a connection option
   * Internal call.
   *
   * @param option Connection option
   * @param value  Option value
   * @return Operation result.
   */
  SqlResult::Type InternalSetConnectOption(SQLUSMALLINT option, SQLULEN value);

  /**
   * Get a connection option value
   * Internal call.
   *
   * @param option Connection option
   * @param value  Option value to be returned
   * @return Operation result.
   */
  SqlResult::Type InternalGetConnectOption(SQLUSMALLINT option,
                                           SQLPOINTER value);

  /**
   * Create a descriptor.
   * Internal call
   *
   * @param desc Descriptor pointer.
   * @return Operation result.
   */
  SqlResult::Type InternalCreateDescriptor(Descriptor*& desc);

#if defined(__APPLE__)
  /**
   * Set function supportability.
   * Internal call.
   *
   * @param funcId Function Id.
   * @param valueBuf Value array or value.
   * @return Operation result.
   */
  SqlResult::Type InternalGetFunctions(SQLUSMALLINT funcId,
                                       SQLUSMALLINT* valueBuf);
  /**
   * Set ODBC3 function support array for SQL_API_ODBC3_ALL_FUNCTIONS.
   *
   * @param valueBuf Value array.
   */
  void SetODBC3FunctionsValue(SQLUSMALLINT* valueBuf);

  /**
   * Set ODBC2 function support array for SQL_API_ALL_FUNCTIONS.
   *
   * @param valueBuf Value array.
   */
  void SetODBC2FunctionsValue(SQLUSMALLINT* valueBuf);
#endif

  /**
   * Try to restore connection to the cluster.
   *
   * @throw IgniteError on failure.
   * @return @c true on success and @c false otherwise.
   */
  bool TryRestoreConnection(const config::Configuration& cfg, IgniteError& err);

  /**
   * Set client proxy properties based on related environment variables
   *
   * @param clientCfg Client configuration.
   */
  void SetClientProxy(Aws::Client::ClientConfiguration& clientCfg); /*@*/

  /** Connection timeout in seconds. */
  int32_t timeout_ = 0;

  /** Autocommit flag. */
  bool autoCommit_ = true;

  /** Metadata ID flag, indicate if the string arguments of catalog functions
   * are treated as identifiers. */
  bool metadataID_;

  /** Configuration. */
  config::Configuration config_;

  /** Connection info. */
  config::ConnectionInfo info_;

  /** Trino query client. */
  std::shared_ptr< Aws::TrinoQuery::TrinoQueryClient > queryClient_; /*#*/

  /** SAML credentials provider */
  std::shared_ptr< TrinoSAMLCredentialsProvider > samlCredProvider_;

  /** mutex for cursor names update */
  std::mutex cursorNameMutex_;

  /** cursor name set */
  std::set< std::string > cursorNames_;

  /** map for statement and cursor name mapping */
  std::map< const Statement*, std::string > cursorNameMap_;

  /** statement attributes struct */
  StatementAttributes stmtAttr_;
};
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_CONNECTION
