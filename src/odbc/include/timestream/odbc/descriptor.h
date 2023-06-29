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

#ifndef _TIMESTREAM_ODBC_DESCRIPTOR
#define _TIMESTREAM_ODBC_DESCRIPTOR

#include <map>

#include <ignite/common/common.h>

#include "timestream/odbc/common_types.h"
#include "timestream/odbc/diagnostic/diagnosable_adapter.h"

namespace timestream {
namespace odbc {

class Connection;
class Statement;

/* Descriptor header struct */
struct DescriptorHeader {
  SQLSMALLINT allocType;
  SQLULEN arraySize;
  SQLUSMALLINT* arrayStatusPtr;
  SQLLEN* bindOffsetPtr;
  SQLINTEGER bindType;
  SQLSMALLINT count;
  SQLULEN* rowsProcessedPtr;
};

/* Descriptor record struct */
struct DescriptorRecord {
  SQLINTEGER autoUniqueValue;
  SQLCHAR* baseColumnName;
  SQLCHAR* baseTableName;
  SQLINTEGER caseSensitive;
  SQLCHAR* catalogName;
  SQLSMALLINT conciseType;
  SQLPOINTER dataPtr;
  SQLSMALLINT datetimeIntervalCode;
  SQLINTEGER datetimeIntervalPrecision;
  SQLLEN displaySize;
  SQLSMALLINT fixedPrecScale;
  SQLLEN* indicatorPtr;
  SQLCHAR* label;
  SQLULEN length;
  SQLCHAR* literalPrefix;
  SQLCHAR* literalSuffix;
  SQLCHAR* localTypeName;
  SQLCHAR* name;
  SQLSMALLINT nullable;
  SQLINTEGER numPrecRadix;
  SQLLEN octetLength;
  SQLLEN* octetLengthPtr;
  SQLSMALLINT parameterType;
  SQLSMALLINT precision;
  SQLSMALLINT rowver;
  SQLSMALLINT scale;
  SQLCHAR* schemaName;
  SQLSMALLINT searchable;
  SQLCHAR* tableName;
  SQLSMALLINT type;
  SQLCHAR* typeName;
  SQLSMALLINT unnamed;
  SQLSMALLINT descUnsigned;
  SQLSMALLINT updatable;
};

/* Descriptor type */
enum DescType { ARD, APD, IRD, IPD, UNKNOWN };

class IGNITE_IMPORT_EXPORT Descriptor : public diagnostic::DiagnosableAdapter {
 public:
  /**
   * Constructor.
   */
  Descriptor();

  /**
   * Destructor.
   */
  ~Descriptor() {
    conn_ = nullptr;
    stmt_ = nullptr;
  }

  /**
   * Init application allocated descriptor head.
   *
   * @param implicit Implicit flag.
   */
  void InitAppHead(bool implicit);

  /**
   * Init implicit allocated descriptor head.
   */
  void InitImpHead();

  /**
   * Record the connection which creates this descriptor.
   *
   * @param conn Connection pointer.
   */
  void SetConnection(Connection* conn) {
    conn_ = conn;
  }

  /**
   * Record the statement which binds this descriptor.
   *
   * @param stmt Statement pointer.
   */
  void SetStatement(Statement* stmt) {
    stmt_ = stmt;
  }

  /**
   * Set descriptor type.
   *
   * @param type Descriptor type.
   */
  void SetType(DescType type) {
    type_ = type;
  }

  /**
   * Get descriptor type.
   *
   * @return Descriptor type enum value.
   */
  DescType GetType() {
    return type_;
  }

  /**
   * Deregister from the statement, where the descriptor is bound
   */
  void Deregister();

  /**
   * Get connection pointer.
   *
   * @return Connection*.
   */
  Connection* GetConnection() {
    return conn_;
  }

  /**
   * Get statement pointer.
   *
   * @return Statement*.
   */
  Statement* GetStatement() {
    return stmt_;
  }

  /**
   * Get descriptor header.
   *
   * @return DescriptorHeader&.
   */
  DescriptorHeader& GetHeader() {
    return header_;
  }

  /**
   * Get descriptor records map.
   *
   * @return Reference to the map.
   */
  std::map< int, DescriptorRecord >& GetRecords() {
    return records_;
  }

