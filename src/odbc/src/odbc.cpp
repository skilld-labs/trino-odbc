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

#include "trino/odbc.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "trino/odbc/config/configuration.h"
#include "trino/odbc/config/connection_string_parser.h"
#include "trino/odbc/connection.h"
#include "trino/odbc/dsn_config.h"
#include "trino/odbc/environment.h"
#include "trino/odbc/log.h"
#include "trino/odbc/statement.h"
#include "trino/odbc/system/odbc_constants.h"
#include "trino/odbc/system/system_dsn.h"
#include "trino/odbc/type_traits.h"
#include "trino/odbc/utility.h"

using ignite::odbc::diagnostic::Diagnosable;
/**
 * Handle window handle.
 * @param windowHandle Window handle.
 * @param config Configuration.
 * @return @c true on success and @c false otherwise.
 */
bool HandleParentWindow(SQLHWND windowHandle,
                        trino::odbc::config::Configuration& config) {
#ifdef _WIN32
  if (windowHandle) {
    LOG_INFO_MSG("Parent window is passed. Creating configuration window.");
    return DisplayConnectionWindow(windowHandle, config);
  }
#else
  IGNITE_UNUSED(windowHandle);
  IGNITE_UNUSED(config);
#endif
  return true;
}

using namespace trino::odbc::utility;
using trino::odbc::Statement;

namespace trino {
SQLRETURN SQLGetInfo(SQLHDBC conn, SQLUSMALLINT infoType, SQLPOINTER infoValue,
                     SQLSMALLINT infoValueMax, SQLSMALLINT* length) {
  using odbc::Connection;
  using odbc::config::ConnectionInfo;

  LOG_DEBUG_MSG("SQLGetInfo called: "
                << infoType << " ("
                << ConnectionInfo::InfoTypeToString(infoType) << "), "
                << std::hex << reinterpret_cast< size_t >(infoValue) << ", "
                << infoValueMax << ", " << std::hex
                << reinterpret_cast< size_t >(length));

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection)
    return SQL_INVALID_HANDLE;

  connection->GetInfo(infoType, infoValue, infoValueMax, length);

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLAllocHandle(SQLSMALLINT type, SQLHANDLE parent,
                         SQLHANDLE* result) {
  LOG_DEBUG_MSG("SQLAllocHandle called with type " << type);
  switch (type) {
    case SQL_HANDLE_ENV:
      return SQLAllocEnv(result);

    case SQL_HANDLE_DBC:
      return SQLAllocConnect(parent, result);

    case SQL_HANDLE_STMT:
      return SQLAllocStmt(parent, result);

    case SQL_HANDLE_DESC: {
      return SQLAllocDesc(parent, result);
    }
    default:
      break;
  }

  *result = 0;

  return SQL_ERROR;
}

SQLRETURN SQLAllocEnv(SQLHENV* env) {
  using odbc::Environment;

  LOG_DEBUG_MSG("SQLAllocEnv called");

  *env = reinterpret_cast< SQLHENV >(new Environment());

  return SQL_SUCCESS;
}

SQLRETURN SQLAllocConnect(SQLHENV env, SQLHDBC* conn) {
  using odbc::Connection;
  using odbc::Environment;

  LOG_DEBUG_MSG("SQLAllocConnect called");

  *conn = SQL_NULL_HDBC;

  Environment* environment = reinterpret_cast< Environment* >(env);

  if (!environment) {
    LOG_ERROR_MSG("environment is nullptr");
    return SQL_INVALID_HANDLE;
  }

  Connection* connection = environment->CreateConnection();

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return environment->GetDiagnosticRecords().GetReturnCode();
  }

  *conn = reinterpret_cast< SQLHDBC >(connection);

  return SQL_SUCCESS;
}

