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

/*$*//*WIP marker*/
/*PP[num]*//*path marker*/
/*@*//*aws marker*/
/*#*//*direct aws marker*/

#include <ignite/common/common.h>

#include "trino/odbc.h"
#include "trino/odbc/environment.h"
#include "trino/odbc/connection.h"
#include "trino/odbc/log.h"
#include "trino/odbc/utility.h"
#include "trino/odbc/statement.h"

#define ENV_UNSUPPORTED_FUNC(env, diagStr)                          \
  using trino::odbc::Environment;                              \
  Environment* environment = reinterpret_cast< Environment* >(env); \
  environment->AddStatusRecord(                                     \
      SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED, diagStr);

#define CONN_UNSUPPORTED_FUNC(conn, diagStr)                      \
  using trino::odbc::Connection;                             \
  Connection* connection = reinterpret_cast< Connection* >(conn); \
  connection->AddStatusRecord(                                    \
      SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED, diagStr);

#define STMT_UNSUPPORTED_FUNC(stmt, diagStr)                   \
  using trino::odbc::Statement;                           \
  Statement* statement = reinterpret_cast< Statement* >(stmt); \
  statement->AddStatusRecord(                                  \
      SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED, diagStr);

#define DESC_UNSUPPORTED_FUNC(desc, diagStr)                      \
  using trino::odbc::Descriptor;                             \
  Descriptor* descriptor = reinterpret_cast< Descriptor* >(desc); \
  descriptor->AddStatusRecord(                                    \
      SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED, diagStr);

SQLRETURN SQL_API SQLGetInfo(SQLHDBC conn, SQLUSMALLINT infoType,
                             SQLPOINTER infoValue, SQLSMALLINT infoValueMax,
                             SQLSMALLINT* length) {
  return trino::SQLGetInfo(conn, infoType, infoValue, infoValueMax,
                                length);
}

SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT type, SQLHANDLE parent,
                                 SQLHANDLE* result) {
  return trino::SQLAllocHandle(type, parent, result);
}

SQLRETURN SQL_API SQLAllocEnv(SQLHENV* env) {
  return trino::SQLAllocEnv(env);
}

SQLRETURN SQL_API SQLAllocConnect(SQLHENV env, SQLHDBC* conn) {
  return trino::SQLAllocConnect(env, conn);
}

SQLRETURN SQL_API SQLAllocStmt(SQLHDBC conn, SQLHSTMT* stmt) {
  return trino::SQLAllocStmt(conn, stmt);
}

SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT type, SQLHANDLE handle) {
  return trino::SQLFreeHandle(type, handle);
}

SQLRETURN SQL_API SQLFreeEnv(SQLHENV env) {
  return trino::SQLFreeEnv(env);
}

SQLRETURN SQL_API SQLFreeConnect(SQLHDBC conn) {
  return trino::SQLFreeConnect(conn);
}

SQLRETURN SQL_API SQLFreeStmt(SQLHSTMT stmt, SQLUSMALLINT option) {
  return trino::SQLFreeStmt(stmt, option);
}

SQLRETURN SQL_API SQLCloseCursor(SQLHSTMT stmt) {
  return trino::SQLCloseCursor(stmt);
}

SQLRETURN SQL_API SQLDriverConnect(SQLHDBC conn, SQLHWND windowHandle,
                                   SQLWCHAR* inConnectionString,
                                   SQLSMALLINT inConnectionStringLen,
                                   SQLWCHAR* outConnectionString,
                                   SQLSMALLINT outConnectionStringBufferLen,
                                   SQLSMALLINT* outConnectionStringLen,
                                   SQLUSMALLINT driverCompletion) {
  return trino::SQLDriverConnect(
      conn, windowHandle, inConnectionString, inConnectionStringLen,
      outConnectionString, outConnectionStringBufferLen, outConnectionStringLen,
      driverCompletion);
}

SQLRETURN SQL_API SQLConnect(SQLHDBC conn, SQLWCHAR* serverName,
                             SQLSMALLINT serverNameLen, SQLWCHAR* userName,
                             SQLSMALLINT userNameLen, SQLWCHAR* auth,
                             SQLSMALLINT authLen) {
  return trino::SQLConnect(conn, serverName, serverNameLen, userName,
                                userNameLen, auth, authLen);
}

