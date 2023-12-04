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

#ifndef _TRINO_ODBC_STATEMENT
#define _TRINO_ODBC_STATEMENT

#include <stdint.h>

#include <map>
#include <memory>

#include "trino/odbc/app/application_data_buffer.h"
#include "trino/odbc/common_types.h"
#include "trino/odbc/diagnostic/diagnosable_adapter.h"
#include "trino/odbc/meta/column_meta.h"
#include "trino/odbc/query/query.h"
#include "trino/odbc/descriptor.h"

using trino::odbc::query::Query;

namespace trino {
namespace odbc {
struct StatementAttributes;
class Connection;

/**
 * SQL-statement abstraction. Holds SQL query user buffers data and
 * call result.
 */
class IGNITE_IMPORT_EXPORT Statement : public diagnostic::DiagnosableAdapter {
  friend class Connection;

 public:
  /**
   * Destructor.
   */
  ~Statement();

  /**
   * Bind result column to data buffer provided by application
   *
   * @param columnIdx Column index.
   * @param targetType Type of target buffer.
   * @param targetValue Pointer to target buffer.
   * @param bufferLength Length of target buffer.
   * @param strLengthOrIndicator Pointer to the length/indicator buffer.
   */
  void BindColumn(uint16_t columnIdx, int16_t targetType, void* targetValue,
                  SqlLen bufferLength, SqlLen* strLengthOrIndicator);

  /**
   * Set column binding offset pointer.
   *
   * @param ptr Column binding offset pointer.
   */
  void SetColumnBindOffsetPtr(int* ptr);

  /**
   * Get column binding offset pointer.
   *
   * @return Column binding offset pointer.
   */
  int* GetColumnBindOffsetPtr();

  /**
   * Get number of columns in the result set.
   *
   * @return Columns number.
   */
  int32_t GetColumnNumber();

  /**
   * Set statement attribute.
   *
   * @param attr Attribute type.
   * @param value Value pointer.
   * @param valueLen Value length.
   */
  void SetAttribute(int attr, void* value, SQLINTEGER valueLen);

  /**
   * Set statement attribute.
   *
   * @param stmtAttr StatementAttributes struct.
   */
  void SetAttribute(StatementAttributes& stmtAttr);

  /**
   * Get statement attribute.
   *
   * @param attr Attribute type.
   * @param buf Buffer for value.
   * @param bufLen Buffer length.
   * @param valueLen Resulting value length.
   */
  void GetAttribute(int attr, void* buf, SQLINTEGER bufLen,
                    SQLINTEGER* valueLen);

  /**
   * Get statement option.
   *
   * @param option Option type.
   * @param value Option value.
   */
  void GetStmtOption(SQLUSMALLINT option, SQLPOINTER value);

  /**
   * Get value of the column in the result set.
   *
   * @param columnIdx Column index.
   * @param buffer Buffer to put column data to.
   */
  void GetColumnData(uint16_t columnIdx, app::ApplicationDataBuffer& buffer);

  /**
   * Prepare SQL query.
   *
   * @param query SQL query.
   */
  void PrepareSqlQuery(const std::string& query);

  /**
   * Execute SQL query.
   *
   * @param query SQL query.
   */
  void ExecuteSqlQuery(const std::string& query);

  /**
   * Execute SQL query.
   */
  void ExecuteSqlQuery();

  /**
   * Cancel SQL query.
   */
  void CancelSqlQuery();

  /**
   * Get columns metadata.
   *
   * @param catalog Catalog search pattern, nullable.
   * @param schema Schema search pattern, nullable.
   * @param table Table search pattern, nullable.
   * @param column Column search pattern, nullable.
   */
  void ExecuteGetColumnsMetaQuery(const boost::optional< std::string >& catalog,
                                  const boost::optional< std::string >& schema,
                                  const boost::optional< std::string >& table,
                                  const boost::optional< std::string >& column);

  /**
   * Get tables metadata.
   *
   * @param catalog Catalog search pattern, nullable.
   * @param schema Schema search pattern, nullable.
   * @param table Table search pattern.
   * @param tableType Table type search pattern, nullable.
   */
  void ExecuteGetTablesMetaQuery(
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schema,
      const boost::optional< std::string >& table,
      const boost::optional< std::string >& tableType);