SQLRETURN SQLAllocStmt(SQLHDBC conn, SQLHSTMT* stmt) {
  using odbc::Connection;

  LOG_DEBUG_MSG("SQLAllocStmt called");

  *stmt = SQL_NULL_HDBC;

  auto connection = static_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  Statement* statement = connection->CreateStatement();

  *stmt = reinterpret_cast< SQLHSTMT >(statement);

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLAllocDesc(SQLHDBC conn, SQLHDESC* desc) {
  using odbc::Connection;

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  Descriptor* descriptor = connection->CreateDescriptor();

  *desc = reinterpret_cast< SQLHDESC >(descriptor);

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLFreeHandle(SQLSMALLINT type, SQLHANDLE handle) {
  LOG_DEBUG_MSG("SQLFreeHandle called with type " << type);

  switch (type) {
    case SQL_HANDLE_ENV:
      return SQLFreeEnv(handle);

    case SQL_HANDLE_DBC:
      return SQLFreeConnect(handle);

    case SQL_HANDLE_STMT:
      return SQLFreeStmt(handle, SQL_DROP);

    case SQL_HANDLE_DESC:
      return SQLFreeDescriptor(handle);

    default:
      break;
  }

  return SQL_ERROR;
}

SQLRETURN SQLFreeEnv(SQLHENV env) {
  using odbc::Environment;

  LOG_DEBUG_MSG("SQLFreeEnv called: " << env);

  Environment* environment = reinterpret_cast< Environment* >(env);

  if (!environment) {
    LOG_ERROR_MSG("environment is nullptr");
    return SQL_INVALID_HANDLE;
  }

  delete environment;

  return SQL_SUCCESS;
}

SQLRETURN SQLFreeConnect(SQLHDBC conn) {
  using odbc::Connection;

  LOG_DEBUG_MSG("SQLFreeConnect called");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  connection->Deregister();

  delete connection;

  return SQL_SUCCESS;
}

SQLRETURN SQLFreeStmt(SQLHSTMT stmt, SQLUSMALLINT option) {
  LOG_DEBUG_MSG("SQLFreeStmt called [option=" << option << ']');

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  if (option == SQL_DROP) {
    // cursor name should be removed from the connection that the cursor name
    // was set for
    statement->GetConnection().RemoveCursorName(statement);
    delete statement;
    return SQL_SUCCESS;
  }

  statement->FreeResources(option);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLFreeDescriptor(SQLHDESC desc) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLFreeDescriptor called");

  Descriptor* descriptor = reinterpret_cast< Descriptor* >(desc);

  if (!descriptor) {
    LOG_ERROR_MSG("descriptor is nullptr");
    return SQL_INVALID_HANDLE;
  }

  // restore the statement implicit descriptors to be active descriptors
  descriptor->Deregister();

  delete descriptor;

  return SQL_SUCCESS;
}

SQLRETURN SQLCloseCursor(SQLHSTMT stmt) {
  LOG_DEBUG_MSG("SQLCloseCursor called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  statement->Close();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLDriverConnect(SQLHDBC conn, SQLHWND windowHandle,
                           SQLWCHAR* inConnectionString,
                           SQLSMALLINT inConnectionStringLen,
                           SQLWCHAR* outConnectionString,
                           SQLSMALLINT outConnectionStringBufferLen,
                           SQLSMALLINT* outConnectionStringLen,
                           SQLUSMALLINT driverCompletion) {
  IGNITE_UNUSED(driverCompletion);

  using odbc::Connection;
  using odbc::diagnostic::DiagnosticRecordStorage;
  using odbc::utility::CopyStringToBuffer;

  LOG_DEBUG_MSG("SQLDriverConnect called");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }
  std::string connectStr =
      SqlWcharToString(inConnectionString, inConnectionStringLen);
  connection->Establish(connectStr, windowHandle);

  DiagnosticRecordStorage& diag = connection->GetDiagnosticRecords();
  if (!diag.IsSuccessful()) {
    LOG_INFO_MSG(
        "SQLFreeStmt exiting becase Diagnostic Record Storage shows operation "
        "is not successful");
    return diag.GetReturnCode();
  }

  bool isTruncated = false;

  size_t reslen = CopyStringToBuffer(
      connectStr, outConnectionString,
      static_cast< size_t >(outConnectionStringBufferLen), isTruncated);

  if (outConnectionStringLen)
    *outConnectionStringLen = static_cast< SQLSMALLINT >(reslen);

  return diag.GetReturnCode();
}

SQLRETURN SQLConnect(SQLHDBC conn, SQLWCHAR* serverName,
                     SQLSMALLINT serverNameLen, SQLWCHAR* userName,
                     SQLSMALLINT userNameLen, SQLWCHAR* auth,
                     SQLSMALLINT authLen) { /*PPP10*/
  using odbc::Connection;
  using odbc::config::Configuration;

  LOG_DEBUG_MSG("SQLConnect called\n");

  Connection* connection = reinterpret_cast< Connection* >(conn);/*PPP9*/

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  odbc::config::Configuration config;

  std::string dsn = SqlWcharToString(serverName, serverNameLen);

  LOG_INFO_MSG("DSN: " << dsn);

  odbc::ReadDsnConfiguration(dsn.c_str(), config,
                             &connection->GetDiagnosticRecords());
  if (userName) {
    std::string userNameStr = SqlWcharToString(userName, userNameLen);
    config.SetUid(userNameStr); /*PPP9*/
  }
  if (auth) {
    std::string passwordStr = SqlWcharToString(auth, authLen);
    config.SetPwd(passwordStr); /*PPP9*/
  }

  connection->Establish(config); /*PPP8*/

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLDisconnect(SQLHDBC conn) {
  using odbc::Connection;

  LOG_DEBUG_MSG("SQLDisconnect called");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  connection->Release();

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLPrepare(SQLHSTMT stmt, SQLWCHAR* query, SQLINTEGER queryLen) {
  LOG_DEBUG_MSG("SQLPrepare called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  std::string sql = SqlWcharToString(query, queryLen);

  LOG_INFO_MSG("SQL: " << sql);

  statement->PrepareSqlQuery(sql);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLExecute(SQLHSTMT stmt) {
  LOG_DEBUG_MSG("SQLExecute called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteSqlQuery();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLExecDirect(SQLHSTMT stmt, SQLWCHAR* query, SQLINTEGER queryLen) {
  LOG_DEBUG_MSG("SQLExecDirect called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  std::string sql = SqlWcharToString(query, queryLen);

  LOG_INFO_MSG("SQL: " << sql);

  statement->ExecuteSqlQuery(sql);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLCancel(SQLHSTMT stmt) {
  LOG_DEBUG_MSG("SQLCancel called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->CancelSqlQuery();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLBindCol(SQLHSTMT stmt, SQLUSMALLINT colNum, SQLSMALLINT targetType,
                     SQLPOINTER targetValue, SQLLEN bufferLength,
                     SQLLEN* strLengthOrIndicator) {
  using namespace odbc::type_traits;
  using odbc::app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLBindCol called: index="
                << colNum << ", type=" << targetType
                << ", targetValue=" << reinterpret_cast< size_t >(targetValue)
                << ", bufferLength=" << bufferLength << ", lengthInd="
                << reinterpret_cast< size_t >(strLengthOrIndicator));

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->BindColumn(colNum, targetType, targetValue, bufferLength,
                        strLengthOrIndicator);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLFetch(SQLHSTMT stmt) {
  LOG_DEBUG_MSG("SQLFetch called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->FetchRow();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLFetchScroll(SQLHSTMT stmt, SQLSMALLINT orientation,
                         SQLLEN offset) {
  LOG_DEBUG_MSG("SQLFetchScroll called with Orientation "
                << orientation << " Offset " << offset);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->FetchScroll(orientation, offset);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLExtendedFetch(SQLHSTMT stmt, SQLUSMALLINT orientation,
                           SQLLEN offset, SQLULEN* rowCount,
                           SQLUSMALLINT* rowStatusArray) {
  LOG_DEBUG_MSG("SQLExtendedFetch called");

  SQLRETURN res = SQLFetchScroll(stmt, orientation, offset);

  if (res == SQL_SUCCESS) {
    if (rowCount)
      *rowCount = 1;

    if (rowStatusArray)
      rowStatusArray[0] = SQL_ROW_SUCCESS;
  } else if (res == SQL_NO_DATA && rowCount)
    *rowCount = 0;

  LOG_DEBUG_MSG("res is " << res);

  // If the SQL function SQLExtendedFetch is called with RowCountPtr
  // setting to 0, the RowCountPtr is a null pointer. The rowCount here
  // is passed from driver manager based on RowCountPtr value.
  // The rowCount is nullptr on Linux but not nullptr on Windows.
  // This behavior is determined by driver manager.
  if (rowCount)
    LOG_DEBUG_MSG("*rowCount is " << *rowCount);

  return res;
}

SQLRETURN SQLNumResultCols(SQLHSTMT stmt, SQLSMALLINT* columnNum) {
  using odbc::meta::ColumnMetaVector;

  LOG_DEBUG_MSG("SQLNumResultCols called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  int32_t res = statement->GetColumnNumber();

  if (columnNum) {
    *columnNum = static_cast< SQLSMALLINT >(res);
    LOG_DEBUG_MSG("columnNum: " << *columnNum);
  }

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLColumns(SQLHSTMT stmt, SQLWCHAR* catalogName,
                     SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                     SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                     SQLSMALLINT tableNameLen, SQLWCHAR* columnName,
                     SQLSMALLINT columnNameLen) {
  LOG_DEBUG_MSG("SQLColumns called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  boost::optional< std::string > catalog =
      SqlWcharToOptString(catalogName, catalogNameLen);
  boost::optional< std::string > schema =
      SqlWcharToOptString(schemaName, schemaNameLen);
  boost::optional< std::string > table =
      SqlWcharToOptString(tableName, tableNameLen);
  boost::optional< std::string > column =
      SqlWcharToOptString(columnName, columnNameLen);

  LOG_INFO_MSG("catalog: " << catalog.get_value_or("")
                           << ", schema: " << schema.get_value_or("")
                           << ", table: " << table << ", column: " << column);

  if (catalog && catalog->empty() && schema && schema->empty()) {
    statement->AddStatusRecord(SqlState::S01000_GENERAL_WARNING,
                               "catalogName and schemaName are empty strings.");
    return SQL_SUCCESS_WITH_INFO;
  }

  statement->ExecuteGetColumnsMetaQuery(catalog, schema, table, column);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLColumnPrivileges(SQLHSTMT stmt, SQLWCHAR* catalogName,
                              SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                              SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                              SQLSMALLINT tableNameLen, SQLWCHAR* columnName,
                              SQLSMALLINT columnNameLen) {
  LOG_DEBUG_MSG("SQLColumnPrivileges called");

  IGNITE_UNUSED(catalogName);
  IGNITE_UNUSED(catalogNameLen);
  IGNITE_UNUSED(schemaName);
  IGNITE_UNUSED(schemaNameLen);
  IGNITE_UNUSED(tableName);
  IGNITE_UNUSED(tableNameLen);
  IGNITE_UNUSED(columnName);
  IGNITE_UNUSED(columnNameLen);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLColumnPrivileges exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteColumnPrivilegesQuery();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLTables(SQLHSTMT stmt, SQLWCHAR* catalogName,
                    SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                    SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                    SQLSMALLINT tableNameLen, SQLWCHAR* tableType,
                    SQLSMALLINT tableTypeLen) {
  LOG_DEBUG_MSG("SQLTables called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  boost::optional< std::string > catalog =
      SqlWcharToOptString(catalogName, catalogNameLen);
  boost::optional< std::string > schema =
      SqlWcharToOptString(schemaName, schemaNameLen);
  boost::optional< std::string > table =
      SqlWcharToOptString(tableName, tableNameLen);
  boost::optional< std::string > tableTypeStr =
      SqlWcharToOptString(tableType, tableTypeLen);

  LOG_INFO_MSG("catalog: " << catalog << ", schema: " << schema << ", table: "
                           << table << ", tableType: " << tableTypeStr);

  statement->ExecuteGetTablesMetaQuery(catalog, schema, table, tableTypeStr);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLTablePrivileges(SQLHSTMT stmt, SQLWCHAR* catalogName,
                             SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                             SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                             SQLSMALLINT tableNameLen) {
  LOG_DEBUG_MSG("SQLTablePrivileges called");

  IGNITE_UNUSED(catalogName);
  IGNITE_UNUSED(catalogNameLen);
  IGNITE_UNUSED(schemaName);
  IGNITE_UNUSED(schemaNameLen);
  IGNITE_UNUSED(tableName);
  IGNITE_UNUSED(tableNameLen);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLTablePrivileges exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteTablePrivilegesQuery();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLMoreResults(SQLHSTMT stmt) {
  LOG_DEBUG_MSG("SQLMoreResults called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->MoreResults();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLNativeSql(SQLHDBC conn, SQLWCHAR* inQuery, SQLINTEGER inQueryLen,
                       SQLWCHAR* outQueryBuffer, SQLINTEGER outQueryBufferLen,
                       SQLINTEGER* outQueryLen) {
  using namespace odbc;

  LOG_DEBUG_MSG("SQLNativeSql called");

  Connection* connection = reinterpret_cast< Connection* >(conn);
  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  int64_t outQueryLenLocal = 0;
  connection->NativeSql(
      inQuery, static_cast< int64_t >(inQueryLen), outQueryBuffer,
      static_cast< int64_t >(outQueryBufferLen), &outQueryLenLocal);
  if (outQueryLen) {
    *outQueryLen = static_cast< SQLINTEGER >(outQueryLenLocal);
    LOG_DEBUG_MSG("*outQueryLen is " << *outQueryLen);
  }

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLColAttribute(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                          SQLUSMALLINT fieldId, SQLPOINTER strAttr,
                          SQLSMALLINT bufferLen, SQLSMALLINT* strAttrLen,
                          SQLLEN* numericAttr) {
  using odbc::meta::ColumnMeta;
  using odbc::meta::ColumnMetaVector;

  LOG_DEBUG_MSG("SQLColAttribute called: "
                << fieldId << " (" << ColumnMeta::AttrIdToString(fieldId)
                << ")");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  // This is a special case
  if (fieldId == SQL_DESC_COUNT) {
    SQLSMALLINT val = 0;

    SQLRETURN res = SQLNumResultCols(stmt, &val);

    if (numericAttr && res == SQL_SUCCESS)
      *numericAttr = val;

    return res;
  }

  statement->GetColumnAttribute(columnNum, fieldId,
                                reinterpret_cast< SQLWCHAR* >(strAttr),
                                bufferLen, strAttrLen, numericAttr);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLDescribeCol(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                         SQLWCHAR* columnNameBuf, SQLSMALLINT columnNameBufLen,
                         SQLSMALLINT* columnNameLen, SQLSMALLINT* dataType,
                         SQLULEN* columnSize, SQLSMALLINT* decimalDigits,
                         SQLSMALLINT* nullable) {
  using odbc::SqlLen;

  LOG_DEBUG_MSG("SQLDescribeCol called with columnNum "
                << columnNum << ", columnNameBuf " << columnNameBuf
                << ", columnNameBufLen" << columnNameBufLen
                << ", columnNameLen " << columnNameLen << ", dataType "
                << dataType << ", columnSize " << columnSize
                << ", decimalDigits " << decimalDigits << ", nullable "
                << nullable);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  std::vector< SQLRETURN > returnCodes;

  // Convert from length in characters to bytes.
  SQLSMALLINT columnNameLenInBytes = 0;
  statement->GetColumnAttribute(columnNum, SQL_DESC_NAME, columnNameBuf,
                                columnNameBufLen * sizeof(SQLWCHAR),
                                &columnNameLenInBytes, 0);
  // Save status of getting column name.
  returnCodes.push_back(statement->GetDiagnosticRecords().GetReturnCode());
  if (columnNameLen) {
    // Convert from length in bytes to characters.
    *columnNameLen = columnNameLenInBytes / sizeof(SQLWCHAR);
  }

  SqlLen dataTypeRes;
  SqlLen columnSizeRes;
  SqlLen decimalDigitsRes;
  SqlLen nullableRes;

  statement->GetColumnAttribute(columnNum, SQL_DESC_TYPE, 0, 0, 0,
                                &dataTypeRes);
  // Save status of getting column type.
  returnCodes.push_back(statement->GetDiagnosticRecords().GetReturnCode());
  statement->GetColumnAttribute(columnNum, SQL_DESC_PRECISION, 0, 0, 0,
                                &columnSizeRes);
  // Save status of getting column precision.
  returnCodes.push_back(statement->GetDiagnosticRecords().GetReturnCode());
  statement->GetColumnAttribute(columnNum, SQL_DESC_SCALE, 0, 0, 0,
                                &decimalDigitsRes);
  // Save status of getting column scale.
  returnCodes.push_back(statement->GetDiagnosticRecords().GetReturnCode());
  statement->GetColumnAttribute(columnNum, SQL_DESC_NULLABLE, 0, 0, 0,
                                &nullableRes);
  // Save status of getting column nullable.
  returnCodes.push_back(statement->GetDiagnosticRecords().GetReturnCode());

  LOG_INFO_MSG("columnNum: "
               << columnNum << ", dataTypeRes: " << dataTypeRes
               << ", columnSizeRes: " << columnSizeRes
               << ", decimalDigitsRes: " << decimalDigitsRes
               << ", nullableRes: " << nullableRes << ", columnNameBuf: "
               << (columnNameBuf
                       ? reinterpret_cast< const char* >(columnNameBuf)
                       : "<null>")
               << ", columnNameLen: " << (columnNameLen ? *columnNameLen : -1));

  if (dataType)
    *dataType = static_cast< SQLSMALLINT >(dataTypeRes);

  if (columnSize)
    *columnSize = static_cast< SQLULEN >(columnSizeRes);

  if (decimalDigits)
    *decimalDigits = static_cast< SQLSMALLINT >(decimalDigitsRes);

  if (nullable)
    *nullable = static_cast< SQLSMALLINT >(nullableRes);

  // Return error code, if any.
  for (auto returnCode : returnCodes) {
    if (!SQL_SUCCEEDED(returnCode)) {
      LOG_INFO_MSG("returnCode is " << returnCode);
      return returnCode;
    }
  }
  // Return success with info, if any.
  for (auto returnCode : returnCodes) {
    if (returnCode != SQL_SUCCESS) {
      LOG_DEBUG_MSG("returnCode is " << returnCode);
      return returnCode;
    }
  }

  return SQL_SUCCESS;
}

SQLRETURN SQLRowCount(SQLHSTMT stmt, SQLLEN* rowCnt) {
  LOG_DEBUG_MSG("SQLRowCount called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  int64_t res = statement->AffectedRows();

  LOG_DEBUG_MSG("Row count: " << res);

  if (rowCnt)
    *rowCnt = static_cast< SQLLEN >((res > 0 ? res : -1));

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLForeignKeys(
    SQLHSTMT stmt, SQLWCHAR* primaryCatalogName,
    SQLSMALLINT primaryCatalogNameLen, SQLWCHAR* primarySchemaName,
    SQLSMALLINT primarySchemaNameLen, SQLWCHAR* primaryTableName,
    SQLSMALLINT primaryTableNameLen, SQLWCHAR* foreignCatalogName,
    SQLSMALLINT foreignCatalogNameLen, SQLWCHAR* foreignSchemaName,
    SQLSMALLINT foreignSchemaNameLen, SQLWCHAR* foreignTableName,
    SQLSMALLINT foreignTableNameLen) {
  LOG_DEBUG_MSG("SQLForeignKeys called");

  IGNITE_UNUSED(primaryCatalogName);
  IGNITE_UNUSED(primaryCatalogNameLen);
  IGNITE_UNUSED(primarySchemaName);
  IGNITE_UNUSED(primarySchemaNameLen);
  IGNITE_UNUSED(primaryTableName);
  IGNITE_UNUSED(primaryTableNameLen);
  IGNITE_UNUSED(foreignCatalogName);
  IGNITE_UNUSED(foreignCatalogNameLen);
  IGNITE_UNUSED(foreignSchemaName);
  IGNITE_UNUSED(foreignSchemaNameLen);
  IGNITE_UNUSED(foreignTableName);
  IGNITE_UNUSED(foreignTableNameLen);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLForeignKeys exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteGetForeignKeysQuery();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetStmtAttr(SQLHSTMT stmt, SQLINTEGER attr, SQLPOINTER valueBuf,
                         SQLINTEGER valueBufLen, SQLINTEGER* valueResLen) {
  LOG_DEBUG_MSG("SQLGetStmtAttr called");

#ifdef _DEBUG
  using odbc::type_traits::StatementAttrIdToString;

  LOG_DEBUG_MSG("Attr: " << StatementAttrIdToString(attr) << " (" << attr
                         << ")");
#endif  //_DEBUG

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->GetAttribute(attr, valueBuf, valueBufLen, valueResLen);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLSetStmtAttr(SQLHSTMT stmt, SQLINTEGER attr, SQLPOINTER value,
                         SQLINTEGER valueLen) {
  LOG_DEBUG_MSG("SQLSetStmtAttr called: " << attr);

#ifdef _DEBUG
  LOG_DEBUG_MSG("Attr: " << odbc::type_traits::StatementAttrIdToString(attr)
                         << " (" << attr << ")");
#endif  //_DEBUG

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->SetAttribute(attr, value, valueLen);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLPrimaryKeys(SQLHSTMT stmt, SQLWCHAR* catalogName,
                         SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                         SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                         SQLSMALLINT tableNameLen) {
  LOG_DEBUG_MSG("SQLPrimaryKeys called");

  IGNITE_UNUSED(catalogName);
  IGNITE_UNUSED(catalogNameLen);
  IGNITE_UNUSED(schemaName);
  IGNITE_UNUSED(schemaNameLen);
  IGNITE_UNUSED(tableName);
  IGNITE_UNUSED(tableNameLen);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLPrimaryKeys exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteGetPrimaryKeysQuery();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetDiagField(SQLSMALLINT handleType, SQLHANDLE handle,
                          SQLSMALLINT recNum, SQLSMALLINT diagId,
                          SQLPOINTER buffer, SQLSMALLINT bufferLen,
                          SQLSMALLINT* resLen) {
  using namespace odbc;
  using namespace odbc::diagnostic;
  using namespace odbc::type_traits;

  using odbc::app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLGetDiagField called with handleType "
                << handleType << ", recNum " << recNum << ", diagId "
                << diagId);

  SqlLen outResLen;
  ApplicationDataBuffer outBuffer(OdbcNativeType::AI_DEFAULT, buffer, bufferLen,
                                  &outResLen);

  SqlResult::Type result;

  DiagnosticField::Type field = DiagnosticFieldToInternal(diagId);

  switch (handleType) {
    case SQL_HANDLE_ENV:
    case SQL_HANDLE_DBC:
    case SQL_HANDLE_STMT: {
      Diagnosable* diag = reinterpret_cast< Diagnosable* >(handle);

      result = diag->GetDiagnosticRecords().GetField(recNum, field, outBuffer);

      break;
    }

    default: {
      result = SqlResult::AI_NO_DATA;
      break;
    }
  }

  if (resLen && result == SqlResult::AI_SUCCESS)
    *resLen = static_cast< SQLSMALLINT >(outResLen);

  return SqlResultToReturnCode(result);
}

SQLRETURN SQLGetDiagRec(SQLSMALLINT handleType, SQLHANDLE handle,
                        SQLSMALLINT recNum, SQLWCHAR* sqlState,
                        SQLINTEGER* nativeError, SQLWCHAR* msgBuffer,
                        SQLSMALLINT msgBufferLen, SQLSMALLINT* msgLen) {
  using namespace odbc::utility;
  using namespace odbc;
  using namespace odbc::diagnostic;
  using namespace odbc::type_traits;

  using odbc::app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLGetDiagRec called with handleType "
                << handleType << ", handle " << handle << ", recNum " << recNum
                << ", sqlState " << sqlState << ", nativeError " << nativeError
                << ", msgBuffer " << msgBuffer << ", msgBufferLen "
                << msgBufferLen << ", msgLen " << msgLen);

  const DiagnosticRecordStorage* records = 0;
  bool isTruncated = false;

  switch (handleType) {
    case SQL_HANDLE_ENV:
    case SQL_HANDLE_DBC:
    case SQL_HANDLE_STMT:
    case SQL_HANDLE_DESC: {
      Diagnosable* diag = reinterpret_cast< Diagnosable* >(handle);

      if (!diag) {
        LOG_ERROR_MSG("diag is null");
        return SQL_INVALID_HANDLE;
      }

      records = &diag->GetDiagnosticRecords();

      break;
    }

    default:
      LOG_ERROR_MSG(
          "SQLGetDiagRec exiting with SQL_INVALID_HANDLE on default case");
      return SQL_INVALID_HANDLE;
  }

  if (recNum < 1 || msgBufferLen < 0) {
    LOG_ERROR_MSG("SQLGetDiagRec exiting with SQL_ERROR. recNum: "
                  << recNum << ", msgBufferLen: " << msgBufferLen);
    return SQL_ERROR;
  }

  if (!records || recNum > records->GetStatusRecordsNumber()) {
    if (records) {
      LOG_ERROR_MSG("SQLGetDiagRec exiting with SQL_NO_DATA. recNum: "
                    << recNum << ", records: " << records
                    << ", records->GetStatusRecordsNumber(): "
                    << records->GetStatusRecordsNumber());
    } else {
      LOG_ERROR_MSG(
          "SQLGetDiagRec exiting with SQL_NO_DATA because records variable is "
          "null. recNum: "
          << recNum << ", records: " << records);
    }
    return SQL_NO_DATA;
  }

  const DiagnosticRecord& record = records->GetStatusRecord(recNum);

  if (sqlState)
    CopyStringToBuffer(record.GetSqlState(), sqlState, 6, isTruncated);

  if (nativeError)
    *nativeError = 0;

  const std::string& errMsg = record.GetMessageText();

  if (!msgBuffer
      || msgBufferLen < static_cast< SQLSMALLINT >(errMsg.size() + 1)) {
    if (!msgLen) {
      LOG_ERROR_MSG(
          "SQLGetDiagRec exiting with SQL_ERROR. msgLen must not be NULL.");
      return SQL_ERROR;
    }

    // Length is given in characters
    *msgLen = CopyStringToBuffer(
        errMsg, msgBuffer, static_cast< size_t >(msgBufferLen), isTruncated);

    return SQL_SUCCESS_WITH_INFO;
  }

  // Length is given in characters
  size_t msgLen0 = CopyStringToBuffer(
      errMsg, msgBuffer, static_cast< size_t >(msgBufferLen), isTruncated);

  if (msgLen)
    *msgLen = msgLen0;

  return SQL_SUCCESS;
}

SQLRETURN SQLGetTypeInfo(SQLHSTMT stmt, SQLSMALLINT type) {
  LOG_DEBUG_MSG("SQLGetTypeInfo called: [type=" << type << ']');

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteGetTypeInfoQuery(static_cast< int16_t >(type));

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetData(SQLHSTMT stmt, SQLUSMALLINT colNum, SQLSMALLINT targetType,
                     SQLPOINTER targetValue, SQLLEN bufferLength,
                     SQLLEN* strLengthOrIndicator) {
  using namespace odbc::type_traits;

  using odbc::app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLGetData called with colNum " << colNum << ", targetType "
                                                 << targetType);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  OdbcNativeType::Type driverType = ToDriverType(targetType);

  ApplicationDataBuffer dataBuffer(driverType, targetValue, bufferLength,
                                   strLengthOrIndicator);

  statement->GetColumnData(colNum, dataBuffer);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLSetEnvAttr(SQLHENV env, SQLINTEGER attr, SQLPOINTER value,
                        SQLINTEGER valueLen) {
  using odbc::Environment;

  LOG_DEBUG_MSG("SQLSetEnvAttr called with Attribute " << attr << ", Value "
                                                       << (size_t)value);

  Environment* environment = reinterpret_cast< Environment* >(env);

  if (!environment) {
    LOG_ERROR_MSG("environment is nullptr");
    return SQL_INVALID_HANDLE;
  }

  environment->SetAttribute(attr, value, valueLen);

  return environment->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetEnvAttr(SQLHENV env, SQLINTEGER attr, SQLPOINTER valueBuf,
                        SQLINTEGER valueBufLen, SQLINTEGER* valueResLen) {
  using namespace odbc;
  using namespace type_traits;

  using app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLGetEnvAttr called with attr " << attr);

  Environment* environment = reinterpret_cast< Environment* >(env);

  if (!environment)
    return SQL_INVALID_HANDLE;

  SqlLen outResLen;
  ApplicationDataBuffer outBuffer(OdbcNativeType::AI_SIGNED_LONG, valueBuf,
                                  static_cast< int32_t >(valueBufLen),
                                  &outResLen);

  environment->GetAttribute(attr, outBuffer);

  if (valueResLen)
    *valueResLen = static_cast< SQLSMALLINT >(outResLen);

  return environment->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLSpecialColumns(SQLHSTMT stmt, SQLSMALLINT idType,
                            SQLWCHAR* catalogName, SQLSMALLINT catalogNameLen,
                            SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen,
                            SQLWCHAR* tableName, SQLSMALLINT tableNameLen,
                            SQLSMALLINT scope, SQLSMALLINT nullable) {
  LOG_DEBUG_MSG("SQLSpecialColumns called");

  IGNITE_UNUSED(idType);
  IGNITE_UNUSED(catalogName);
  IGNITE_UNUSED(catalogNameLen);
  IGNITE_UNUSED(schemaName);
  IGNITE_UNUSED(schemaNameLen);
  IGNITE_UNUSED(tableName);
  IGNITE_UNUSED(tableNameLen);
  IGNITE_UNUSED(scope);
  IGNITE_UNUSED(nullable);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLSpecialColumns exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteSpecialColumnsQuery();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLStatistics(SQLHSTMT stmt, SQLWCHAR* catalogName,
                        SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                        SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                        SQLSMALLINT tableNameLen, SQLUSMALLINT unique,
                        SQLUSMALLINT reserved) {
  LOG_DEBUG_MSG("SQLStatistics called");

  IGNITE_UNUSED(catalogName);
  IGNITE_UNUSED(catalogNameLen);
  IGNITE_UNUSED(schemaName);
  IGNITE_UNUSED(schemaNameLen);
  IGNITE_UNUSED(tableName);
  IGNITE_UNUSED(tableNameLen);
  IGNITE_UNUSED(unique);
  IGNITE_UNUSED(reserved);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLStatistics exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteStatisticsQuery();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLProcedureColumns(SQLHSTMT stmt, SQLWCHAR* catalogName,
                              SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                              SQLSMALLINT schemaNameLen, SQLWCHAR* procName,
                              SQLSMALLINT procNameLen, SQLWCHAR* columnName,
                              SQLSMALLINT columnNameLen) {
  LOG_DEBUG_MSG("SQLProcedureColumns called");

  IGNITE_UNUSED(catalogName);
  IGNITE_UNUSED(catalogNameLen);
  IGNITE_UNUSED(schemaName);
  IGNITE_UNUSED(schemaNameLen);
  IGNITE_UNUSED(procName);
  IGNITE_UNUSED(procNameLen);
  IGNITE_UNUSED(columnName);
  IGNITE_UNUSED(columnNameLen);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLProcedures exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteProcedureColumnsQuery();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLProcedures(SQLHSTMT stmt, SQLWCHAR* catalogName,
                        SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                        SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                        SQLSMALLINT tableNameLen) {
  LOG_DEBUG_MSG("SQLProcedures called");

  IGNITE_UNUSED(catalogName);
  IGNITE_UNUSED(catalogNameLen);
  IGNITE_UNUSED(schemaName);
  IGNITE_UNUSED(schemaNameLen);
  IGNITE_UNUSED(tableName);
  IGNITE_UNUSED(tableNameLen);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLProcedures exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteProceduresQuery();

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLError(SQLHENV env, SQLHDBC conn, SQLHSTMT stmt, SQLWCHAR* state,
                   SQLINTEGER* error, SQLWCHAR* msgBuf, SQLSMALLINT msgBufLen,
                   SQLSMALLINT* msgResLen) {
  using namespace trino::odbc::utility;
  using namespace trino::odbc;
  using namespace trino::odbc::diagnostic;
  using namespace trino::odbc::type_traits;

  using trino::odbc::app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLError is called with env "
                << env << ", conn " << conn << ", stmt " << stmt << ", state "
                << state << ", error " << error << ", msgBuf " << msgBuf
                << ", msgBufLen " << msgBufLen << " msgResLen " << msgResLen);

  SQLHANDLE handle = 0;

  if (env != 0)
    handle = static_cast< SQLHANDLE >(env);
  else if (conn != 0)
    handle = static_cast< SQLHANDLE >(conn);
  else if (stmt != 0)
    handle = static_cast< SQLHANDLE >(stmt);
  else {
    LOG_ERROR_MSG("SQLError exiting with SQL_INVALID_HANDLE");
    return SQL_INVALID_HANDLE;
  }

  Diagnosable* diag = reinterpret_cast< Diagnosable* >(handle);

  DiagnosticRecordStorage& records = diag->GetDiagnosticRecords();

  int32_t recNum = records.GetLastNonRetrieved();
  LOG_DEBUG_MSG("recNum is " << recNum);

  if (recNum < 1 || recNum > records.GetStatusRecordsNumber()) {
    LOG_ERROR_MSG("SQLError exiting with SQL_NO_DATA");
    return SQL_NO_DATA;
  }

  DiagnosticRecord& record = records.GetStatusRecord(recNum);

  record.MarkRetrieved();

  bool isTruncated = false;
  if (state)
    CopyStringToBuffer(record.GetSqlState(), state, 6, isTruncated);

  if (error)
    *error = 0;

  std::string errMsg = record.GetMessageText();
  // NOTE: msgBufLen is in characters.
  size_t outResLen = CopyStringToBuffer(
      errMsg, msgBuf, static_cast< size_t >(msgBufLen), isTruncated);

  if (msgResLen)
    *msgResLen = static_cast< SQLSMALLINT >(outResLen);

  return SQL_SUCCESS;
}

SQLRETURN SQLGetConnectAttr(SQLHDBC conn, SQLINTEGER attr, SQLPOINTER valueBuf,
                            SQLINTEGER valueBufLen, SQLINTEGER* valueResLen) {
  using namespace odbc;
  using namespace type_traits;

  using app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLGetConnectAttr called with attr " << attr);

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  connection->GetAttribute(attr, valueBuf, valueBufLen, valueResLen);

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLSetConnectAttr(SQLHDBC conn, SQLINTEGER attr, SQLPOINTER value,
                            SQLINTEGER valueLen) {
  using odbc::Connection;

  LOG_DEBUG_MSG("SQLSetConnectAttr called(" << attr << ", " << value << ")");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  connection->SetAttribute(attr, value, valueLen);

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetCursorName(SQLHSTMT stmt, SQLWCHAR* nameBuf,
                           SQLSMALLINT nameBufLen, SQLSMALLINT* nameResLen) {
  LOG_DEBUG_MSG("SQLGetCursorName called with nameBufLen " << nameBufLen);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }
  statement->GetCursorName(nameBuf, nameBufLen, nameResLen);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLSetCursorName(SQLHSTMT stmt, SQLWCHAR* name, SQLSMALLINT nameLen) {
  LOG_DEBUG_MSG("SQLSetCursorName called with name " << name << ", nameLen "
                                                     << nameLen);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->SetCursorName(name, nameLen);

  return statement->GetDiagnosticRecords().GetReturnCode();
}
SQLRETURN SQLSetDescField(SQLHDESC descr, SQLSMALLINT recNum,
                          SQLSMALLINT fieldId, SQLPOINTER buffer,
                          SQLINTEGER bufferLen) {
  LOG_DEBUG_MSG("SQLSetDescField called with recNum " << recNum << ", fieldId "
                                                      << fieldId);

  Descriptor* descriptor = reinterpret_cast< Descriptor* >(descr);

  if (!descriptor) {
    LOG_ERROR_MSG("descriptor is nullptr");
    return SQL_INVALID_HANDLE;
  }

  descriptor->SetField(recNum, fieldId, buffer, bufferLen);

  return descriptor->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetDescField(SQLHDESC descr, SQLSMALLINT recNum,
                          SQLSMALLINT fieldId, SQLPOINTER buffer,
                          SQLINTEGER bufferLen, SQLINTEGER* resLen) {
  LOG_DEBUG_MSG("SQLGetDescField called with recNum " << recNum << ", fieldId "
                                                      << fieldId);
  Descriptor* descriptor = reinterpret_cast< Descriptor* >(descr);

  if (!descriptor) {
    LOG_ERROR_MSG("descriptor is nullptr");
    return SQL_INVALID_HANDLE;
  }

  descriptor->GetField(recNum, fieldId, buffer, bufferLen, resLen);

  return descriptor->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLCopyDesc(SQLHDESC src, SQLHDESC dst) {
  LOG_DEBUG_MSG("SQLCopyDesc called");

  Descriptor* srcDesc = reinterpret_cast< Descriptor* >(src);

  if (!srcDesc) {
    LOG_ERROR_MSG("source descriptor is nullptr");
    return SQL_INVALID_HANDLE;
  }

  Descriptor* dstDesc = reinterpret_cast< Descriptor* >(dst);

  if (!dstDesc) {
    LOG_ERROR_MSG("destination descriptor is nullptr");
    return SQL_INVALID_HANDLE;
  }

  srcDesc->CopyDesc(dstDesc);

  return srcDesc->GetDiagnosticRecords().GetReturnCode();
}

#if defined(__APPLE__)
SQLRETURN SQL_API SQLGetFunctions(SQLHDBC conn, SQLUSMALLINT funcId,
                                  SQLUSMALLINT* valueBuf) {
  using odbc::Connection;

  LOG_DEBUG_MSG("SQLGetFunctions called with funcId " << funcId);

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  connection->GetFunctions(funcId, valueBuf);

  return connection->GetDiagnosticRecords().GetReturnCode();
}
#endif

SQLRETURN SQLSetConnectOption(SQLHDBC conn, SQLUSMALLINT option,
                              SQLULEN value) {
  using odbc::Connection;

  LOG_DEBUG_MSG("SQLSetConnectOption called(" << option << ", " << value
                                              << ")");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  connection->SetConnectOption(option, value);
  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetConnectOption(SQLHDBC conn, SQLUSMALLINT option,
                              SQLPOINTER value) {
  using odbc::Connection;

  LOG_DEBUG_MSG("SQLGetConnectOption called(" << option << ")");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG("connection is nullptr");
    return SQL_INVALID_HANDLE;
  }

  connection->GetConnectOption(option, value);
  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetStmtOption(SQLHSTMT stmt, SQLUSMALLINT option,
                           SQLPOINTER value) {
  LOG_DEBUG_MSG("SQLGetStmtOption called with option " << option);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  statement->GetStmtOption(option, value);

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLColAttributes(SQLHSTMT stmt, SQLUSMALLINT colNum,
                           SQLUSMALLINT fieldId, SQLPOINTER strAttrBuf,
                           SQLSMALLINT strAttrBufLen,
                           SQLSMALLINT* strAttrResLen, SQLLEN* numAttrBuf) {
  LOG_DEBUG_MSG("SQLColAttributes called: "
                << fieldId << " ("
                << odbc::meta::ColumnMeta::AttrIdToString(fieldId) << ")");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG("statement is nullptr");
    return SQL_INVALID_HANDLE;
  }

  if (fieldId == SQL_COLUMN_NAME) {
    fieldId = SQL_DESC_NAME;
  } else if (fieldId == SQL_COLUMN_NULLABLE) {
    fieldId = SQL_DESC_NULLABLE;
  } else if (fieldId == SQL_COLUMN_COUNT) {
    fieldId = SQL_DESC_COUNT;
  }

  SQLRETURN ret = SQLColAttribute(stmt, colNum, fieldId, strAttrBuf,
                                  strAttrBufLen, strAttrResLen, numAttrBuf);

  int32_t odbcVer = statement->GetConnection().GetEnvODBCVer();
  if (odbcVer == SQL_OV_ODBC2) {
    if (fieldId == SQL_COLUMN_TYPE) {
      if (*numAttrBuf == SQL_TYPE_DATE)
        *numAttrBuf = SQL_DATE;
      else if (*numAttrBuf == SQL_TYPE_TIME)
        *numAttrBuf = SQL_TIME;
      else if (*numAttrBuf == SQL_TYPE_TIMESTAMP)
        *numAttrBuf = SQL_TIMESTAMP;
    }
  }
  return ret;
}
}  // namespace trino