SQLRETURN SQL_API SQLDisconnect(SQLHDBC conn) {
  return trino::SQLDisconnect(conn);
}

SQLRETURN SQL_API SQLPrepare(SQLHSTMT stmt, SQLWCHAR* query,
                             SQLINTEGER queryLen) {
  return trino::SQLPrepare(stmt, query, queryLen);
}

SQLRETURN SQL_API SQLExecute(SQLHSTMT stmt) {
  return trino::SQLExecute(stmt);
}

SQLRETURN SQL_API SQLExecDirect(SQLHSTMT stmt, SQLWCHAR* query,
                                SQLINTEGER queryLen) {
  return trino::SQLExecDirect(stmt, query, queryLen);
}

SQLRETURN SQL_API SQLCancel(SQLHSTMT stmt) {
  return trino::SQLCancel(stmt);
}

SQLRETURN SQL_API SQLBindCol(SQLHSTMT stmt, SQLUSMALLINT colNum,
                             SQLSMALLINT targetType, SQLPOINTER targetValue,
                             SQLLEN bufferLength,
                             SQLLEN* strLengthOrIndicator) {
  return trino::SQLBindCol(stmt, colNum, targetType, targetValue,
                                bufferLength, strLengthOrIndicator);
}

SQLRETURN SQL_API SQLFetch(SQLHSTMT stmt) {
  return trino::SQLFetch(stmt);
}

SQLRETURN SQL_API SQLFetchScroll(SQLHSTMT stmt, SQLSMALLINT orientation,
                                 SQLLEN offset) {
  return trino::SQLFetchScroll(stmt, orientation, offset);
}

SQLRETURN SQL_API SQLExtendedFetch(SQLHSTMT stmt, SQLUSMALLINT orientation,
                                   SQLLEN offset, SQLULEN* rowCount,
                                   SQLUSMALLINT* rowStatusArray) {
  return trino::SQLExtendedFetch(stmt, orientation, offset, rowCount,
                                      rowStatusArray);
}

SQLRETURN SQL_API SQLNumResultCols(SQLHSTMT stmt, SQLSMALLINT* columnNum) {
  return trino::SQLNumResultCols(stmt, columnNum);
}

SQLRETURN SQL_API SQLTables(SQLHSTMT stmt, SQLWCHAR* catalogName,
                            SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                            SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                            SQLSMALLINT tableNameLen, SQLWCHAR* tableType,
                            SQLSMALLINT tableTypeLen) {
  return trino::SQLTables(stmt, catalogName, catalogNameLen, schemaName,
                               schemaNameLen, tableName, tableNameLen,
                               tableType, tableTypeLen);
}

SQLRETURN SQL_API SQLTablePrivileges(SQLHSTMT stmt, SQLWCHAR* catalogName,
                                     SQLSMALLINT catalogNameLen,
                                     SQLWCHAR* schemaName,
                                     SQLSMALLINT schemaNameLen,
                                     SQLWCHAR* tableName,
                                     SQLSMALLINT tableNameLen) {
  return trino::SQLTablePrivileges(stmt, catalogName, catalogNameLen,
                                        schemaName, schemaNameLen, tableName,
                                        tableNameLen);
}

SQLRETURN SQL_API SQLColumns(SQLHSTMT stmt, SQLWCHAR* catalogName,
                             SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                             SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                             SQLSMALLINT tableNameLen, SQLWCHAR* columnName,
                             SQLSMALLINT columnNameLen) {
  return trino::SQLColumns(stmt, catalogName, catalogNameLen, schemaName,
                                schemaNameLen, tableName, tableNameLen,
                                columnName, columnNameLen);
}

SQLRETURN SQL_API SQLColumnPrivileges(
    SQLHSTMT stmt, SQLWCHAR* catalogName, SQLSMALLINT catalogNameLen,
    SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen, SQLWCHAR* columnName, SQLSMALLINT columnNameLen) {
  return trino::SQLColumnPrivileges(
      stmt, catalogName, catalogNameLen, schemaName, schemaNameLen, tableName,
      tableNameLen, columnName, columnNameLen);
}