  /**
   * Get foreign keys. Empty result will be returned.
   */
  void ExecuteGetForeignKeysQuery();

  /**
   * Get primary keys. Empty result will be returned.
   */
  void ExecuteGetPrimaryKeysQuery();

  /**
   * Get special columns. Empty result will be returned.
   */
  void ExecuteSpecialColumnsQuery();

  /**
   * Get statistics. Empty result will be returned.
   */
  void ExecuteStatisticsQuery();

  /**
   * Get procedure columns. Empty result will be returned.
   */
  void ExecuteProcedureColumnsQuery();

  /**
   * Get procedures. Empty result will be returned.
   */
  void ExecuteProceduresQuery();

  /**
   * Get column privileges. Empty result will be returned.
   */
  void ExecuteColumnPrivilegesQuery();

  /**
   * Get table privileges. Empty result will be returned.
   */
  void ExecuteTablePrivilegesQuery();

  /**
   * Get type info.
   *
   * @param sqlType SQL type for which to return info or SQL_ALL_TYPES.
   */
  void ExecuteGetTypeInfoQuery(int16_t sqlType);

  /**
   * Free resources
   * @param option indicates what needs to be freed
   */
  void FreeResources(int16_t option);

  /**
   * Close statement.
   */
  void Close();

  /**
   * Fetch query result row with offset
   * @param orientation Fetch type
   * @param offset Fetch offset
   */
  void FetchScroll(int16_t orientation, int64_t offset);

  /**
   * Fetch query result row.
   */
  void FetchRow();

  /**
   * Get column metadata.
   *
   * @return Column metadata.
   */
  const meta::ColumnMetaVector* GetMeta();

  /**
   * Check if data is available.
   *
   * @return True if data is available.
   */
  bool DataAvailable() const;

  /**
   * More results.
   *
   * Move to next result set or affected rows number.
   */
  void MoreResults();

  /**
   * Get column attribute.
   *
   * @param colIdx Column index.
   * @param attrId Attribute ID.
   * @param strbuf Buffer for string attribute value.
   * @param buflen String buffer size.
   * @param reslen Buffer to put resulting string length to.
   * @param numbuf Numeric value buffer.
   */
  void GetColumnAttribute(uint16_t colIdx, uint16_t attrId, SQLWCHAR* strbuf,
                          int16_t buflen, int16_t* reslen, SqlLen* numbuf);

  /**
   * Get number of rows affected by the statement.
   *
   * @return Number of rows affected by the statement.
   */
  int64_t AffectedRows();

  /**
   * Set rows fetched buffer pointer.
   *
   * @param ptr Rows fetched buffer pointer.
   */
  void SetRowsFetchedPtr(SQLULEN* ptr);

  /**
   * Get rows fetched buffer pointer.
   *
   * @return Rows fetched buffer pointer.
   */
  SQLULEN* GetRowsFetchedPtr();

  /**
   * Set row statuses array pointer.
   *
   * @param ptr Row statuses array pointer.
   */
  void SetRowStatusesPtr(SQLUSMALLINT* ptr);

  /**
   * Get row statuses array pointer.
   *
   * @return Row statuses array pointer.
   */
  SQLUSMALLINT* GetRowStatusesPtr();

  /**
   * Set current active ARD descriptor
   *
   * @param desc Descriptor pointer.
   */
  void SetARDDesc(Descriptor* desc);

  /**
   * Get cursor name.
   *
   * @param nameBuf Buffer pointer to hold the returned cursor name.
   * @param nameBufLen Cursor name buffer length.
   * @param nameResLen Returned cursor name result length.
   */
  void GetCursorName(SQLWCHAR* nameBuf, SQLSMALLINT nameBufLen,
                     SQLSMALLINT* nameResLen);

  /**
   * Set cursor name.
   *
   * @param name Cursor name.
   * @param nameLen Cursor name length.
   */
  void SetCursorName(SQLWCHAR* name, SQLSMALLINT nameLen);

  /**
   * Get connection that creates the statement
   *
   * @return connection object reference.
   */
  Connection& GetConnection() {
    return connection;
  }

