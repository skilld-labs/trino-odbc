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

#ifndef _TIMESTREAM_ODBC_ODBC
#define _TIMESTREAM_ODBC_ODBC

#include "timestream/odbc/system/odbc_constants.h"
#include "timestream/odbc/utility.h"

/**
 * @file odbc.h
 *
 * Functions here are placed to the timestream namespace so there are no
 * collisions with standard ODBC functions when we call driver API
 * functions from other API functions. I.e, when we call SQLAllocEnv
 * from SQLAllocHandle linker can place Driver Manager call here,
 * instead of internal driver call. On other hand if we call
 * timestream::SQLAllocEnv from timestream::SQLAllocHandle we can be sure
 * there are no collisions.
 */

namespace timestream {
SQLRETURN SQLGetInfo(SQLHDBC conn, SQLUSMALLINT infoType, SQLPOINTER infoValue,
                     SQLSMALLINT infoValueMax, SQLSMALLINT* length);

SQLRETURN SQLAllocHandle(SQLSMALLINT type, SQLHANDLE parent, SQLHANDLE* result);

SQLRETURN SQLAllocEnv(SQLHENV* env);

SQLRETURN SQLAllocConnect(SQLHENV env, SQLHDBC* conn);

SQLRETURN SQLAllocStmt(SQLHDBC conn, SQLHSTMT* stmt);

SQLRETURN SQLAllocDesc(SQLHDBC conn, SQLHDESC* desc);

SQLRETURN SQLFreeHandle(SQLSMALLINT type, SQLHANDLE handle);

SQLRETURN SQLFreeEnv(SQLHENV env);

SQLRETURN SQLFreeConnect(SQLHDBC conn);

SQLRETURN SQLFreeStmt(SQLHSTMT stmt, SQLUSMALLINT option);

SQLRETURN SQLFreeDescriptor(SQLHDESC desc);

SQLRETURN SQLCloseCursor(SQLHSTMT stmt);

SQLRETURN SQLDriverConnect(SQLHDBC conn, SQLHWND windowHandle,
                           SQLWCHAR* inConnectionString,
                           SQLSMALLINT inConnectionStringLen,
                           SQLWCHAR* outConnectionString,
                           SQLSMALLINT outConnectionStringBufferLen,
                           SQLSMALLINT* outConnectionStringLen,
                           SQLUSMALLINT driverCompletion);

SQLRETURN SQLConnect(SQLHDBC conn, SQLWCHAR* serverName,
                     SQLSMALLINT serverNameLen, SQLWCHAR* userName,
                     SQLSMALLINT userNameLen, SQLWCHAR* auth,
                     SQLSMALLINT authLen);

SQLRETURN SQLDisconnect(SQLHDBC conn);

SQLRETURN SQLPrepare(SQLHSTMT stmt, SQLWCHAR* query, SQLINTEGER queryLen);

SQLRETURN SQLExecute(SQLHSTMT stmt);

SQLRETURN SQLExecDirect(SQLHSTMT stmt, SQLWCHAR* query, SQLINTEGER queryLen);

SQLRETURN SQLCancel(SQLHSTMT stmt);

SQLRETURN SQLBindCol(SQLHSTMT stmt, SQLUSMALLINT colNum, SQLSMALLINT targetType,
                     SQLPOINTER targetValue, SQLLEN bufferLength,
                     SQLLEN* strLengthOrIndicator);

SQLRETURN SQLFetch(SQLHSTMT stmt);

SQLRETURN SQLFetchScroll(SQLHSTMT stmt, SQLSMALLINT orientation, SQLLEN offset);

SQLRETURN SQLExtendedFetch(SQLHSTMT stmt, SQLUSMALLINT orientation,
                           SQLLEN offset, SQLULEN* rowCount,
                           SQLUSMALLINT* rowStatusArray);

SQLRETURN SQLNumResultCols(SQLHSTMT stmt, SQLSMALLINT* columnNum);

SQLRETURN SQLTables(SQLHSTMT stmt, SQLWCHAR* catalogName,
                    SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                    SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                    SQLSMALLINT tableNameLen, SQLWCHAR* tableType,
                    SQLSMALLINT tableTypeLen);

SQLRETURN SQLTablePrivileges(SQLHSTMT stmt, SQLWCHAR* catalogName,
                             SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                             SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                             SQLSMALLINT tableNameLen);

SQLRETURN SQLColumns(SQLHSTMT stmt, SQLWCHAR* catalogName,
                     SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                     SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                     SQLSMALLINT tableNameLen, SQLWCHAR* columnName,
                     SQLSMALLINT columnNameLen);

SQLRETURN SQLColumnPrivileges(SQLHSTMT stmt, SQLWCHAR* catalogName,
                              SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                              SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                              SQLSMALLINT tableNameLen, SQLWCHAR* columnName,
                              SQLSMALLINT columnNameLen);

SQLRETURN SQLMoreResults(SQLHSTMT stmt);

SQLRETURN SQLNativeSql(SQLHDBC conn, SQLWCHAR* inQuery, SQLINTEGER inQueryLen,
                       SQLWCHAR* outQueryBuffer, SQLINTEGER outQueryBufferLen,
                       SQLINTEGER* outQueryLen);

SQLRETURN SQLColAttribute(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                          SQLUSMALLINT fieldId, SQLPOINTER strAttr,
                          SQLSMALLINT bufferLen, SQLSMALLINT* strAttrLen,
                          SQLLEN* numericAttr);

SQLRETURN SQLDescribeCol(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                         SQLWCHAR* columnNameBuf, SQLSMALLINT columnNameBufLen,
                         SQLSMALLINT* columnNameLen, SQLSMALLINT* dataType,
                         SQLULEN* columnSize, SQLSMALLINT* decimalDigits,
                         SQLSMALLINT* nullable);

SQLRETURN SQLRowCount(SQLHSTMT stmt, SQLLEN* rowCnt);

SQLRETURN SQLForeignKeys(
    SQLHSTMT stmt, SQLWCHAR* primaryCatalogName,
    SQLSMALLINT primaryCatalogNameLen, SQLWCHAR* primarySchemaName,
    SQLSMALLINT primarySchemaNameLen, SQLWCHAR* primaryTableName,
    SQLSMALLINT primaryTableNameLen, SQLWCHAR* foreignCatalogName,
    SQLSMALLINT foreignCatalogNameLen, SQLWCHAR* foreignSchemaName,
    SQLSMALLINT foreignSchemaNameLen, SQLWCHAR* foreignTableName,
    SQLSMALLINT foreignTableNameLen);

SQLRETURN SQLGetStmtAttr(SQLHSTMT stmt, SQLINTEGER attr, SQLPOINTER valueBuf,
                         SQLINTEGER valueBufLen, SQLINTEGER* valueResLen);

SQLRETURN SQLSetStmtAttr(SQLHSTMT stmt, SQLINTEGER attr, SQLPOINTER value,
                         SQLINTEGER valueLen);

SQLRETURN SQLPrimaryKeys(SQLHSTMT stmt, SQLWCHAR* catalogName,
                         SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                         SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                         SQLSMALLINT tableNameLen);

SQLRETURN SQLGetDiagField(SQLSMALLINT handleType, SQLHANDLE handle,
                          SQLSMALLINT recNum, SQLSMALLINT diagId,
                          SQLPOINTER buffer, SQLSMALLINT bufferLen,
                          SQLSMALLINT* resLen);

SQLRETURN SQLGetDiagRec(SQLSMALLINT handleType, SQLHANDLE handle,
                        SQLSMALLINT recNum, SQLWCHAR* sqlState,
                        SQLINTEGER* nativeError, SQLWCHAR* msgBuffer,
                        SQLSMALLINT msgBufferLen, SQLSMALLINT* msgLen);

SQLRETURN SQLGetTypeInfo(SQLHSTMT stmt, SQLSMALLINT type);

SQLRETURN SQLGetData(SQLHSTMT stmt, SQLUSMALLINT colNum, SQLSMALLINT targetType,
                     SQLPOINTER targetValue, SQLLEN bufferLength,
                     SQLLEN* strLengthOrIndicator);

SQLRETURN SQLSetEnvAttr(SQLHENV env, SQLINTEGER attr, SQLPOINTER value,
                        SQLINTEGER valueLen);

SQLRETURN SQLGetEnvAttr(SQLHENV env, SQLINTEGER attr, SQLPOINTER valueBuf,
                        SQLINTEGER valueBufLen, SQLINTEGER* valueResLen);

SQLRETURN SQLSpecialColumns(SQLHSTMT stmt, SQLSMALLINT idType,
                            SQLWCHAR* catalogName, SQLSMALLINT catalogNameLen,
                            SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen,
                            SQLWCHAR* tableName, SQLSMALLINT tableNameLen,
                            SQLSMALLINT scope, SQLSMALLINT nullable);

SQLRETURN SQLStatistics(SQLHSTMT stmt, SQLWCHAR* catalogName,
                        SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                        SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                        SQLSMALLINT tableNameLen, SQLUSMALLINT unique,
                        SQLUSMALLINT reserved);

SQLRETURN SQLProcedureColumns(SQLHSTMT stmt, SQLWCHAR* catalogName,
                              SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                              SQLSMALLINT schemaNameLen, SQLWCHAR* procName,
                              SQLSMALLINT procNameLen, SQLWCHAR* columnName,
                              SQLSMALLINT columnNameLen);

SQLRETURN SQLProcedures(SQLHSTMT stmt, SQLWCHAR* catalogName,
                        SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                        SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                        SQLSMALLINT tableNameLen);

SQLRETURN SQLError(SQLHENV env, SQLHDBC conn, SQLHSTMT stmt, SQLWCHAR* state,
                   SQLINTEGER* error, SQLWCHAR* msgBuf, SQLSMALLINT msgBufLen,
                   SQLSMALLINT* msgResLen);

SQLRETURN SQLGetConnectAttr(SQLHDBC conn, SQLINTEGER attr, SQLPOINTER valueBuf,
                            SQLINTEGER valueBufLen, SQLINTEGER* valueResLen);

SQLRETURN SQLSetConnectAttr(SQLHDBC conn, SQLINTEGER attr, SQLPOINTER value,
                            SQLINTEGER valueLen);

SQLRETURN SQLGetCursorName(SQLHSTMT stmt, SQLWCHAR* nameBuf,
                           SQLSMALLINT nameBufLen, SQLSMALLINT* nameResLen);

SQLRETURN SQLSetCursorName(SQLHSTMT stmt, SQLWCHAR* name, SQLSMALLINT nameLen);

SQLRETURN SQLSetDescField(SQLHDESC descr, SQLSMALLINT recNum,
                          SQLSMALLINT fieldId, SQLPOINTER buffer,
                          SQLINTEGER bufferLen);

SQLRETURN SQLGetDescField(SQLHDESC descr, SQLSMALLINT recNum,
                          SQLSMALLINT fieldId, SQLPOINTER buffer,
                          SQLINTEGER bufferLen, SQLINTEGER* resLen);

SQLRETURN SQLCopyDesc(SQLHDESC src, SQLHDESC dst);

#if defined(__APPLE__)
SQLRETURN SQL_API SQLGetFunctions(SQLHDBC conn, SQLUSMALLINT funcId,
                                  SQLUSMALLINT* valueBuf);
#endif

SQLRETURN SQLSetConnectOption(SQLHDBC conn, SQLUSMALLINT option, SQLULEN value);

SQLRETURN SQLGetConnectOption(SQLHDBC conn, SQLUSMALLINT option,
                              SQLPOINTER value);

SQLRETURN SQLGetStmtOption(SQLHSTMT stmt, SQLUSMALLINT option,
                           SQLPOINTER value);

SQLRETURN SQLColAttributes(SQLHSTMT stmt, SQLUSMALLINT colNum,
                           SQLUSMALLINT fieldId, SQLPOINTER strAttrBuf,
                           SQLSMALLINT strAttrBufLen,
                           SQLSMALLINT* strAttrResLen, SQLLEN* numAttrBuf);
}  // namespace timestream

#endif  //_TIMESTREAM_ODBC