SQLRETURN SQL_API SQLMoreResults(SQLHSTMT stmt) {
  return trino::SQLMoreResults(stmt);
}

SQLRETURN SQL_API SQLNativeSql(SQLHDBC conn, SQLWCHAR* inQuery,
                               SQLINTEGER inQueryLen, SQLWCHAR* outQueryBuffer,
                               SQLINTEGER outQueryBufferLen,
                               SQLINTEGER* outQueryLen) {
  return trino::SQLNativeSql(conn, inQuery, inQueryLen, outQueryBuffer,
                                  outQueryBufferLen, outQueryLen);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLColAttribute(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                                  SQLUSMALLINT fieldId, SQLPOINTER strAttr,
                                  SQLSMALLINT bufferLen,
                                  SQLSMALLINT* strAttrLen, SQLLEN* numericAttr)
#else
SQLRETURN SQL_API SQLColAttribute(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                                  SQLUSMALLINT fieldId, SQLPOINTER strAttr,
                                  SQLSMALLINT bufferLen,
                                  SQLSMALLINT* strAttrLen,
                                  SQLPOINTER numericAttr)
#endif
{
  return trino::SQLColAttribute(stmt, columnNum, fieldId, strAttr,
                                     bufferLen, strAttrLen,
                                     (SQLLEN*)numericAttr);
}

SQLRETURN SQL_API SQLDescribeCol(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                                 SQLWCHAR* columnNameBuf,
                                 SQLSMALLINT columnNameBufLen,
                                 SQLSMALLINT* columnNameLen,
                                 SQLSMALLINT* dataType, SQLULEN* columnSize,
                                 SQLSMALLINT* decimalDigits,
                                 SQLSMALLINT* nullable) {
  return trino::SQLDescribeCol(stmt, columnNum, columnNameBuf,
                                    columnNameBufLen, columnNameLen, dataType,
                                    columnSize, decimalDigits, nullable);
}

SQLRETURN SQL_API SQLRowCount(SQLHSTMT stmt, SQLLEN* rowCnt) {
  return trino::SQLRowCount(stmt, rowCnt);
}

SQLRETURN SQL_API
SQLForeignKeys(SQLHSTMT stmt, SQLWCHAR* primaryCatalogName,
               SQLSMALLINT primaryCatalogNameLen, SQLWCHAR* primarySchemaName,
               SQLSMALLINT primarySchemaNameLen, SQLWCHAR* primaryTableName,
               SQLSMALLINT primaryTableNameLen, SQLWCHAR* foreignCatalogName,
               SQLSMALLINT foreignCatalogNameLen, SQLWCHAR* foreignSchemaName,
               SQLSMALLINT foreignSchemaNameLen, SQLWCHAR* foreignTableName,
               SQLSMALLINT foreignTableNameLen) {
  return trino::SQLForeignKeys(
      stmt, primaryCatalogName, primaryCatalogNameLen, primarySchemaName,
      primarySchemaNameLen, primaryTableName, primaryTableNameLen,
      foreignCatalogName, foreignCatalogNameLen, foreignSchemaName,
      foreignSchemaNameLen, foreignTableName, foreignTableNameLen);
}

SQLRETURN SQL_API SQLGetStmtAttr(SQLHSTMT stmt, SQLINTEGER attr,
                                 SQLPOINTER valueBuf, SQLINTEGER valueBufLen,
                                 SQLINTEGER* valueResLen) {
  return trino::SQLGetStmtAttr(stmt, attr, valueBuf, valueBufLen,
                                    valueResLen);
}

SQLRETURN SQL_API SQLSetStmtAttr(SQLHSTMT stmt, SQLINTEGER attr,
                                 SQLPOINTER value, SQLINTEGER valueLen) {
  return trino::SQLSetStmtAttr(stmt, attr, value, valueLen);
}

SQLRETURN SQL_API SQLPrimaryKeys(SQLHSTMT stmt, SQLWCHAR* catalogName,
                                 SQLSMALLINT catalogNameLen,
                                 SQLWCHAR* schemaName,
                                 SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                                 SQLSMALLINT tableNameLen) {
  return trino::SQLPrimaryKeys(stmt, catalogName, catalogNameLen,
                                    schemaName, schemaNameLen, tableName,
                                    tableNameLen);
}

SQLRETURN SQL_API SQLGetDiagField(SQLSMALLINT handleType, SQLHANDLE handle,
                                  SQLSMALLINT recNum, SQLSMALLINT diagId,
                                  SQLPOINTER buffer, SQLSMALLINT bufferLen,
                                  SQLSMALLINT* resLen) {
  return trino::SQLGetDiagField(handleType, handle, recNum, diagId, buffer,
                                     bufferLen, resLen);
}

SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT handleType, SQLHANDLE handle,
                                SQLSMALLINT recNum, SQLWCHAR* sqlState,
                                SQLINTEGER* nativeError, SQLWCHAR* msgBuffer,
                                SQLSMALLINT msgBufferLen, SQLSMALLINT* msgLen) {
  return trino::SQLGetDiagRec(handleType, handle, recNum, sqlState,
                                   nativeError, msgBuffer, msgBufferLen,
                                   msgLen);
}