  /**
   * Unbind specified column buffer.
   *
   * @param columnIdx Column index.
   */
  void SafeUnbindColumn(uint16_t columnIdx);

  /**
   * Restore the descriptor to be implicit created one
   *
   * @param type Descriptor type that will be restored.
   */
  void RestoreDescriptor(DescType type);

 protected:
  /**
   * Constructor.
   * Called by friend classes.
   *
   * @param parent Connection associated with the statement.
   */
  Statement(Connection& parent);

 private:
  IGNITE_NO_COPY_ASSIGNMENT(Statement);

  /**
   * Bind result column to specified data buffer.
   *
   * @param columnIdx Column index.
   * @param buffer Buffer to put column data to.
   */
  void SafeBindColumn(uint16_t columnIdx,
                      const app::ApplicationDataBuffer& buffer);

  /**
   * Unbind all column buffers.
   */
  void SafeUnbindAllColumns();

  /**
   * Bind result column to data buffer provided by application
   *
   * @param columnIdx Column index.
   * @param targetType Type of target buffer.
   * @param targetValue Pointer to target buffer.
   * @param bufferLength Length of target buffer.
   * @param strLengthOrIndicator Pointer to the length/indicator buffer.
   * @return Operation result.
   */
  SqlResult::Type InternalBindColumn(uint16_t columnIdx, int16_t targetType,
                                     void* targetValue, SqlLen bufferLength,
                                     SqlLen* strLengthOrIndicator);

  /**
   * Set descriptor field when InternalBindColumn is executed
   *
   * @param columnIdx Column index.
   * @param targetType Type of target buffer.
   * @param targetValue Pointer to target buffer.
   * @param bufferLength Length of target buffer.
   * @param strLengthOrIndicator Pointer to the length/indicator buffer.
   */
  void SetDescriptorFields(uint16_t columnIdx, int16_t targetType,
                           void* targetValue, SqlLen bufferLength,
                           SqlLen* strLengthOrIndicator);

  /**
   * Set statement attribute.
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
   * Get statement attribute.
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
   * Get statement option.
   * Internal call.
   *
   * @param option Option type.
   * @param value Option value.
   * @return Operation result.
   */
  SqlResult::Type InternalGetStmtOption(SQLUSMALLINT option, SQLPOINTER value);

  /**
   * Get number parameters required by the prepared statement.
   *
   * @param paramNum Number of parameters.
   */
  SqlResult::Type InternalGetParametersNumber(uint16_t& paramNum);

  /**
   * Get value of the column in the result set.
   *
   * @param columnIdx Column index.
   * @param buffer Buffer to put column data to.
   * @return Operation result.
   */
  SqlResult::Type InternalGetColumnData(uint16_t columnIdx,
                                        app::ApplicationDataBuffer& buffer);

  /**
   * Free resources
   * @param option indicates what needs to be freed
   * @return Operation result.
   */
  SqlResult::Type InternalFreeResources(int16_t option);

  /**
   * Close statement.
   * Internal call.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalClose();

  /**
   * Process internal SQL command.
   *
   * @param query SQL query.
   * @return Operation result.
   */
  SqlResult::Type ProcessInternalCommand(const std::string& query);

  /**
   * Prepare SQL query.
   *
   * @param query SQL query.
   * @return Operation result.
   */
  SqlResult::Type InternalPrepareSqlQuery(const std::string& query);

  /**
   * Execute SQL query.
   *
   * @param query SQL query.
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteSqlQuery(const std::string& query);

  /**
   * Execute SQL query.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteSqlQuery();

  /**
   * Cancel SQL query.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalCancelSqlQuery();

  /**
   * Fetch query result row with offset
   * @param orientation Fetch type
   * @param offset Fetch offset
   * @return Operation result.
   */
  SqlResult::Type InternalFetchScroll(int16_t orientation, int64_t offset);

  /**
   * Fetch query result row.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalFetchRow();

  /**
   * Get number of columns in the result set.
   *
   * @param res Columns number.
   * @return Operation result.
   */
  SqlResult::Type InternalGetColumnNumber(int32_t& res);