  /**
   * Set a descriptor field.
   *
   * @param recNum Record index.
   * @param fieldId Field Id.
   * @param buffer Buffer that holds the value.
   * @param bufferLen Buffer length.
   */
  void SetField(int recNum, int fieldId, SQLPOINTER buffer,
                SQLINTEGER bufferLen);

  /**
   * Get a descriptor field value.
   *
   * @param recNum Record index.
   * @param fieldId Field Id.
   * @param buffer Buffer that holds the value.
   * @param bufferLen Buffer length.
   * @param resLen Result length.
   */
  void GetField(int recNum, int fieldId, SQLPOINTER buffer,
                SQLINTEGER bufferLen, SQLINTEGER* resLen);

  /**
   * Copy current descriptor content to destination descriptor
   *
   * @param dst Destination descriptor.
   */
  void CopyDesc(Descriptor* dst);

  /**
   * Convert descriptor type from enum to string.
   *
   * @param type Descriptor type.
   * @return Descriptor type in string.
   */
  static std::string DescTypeToString(DescType type);

  /**
   * Convert field Id from int to string.
   *
   * @param fieldId Field Id.
   * @return Field Id in string.
   */
  static std::string FieldIdToString(int fieldId);

  /**
   * Convert SQL type from int to string.
   *
   * @param type SQL type.
   * @return SQL type in string.
   */
  static std::string SQLTypeToString(int type);

  /**
   * Convert interval code from int to string.
   *
   * @param type Interval code.
   * @return Interval code in string.
   */
  static std::string IntervalCodeToString(int code);

 private:
  /**
   * Set a descriptor field.
   * Internal call
   *
   * @param recNum Record index.
   * @param fieldId Field Id.
   * @param buffer Buffer that holds the value.
   * @param bufferLen Buffer length.
   * @return Operation result.
   */
  SqlResult::Type InternalSetField(int recNum, int fieldId, SQLPOINTER buffer,
                                   SQLINTEGER bufferLen);

  /**
   * Get a descriptor field value.
   * Internal call
   *
   * @param recNum Record index.
   * @param fieldId Field Id.
   * @param buffer Buffer that holds the value.
   * @param bufferLen Buffer length.
   * @param resLen Result length.
   * @return Operation result.
   */
  SqlResult::Type InternalGetField(int recNum, int fieldId, SQLPOINTER buffer,
                                   SQLINTEGER bufferLen, SQLINTEGER* resLen);

  /**
   * Copy current descriptor content to destination descriptor
   * Internal call
   *
   * @param dst Destination descriptor.
   * @return Operation result.
   */
  SqlResult::Type InternalCopyDesc(Descriptor* dst);

  /**
   * Check if the value is a valid concise type.
   *
   * @param value Concise SQL type.
   * @return true on valid concise type, false on invalid concise type
   */
  bool IsValidConciseType(int value);

  /**
   * Check if the value is a valid SQL type.
   *
   * @param value SQL type.
   * @return true on valid type, false on invalid type
   */
  bool IsValidType(int value);

  /**
   * Check if the value is a valid interval code.
   *
   * @param record Descriptor record.
   * @param value Interval code value.
   * @return true on valid interval code, false on invalid interval code
   */
  bool IsValidIntervalCode(DescriptorRecord& record, int value);

  /**
   * Set concise type for a record.
   *
   * @param record Descriptor record.
   * @param value Concise SQL type.
   */
  void SetConciseType(DescriptorRecord& record, int value);

  /**
   * Set SQL_DESC_TYPE value for a record.
   *
   * @param record Descriptor record.
   * @param value Type value.
   */
  void SetDescType(DescriptorRecord& record, int value);

  /** Descriptor type */
  DescType type_;

  /** Connection pointer */
  Connection* conn_;

  /** Statement pointer */
  Statement* stmt_;

  /** Descriptor header */
  DescriptorHeader header_;

  /** Descriptor record map, key is column index */
  std::map< int, DescriptorRecord > records_;
};

}  // namespace odbc
}  // namespace timestream

#endif  // _TIMESTREAM_ODBC_DESCRIPTOR