SQLRETURN SQL_API SQLGetTypeInfo(SQLHSTMT stmt, SQLSMALLINT type) {
  return trino::SQLGetTypeInfo(stmt, type);
}

SQLRETURN SQL_API SQLGetData(SQLHSTMT stmt, SQLUSMALLINT colNum,
                             SQLSMALLINT targetType, SQLPOINTER targetValue,
                             SQLLEN bufferLength,
                             SQLLEN* strLengthOrIndicator) {
  return trino::SQLGetData(stmt, colNum, targetType, targetValue,
                                bufferLength, strLengthOrIndicator);
}

SQLRETURN SQL_API SQLSetEnvAttr(SQLHENV env, SQLINTEGER attr, SQLPOINTER value,
                                SQLINTEGER valueLen) {
  return trino::SQLSetEnvAttr(env, attr, value, valueLen);
}

SQLRETURN SQL_API SQLGetEnvAttr(SQLHENV env, SQLINTEGER attr,
                                SQLPOINTER valueBuf, SQLINTEGER valueBufLen,
                                SQLINTEGER* valueResLen) {
  return trino::SQLGetEnvAttr(env, attr, valueBuf, valueBufLen,
                                   valueResLen);
}

SQLRETURN SQL_API SQLSpecialColumns(
    SQLHSTMT stmt, SQLUSMALLINT idType, SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen,
    SQLWCHAR* tableName, SQLSMALLINT tableNameLen, SQLUSMALLINT scope,
    SQLUSMALLINT nullable) {
  return trino::SQLSpecialColumns(
      stmt, idType, catalogName, catalogNameLen, schemaName, schemaNameLen,
      tableName, tableNameLen, scope, nullable);
}

SQLRETURN SQL_API SQLStatistics(SQLHSTMT stmt, SQLWCHAR* catalogName,
                                SQLSMALLINT catalogNameLen,
                                SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen,
                                SQLWCHAR* tableName, SQLSMALLINT tableNameLen,
                                SQLUSMALLINT unique, SQLUSMALLINT reserved) {
  return trino::SQLStatistics(stmt, catalogName, catalogNameLen,
                                   schemaName, schemaNameLen, tableName,
                                   tableNameLen, unique, reserved);
}

SQLRETURN SQL_API SQLProcedureColumns(
    SQLHSTMT stmt, SQLWCHAR* catalogName, SQLSMALLINT catalogNameLen,
    SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen, SQLWCHAR* procName,
    SQLSMALLINT procNameLen, SQLWCHAR* columnName, SQLSMALLINT columnNameLen) {
  return trino::SQLProcedureColumns(
      stmt, catalogName, catalogNameLen, schemaName, schemaNameLen, procName,
      procNameLen, columnName, columnNameLen);
}

SQLRETURN SQL_API SQLProcedures(SQLHSTMT stmt, SQLWCHAR* catalogName,
                                SQLSMALLINT catalogNameLen,
                                SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen,
                                SQLWCHAR* tableName, SQLSMALLINT tableNameLen) {
  return trino::SQLProcedures(stmt, catalogName, catalogNameLen,
                                   schemaName, schemaNameLen, tableName,
                                   tableNameLen);
}