  /**
   * Get columns metadata.
   *
   * @param catalog Catalog search pattern.
   * @param schema Schema search pattern.
   * @param table Table search pattern.
   * @param column Column search pattern.
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteGetColumnsMetaQuery(
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schema,
      const boost::optional< std::string >& table,
      const boost::optional< std::string >& column);

  /**
   * Get tables metadata.
   *
   * @param catalog Catalog search pattern, nullable.
   * @param schema Schema search pattern, nullable.
   * @param table Table search pattern.
   * @param tableType Table type search pattern, nullable.
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteGetTablesMetaQuery(
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schema,
      const boost::optional< std::string >& table,
      const boost::optional< std::string >& tableType);

  /**
   * Get foreign keys. Empty result will be returned.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteGetForeignKeysQuery();

  /**
   * Get primary keys. Empty result will be returned.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteGetPrimaryKeysQuery();

  /**
   * Get special columns. Empty result will be returned.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteSpecialColumnsQuery();

  /**
   * Get statistics. Empty result will be returned.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteStatisticsQuery();

  /**
   * Get procedure columns. Empty result will be returned.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteProcedureColumnsQuery();

  /**
   * Get procedures. Empty result will be returned.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteProceduresQuery();

  /**
   * Get column privileges. Empty result will be returned.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteColumnPrivilegesQuery();

  /**
   * Get table privileges. Empty result will be returned.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteTablePrivilegesQuery();

  /**
   * Get type info.
   *
   * @param sqlType SQL type for which to return info or SQL_ALL_TYPES.
   * @return Operation result.
   */
  SqlResult::Type InternalExecuteGetTypeInfoQuery(int16_t sqlType);

  /**
   * Next results.
   *
   * Move to next result set or affected rows number.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalMoreResults();

  /**
   * Get column attribute.
   *
   * @param colIdx Column index.
   * @param attrId Attribute ID.
   * @param strbuf Buffer for string attribute value.
   * @param buflen String buffer size.
   * @param reslen Buffer to put resulting string length to.
   * @param numbuf Numeric value buffer.
   * @return Operation result.
   */
  SqlResult::Type InternalGetColumnAttribute(uint16_t colIdx, uint16_t attrId,
                                             SQLWCHAR* strbuf, int16_t buflen,
                                             int16_t* reslen, SqlLen* numbuf);

  /**
   * Get number of rows affected by the statement.
   *
   * @param rowCnt Number of rows affected by the statement.
   * @return Operation result.
   */
  SqlResult::Type InternalAffectedRows(int64_t& rowCnt);

  SqlResult::Type InternalGetCursorName(SQLWCHAR* nameBuf,
                                        SQLSMALLINT nameBufLen,
                                        SQLSMALLINT* nameResLen);

  SqlResult::Type InternalSetCursorName(SQLWCHAR* name, SQLSMALLINT nameLen);

  /**
   * Convert SQLRESULT to SQL_ROW_RESULT.
   *
   * @return Operation result.
   */
  uint16_t SqlResultToRowResult(SqlResult::Type value);

  /** Connection associated with the statement. */
  Connection& connection;

  /** Column bindings. */
  app::ColumnBindingMap columnBindings;

  /** Underlying query. */
  std::unique_ptr< Query > currentQuery;

  /** Buffer to store number of rows fetched by the last fetch. */
  SQLULEN* rowsFetched;

  /** Array to store statuses of rows fetched by the last fetch. */
  SQLUSMALLINT* rowStatuses;

  /** Offset added to pointers to change binding of column data. */
  int* columnBindOffset;

  /** Row array size. */
  SqlUlen rowArraySize;

  /** implicitly allocated ARD */
  std::unique_ptr< Descriptor > ardi;

  /** implicitly allocated APD(not in use now) */
  std::unique_ptr< Descriptor > apdi;

  /** implicitly allocated IRD(not in use now) */
  std::unique_ptr< Descriptor > irdi;

  /** implicitly allocated IPD(not in use now) */
  std::unique_ptr< Descriptor > ipdi;

  /** ARD current in use */
  Descriptor* ard;

  /** IRD current in use */
  Descriptor* ird;
};
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_STATEMENT