SQLRETURN SQL_API SQLError(SQLHENV env, SQLHDBC conn, SQLHSTMT stmt,
                           SQLWCHAR* state, SQLINTEGER* error, SQLWCHAR* msgBuf,
                           SQLSMALLINT msgBufLen, SQLSMALLINT* msgResLen) {
  return trino::SQLError(env, conn, stmt, state, error, msgBuf, msgBufLen,
                              msgResLen);
}

SQLRETURN SQL_API SQLGetConnectAttr(SQLHDBC conn, SQLINTEGER attr,
                                    SQLPOINTER valueBuf, SQLINTEGER valueBufLen,
                                    SQLINTEGER* valueResLen) {
  return trino::SQLGetConnectAttr(conn, attr, valueBuf, valueBufLen,
                                       valueResLen);
}

SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC conn, SQLINTEGER attr,
                                    SQLPOINTER value, SQLINTEGER valueLen) {
  return trino::SQLSetConnectAttr(conn, attr, value, valueLen);
}

SQLRETURN SQL_API SQLGetCursorName(SQLHSTMT stmt, SQLWCHAR* nameBuf,
                                   SQLSMALLINT nameBufLen,
                                   SQLSMALLINT* nameResLen) {
  return trino::SQLGetCursorName(stmt, nameBuf, nameBufLen, nameResLen);
}

SQLRETURN SQL_API SQLSetCursorName(SQLHSTMT stmt, SQLWCHAR* name,
                                   SQLSMALLINT nameLen) {
  return trino::SQLSetCursorName(stmt, name, nameLen);
}

SQLRETURN SQL_API SQLSetDescField(SQLHDESC descr, SQLSMALLINT recNum,
                                  SQLSMALLINT fieldId, SQLPOINTER buffer,
                                  SQLINTEGER bufferLen) {
  return trino::SQLSetDescField(descr, recNum, fieldId, buffer, bufferLen);
}

SQLRETURN SQL_API SQLGetDescField(SQLHDESC descr, SQLSMALLINT recNum,
                                  SQLSMALLINT fieldId, SQLPOINTER buffer,
                                  SQLINTEGER bufferLen, SQLINTEGER* resLen) {
  return trino::SQLGetDescField(descr, recNum, fieldId, buffer, bufferLen,
                                     resLen);
}

SQLRETURN SQL_API SQLCopyDesc(SQLHDESC src, SQLHDESC dst) {
  return trino::SQLCopyDesc(src, dst);
}

#if defined(__APPLE__)
// only for macOS as iODBC driver manger on BigSur does not
// implement this function
SQLRETURN SQL_API SQLGetFunctions(SQLHDBC conn, SQLUSMALLINT funcId,
                                  SQLUSMALLINT* valueBuf) {
  return trino::SQLGetFunctions(conn, funcId, valueBuf);
}
#endif

SQLRETURN SQL_API SQLSetConnectOption(SQLHDBC conn, SQLUSMALLINT option,
                                      SQLULEN value) {
  return trino::SQLSetConnectOption(conn, option, value);
}

SQLRETURN SQL_API SQLGetConnectOption(SQLHDBC conn, SQLUSMALLINT option,
                                      SQLPOINTER value) {
  return trino::SQLGetConnectOption(conn, option, value);
}

SQLRETURN SQL_API SQLGetStmtOption(SQLHSTMT stmt, SQLUSMALLINT option,
                                   SQLPOINTER value) {
  return trino::SQLGetStmtOption(stmt, option, value);
}

SQLRETURN SQL_API SQLColAttributes(SQLHSTMT stmt, SQLUSMALLINT colNum,
                                   SQLUSMALLINT fieldId, SQLPOINTER strAttrBuf,
                                   SQLSMALLINT strAttrBufLen,
                                   SQLSMALLINT* strAttrResLen,
                                   SQLLEN* numAttrBuf) {
  return trino::SQLColAttributes(stmt, colNum, fieldId, strAttrBuf,
                                      strAttrBufLen, strAttrResLen, numAttrBuf);
}

// ===================================================================================
// ==== Not implemented ====
// ===================================================================================

SQLRETURN SQL_API SQLSetStmtOption(SQLHSTMT stmt, SQLUSMALLINT option,
                                   SQLULEN value) {
  IGNITE_UNUSED(stmt);
  IGNITE_UNUSED(option);
  IGNITE_UNUSED(value);

  LOG_DEBUG_MSG("unsupported function SQLSetStmtOption called");

  STMT_UNSUPPORTED_FUNC(stmt, "SQLSetStmtOption is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLBrowseConnect(SQLHDBC conn, SQLWCHAR* inConnectionStr,
                                   SQLSMALLINT inConnectionStrLen,
                                   SQLWCHAR* outConnectionStrBuf,
                                   SQLSMALLINT outConnectionStrBufLen,
                                   SQLSMALLINT* outConnectionStrResLen) {
  IGNITE_UNUSED(conn);
  IGNITE_UNUSED(inConnectionStr);
  IGNITE_UNUSED(inConnectionStrLen);
  IGNITE_UNUSED(outConnectionStrBuf);
  IGNITE_UNUSED(outConnectionStrBufLen);
  IGNITE_UNUSED(outConnectionStrResLen);

  LOG_DEBUG_MSG("unsupported function SQLBrowseConnect called");

  CONN_UNSUPPORTED_FUNC(conn, "SQLBrowseConnect is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLSetPos(SQLHSTMT stmt, SQLSETPOSIROW rowNum,
                            SQLUSMALLINT operation, SQLUSMALLINT lockType) {
  IGNITE_UNUSED(rowNum);
  IGNITE_UNUSED(operation);
  IGNITE_UNUSED(lockType);

  LOG_DEBUG_MSG("unsupported function SQLSetPos called");

  STMT_UNSUPPORTED_FUNC(stmt, "SQLSetPos is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLSetScrollOptions(SQLHSTMT stmt, SQLUSMALLINT concurrency,
                                      SQLLEN crowKeyset,
                                      SQLUSMALLINT crowRowset) {
  IGNITE_UNUSED(stmt);
  IGNITE_UNUSED(concurrency);
  IGNITE_UNUSED(crowKeyset);
  IGNITE_UNUSED(crowRowset);

  LOG_DEBUG_MSG("unsupported function SQLSetScrollOptions called");

  STMT_UNSUPPORTED_FUNC(stmt, "SQLSetScrollOptions is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLBulkOperations(SQLHSTMT stmt, SQLUSMALLINT operation) {
  IGNITE_UNUSED(operation);

  LOG_DEBUG_MSG("Unsupported function SQLBulkOperations called");

  STMT_UNSUPPORTED_FUNC(stmt, "SQLBulkOperations is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLEndTran(SQLSMALLINT handleType, SQLHANDLE handle,
                             SQLSMALLINT completionType) {
  IGNITE_UNUSED(completionType);

  LOG_DEBUG_MSG("Unsupported function SQLEndTran called");

  if (handleType == SQL_HANDLE_ENV) {
    ENV_UNSUPPORTED_FUNC(handle, "SQLEndTran is not supported.");
  } else if (handleType == SQL_HANDLE_DBC) {
    CONN_UNSUPPORTED_FUNC(handle, "SQLEndTran is not supported.");
  }

  return SQL_ERROR;
}

SQLRETURN SQL_API SQLGetDescRec(SQLHDESC desc, SQLSMALLINT RecNumber,
                                SQLWCHAR* nameBuffer, SQLSMALLINT nameBufferLen,
                                SQLSMALLINT* strLen, SQLSMALLINT* type,
                                SQLSMALLINT* subType, SQLLEN* len,
                                SQLSMALLINT* precision, SQLSMALLINT* scale,
                                SQLSMALLINT* nullable) {
  IGNITE_UNUSED(RecNumber);
  IGNITE_UNUSED(nameBuffer);
  IGNITE_UNUSED(nameBufferLen);
  IGNITE_UNUSED(strLen);
  IGNITE_UNUSED(type);
  IGNITE_UNUSED(subType);
  IGNITE_UNUSED(len);
  IGNITE_UNUSED(precision);
  IGNITE_UNUSED(scale);
  IGNITE_UNUSED(nullable);

  LOG_DEBUG_MSG("unsupported function SQLGetDescRec called");

  DESC_UNSUPPORTED_FUNC(desc, "SQLGetDescRec is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLSetDescRec(SQLHDESC desc, SQLSMALLINT recNum,
                                SQLSMALLINT type, SQLSMALLINT subType,
                                SQLLEN len, SQLSMALLINT precision,
                                SQLSMALLINT scale, SQLPOINTER buffer,
                                SQLLEN* resLen, SQLLEN* id) {
  IGNITE_UNUSED(recNum);
  IGNITE_UNUSED(type);
  IGNITE_UNUSED(subType);
  IGNITE_UNUSED(len);
  IGNITE_UNUSED(precision);
  IGNITE_UNUSED(scale);
  IGNITE_UNUSED(buffer);
  IGNITE_UNUSED(resLen);
  IGNITE_UNUSED(id);

  LOG_DEBUG_MSG("unsupported function SQLSetDescRec called");

  DESC_UNSUPPORTED_FUNC(desc, "SQLSetDescRec is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLBindParameter(SQLHSTMT stmt, SQLUSMALLINT paramIdx,
                                   SQLSMALLINT ioType, SQLSMALLINT bufferType,
                                   SQLSMALLINT paramSqlType, SQLULEN columnSize,
                                   SQLSMALLINT decDigits, SQLPOINTER buffer,
                                   SQLLEN bufferLen, SQLLEN* resLen) {
  IGNITE_UNUSED(paramIdx);
  IGNITE_UNUSED(ioType);
  IGNITE_UNUSED(bufferType);
  IGNITE_UNUSED(paramSqlType);
  IGNITE_UNUSED(columnSize);
  IGNITE_UNUSED(decDigits);
  IGNITE_UNUSED(buffer);
  IGNITE_UNUSED(bufferLen);
  IGNITE_UNUSED(resLen);

  LOG_DEBUG_MSG("unsupported function SQLBindParameter called");

  STMT_UNSUPPORTED_FUNC(stmt, "SQLBindParameter is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLDescribeParam(SQLHSTMT stmt, SQLUSMALLINT paramNum,
                                   SQLSMALLINT* dataType, SQLULEN* paramSize,
                                   SQLSMALLINT* decimalDigits,
                                   SQLSMALLINT* nullable) {
  IGNITE_UNUSED(paramNum);
  IGNITE_UNUSED(dataType);
  IGNITE_UNUSED(paramSize);
  IGNITE_UNUSED(decimalDigits);
  IGNITE_UNUSED(nullable);

  LOG_DEBUG_MSG("unsupported function SQLDescribeParam called");

  STMT_UNSUPPORTED_FUNC(stmt, "SQLDescribeParam is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLParamData(SQLHSTMT stmt, SQLPOINTER* value) {
  IGNITE_UNUSED(value);

  LOG_DEBUG_MSG("unsupported function SQLParamData called");

  STMT_UNSUPPORTED_FUNC(stmt, "SQLParamData is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLParamOptions(SQLHSTMT stmt, SQLULEN paramSetSize,
                                  SQLULEN* paramsProcessed) {
  IGNITE_UNUSED(stmt);
  IGNITE_UNUSED(paramSetSize);
  IGNITE_UNUSED(paramsProcessed);

  LOG_DEBUG_MSG("unsupported function SQLParamOptions called");

  STMT_UNSUPPORTED_FUNC(stmt, "SQLParamOptions is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLNumParams(SQLHSTMT stmt, SQLSMALLINT* paramCnt) {
  IGNITE_UNUSED(paramCnt);

  LOG_DEBUG_MSG("unsupported function SQLNumParams called");

  STMT_UNSUPPORTED_FUNC(stmt, "SQLNumParams is not supported.");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLPutData(SQLHSTMT stmt, SQLPOINTER data,
                             SQLLEN strLengthOrIndicator) {
  IGNITE_UNUSED(data);
  IGNITE_UNUSED(strLengthOrIndicator);

  LOG_DEBUG_MSG("unsupported function SQLPutData called");

  STMT_UNSUPPORTED_FUNC(stmt, "SQLPutData is not supported.");
  return SQL_ERROR;
}
