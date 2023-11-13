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
#include "trino/odbc/descriptor.h"
#include "trino/odbc/log.h"
#include "trino/odbc/statement.h"

#define ALLOWED_DESC_TYPE1(type, fieldId, allowed_type)                    \
  if (type != allowed_type) {                                              \
    std::stringstream ss;                                                  \
    ss << "Current descriptor type " << Descriptor::DescTypeToString(type) \
       << " is not allowed to get field "                                  \
       << Descriptor::FieldIdToString(fieldId);                            \
    AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, ss.str());             \
    return SqlResult::AI_ERROR;                                            \
  }

#define ALLOWED_DESC_TYPE2(type, fieldId, allowed_type1, allowed_type2)    \
  if (type != allowed_type1 && type != allowed_type2) {                    \
    std::stringstream ss;                                                  \
    ss << "Current descriptor type " << Descriptor::DescTypeToString(type) \
       << " is not allowed to get field "                                  \
       << Descriptor::FieldIdToString(fieldId);                            \
    AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, ss.str());             \
    return SqlResult::AI_ERROR;                                            \
  }

#define GET_DESC_FIELD_VALUE(type, value)        \
  type* val = reinterpret_cast< type* >(buffer); \
  *val = value;

namespace trino {
namespace odbc {
Descriptor::Descriptor()
    : type_(DescType::UNKNOWN), conn_(nullptr), stmt_(nullptr) {
}

void Descriptor::InitAppHead(bool implicit) {
  header_.allocType = implicit ? SQL_DESC_ALLOC_AUTO : SQL_DESC_ALLOC_USER;
  header_.arraySize = 1;
  header_.arrayStatusPtr = nullptr;
  header_.bindOffsetPtr = nullptr;
  header_.bindType = SQL_BIND_BY_COLUMN;
  header_.count = 0;
}

void Descriptor::InitImpHead() {
  header_.allocType = SQL_DESC_ALLOC_AUTO;
  header_.arrayStatusPtr = nullptr;
  header_.count = 0;
  header_.rowsProcessedPtr = nullptr;
}

void Descriptor::Deregister() {
  // restore the statement implicit descriptor to be active descriptor
  stmt_->RestoreDescriptor(type_);
}

void Descriptor::SetField(int recNum, int fieldId, SQLPOINTER buffer,
                          SQLINTEGER bufferLen) {
  IGNITE_ODBC_API_CALL(InternalSetField(recNum, fieldId, buffer, bufferLen));
}

SqlResult::Type Descriptor::InternalSetField(int recNum, int fieldId,
                                             SQLPOINTER buffer,
                                             SQLINTEGER bufferLen) {
  if (bufferLen < 0 && (bufferLen != SQL_NTS && bufferLen != SQL_IS_POINTER)) {
    std::stringstream ss;
    ss << "Invalid buffer length " << bufferLen;

    AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, ss.str());
    return SqlResult::AI_ERROR;
  }

  DescriptorRecord& record = records_[recNum];
  switch (fieldId) {
    // Header fields could not be set
    case SQL_DESC_ALLOC_TYPE:
    case SQL_DESC_ARRAY_SIZE:
    case SQL_DESC_ARRAY_STATUS_PTR:
    case SQL_DESC_BIND_OFFSET_PTR:
    case SQL_DESC_BIND_TYPE:
    case SQL_DESC_COUNT:
    case SQL_DESC_ROWS_PROCESSED_PTR: {
      AddStatusRecord(SqlState::SHY091_INVALID_DESCR_FIELD_ID,
                      "Invalid descriptor field id");
      return SqlResult::AI_ERROR;
    }

    case SQL_DESC_CONCISE_TYPE: {
      int value =
          static_cast< SQLSMALLINT >(reinterpret_cast< ptrdiff_t >(buffer));
      if (!IsValidConciseType(value)) {
        std::stringstream ss;
        ss << "Invalid concise type " << Descriptor::SQLTypeToString(value);

        AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, ss.str());
        return SqlResult::AI_ERROR;
      }
      SetConciseType(record, value);
      break;
    }

    case SQL_DESC_DATA_PTR: {
      if (!buffer) {
        stmt_->SafeUnbindColumn(recNum);
        record.length = 0;
        record.octetLength = 0;
      } else {
        record.dataPtr = buffer;
        record.length = bufferLen;
        record.octetLength = bufferLen;
      }
      break;
    }

    case SQL_DESC_DATETIME_INTERVAL_CODE: {
      int code =
          static_cast< SQLSMALLINT >(reinterpret_cast< ptrdiff_t >(buffer));
      if (!IsValidIntervalCode(record, code)) {
        std::stringstream ss;
        ss << "Invalid interval code " << Descriptor::IntervalCodeToString(code)
           << " for type " << Descriptor::SQLTypeToString(record.type);

        AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, ss.str());
        return SqlResult::AI_ERROR;
      }
      record.datetimeIntervalCode = code;
      break;
    }

    case SQL_DESC_DATETIME_INTERVAL_PRECISION: {
      if (record.type != SQL_INTERVAL) {
        AddStatusRecord(SqlState::SHY000_GENERAL_ERROR,
                        "Interval precision could only be set when "
                        "SQL_DESC_TYPE is set to SQL_INTERVAL");
        return SqlResult::AI_ERROR;
      }
      record.datetimeIntervalPrecision =
          static_cast< SQLINTEGER >(reinterpret_cast< ptrdiff_t >(buffer));
      break;
    }

    case SQL_DESC_INDICATOR_PTR: {
      record.indicatorPtr = reinterpret_cast< SQLLEN* >(buffer);
      break;
    }

    case SQL_DESC_OCTET_LENGTH_PTR: {
      record.octetLengthPtr = reinterpret_cast< SQLLEN* >(buffer);
      break;
    }

    case SQL_DESC_OCTET_LENGTH:
    case SQL_DESC_LENGTH: {
      if (record.conciseType == SQL_CHAR || record.conciseType == SQL_VARCHAR
          || record.conciseType == SQL_LONGVARCHAR
          || record.conciseType == SQL_WCHAR
          || record.conciseType == SQL_WVARCHAR
          || record.conciseType == SQL_WLONGVARCHAR) {
        record.length = reinterpret_cast< SQLULEN >(buffer);
        record.octetLength = record.length;
      } else {
        std::stringstream ss;
        ss << "SQL_DESC_LENGTH could not be set for fixed length type "
           << Descriptor::SQLTypeToString(record.conciseType);

        AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, ss.str());
        return SqlResult::AI_ERROR;
      }
      break;
    }
    case SQL_DESC_TYPE: {
      int value =
          static_cast< SQLSMALLINT >(reinterpret_cast< ptrdiff_t >(buffer));
      if (!IsValidType(value)) {
        std::stringstream ss;
        ss << "Invalid type " << Descriptor::SQLTypeToString(value);

        AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, ss.str());
        return SqlResult::AI_ERROR;
      }
      SetDescType(record, value);
      break;
    }
    default: {
      std::stringstream ss;
      ss << "Unsupported fieldId " << fieldId;

      AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, ss.str());
      return SqlResult::AI_ERROR;
    }
  }
  return SqlResult::AI_SUCCESS;
}

void Descriptor::GetField(int recNum, int fieldId, SQLPOINTER buffer,
                          SQLINTEGER bufferLen, SQLINTEGER* resLen) {
  IGNITE_ODBC_API_CALL(
      InternalGetField(recNum, fieldId, buffer, bufferLen, resLen));
}

SqlResult::Type Descriptor::InternalGetField(int recNum, int fieldId,
                                             SQLPOINTER buffer,
                                             SQLINTEGER bufferLen,
                                             SQLINTEGER* resLen) {
  DescriptorRecord& record = records_[recNum];
  switch (fieldId) {
    case SQL_DESC_ALLOC_TYPE: {
      GET_DESC_FIELD_VALUE(SQLSMALLINT, header_.allocType)
      break;
    }
    case SQL_DESC_ARRAY_SIZE: {
      ALLOWED_DESC_TYPE2(type_, fieldId, ARD, APD)
      GET_DESC_FIELD_VALUE(SQLULEN, header_.arraySize)
      break;
    }
    case SQL_DESC_ARRAY_STATUS_PTR: {
      GET_DESC_FIELD_VALUE(SQLUSMALLINT*, header_.arrayStatusPtr)
      break;
    }
    case SQL_DESC_BIND_OFFSET_PTR: {
      ALLOWED_DESC_TYPE2(type_, fieldId, ARD, APD)
      GET_DESC_FIELD_VALUE(SQLLEN*, header_.bindOffsetPtr)
      break;
    }
    case SQL_DESC_BIND_TYPE: {
      ALLOWED_DESC_TYPE2(type_, fieldId, ARD, APD)
      GET_DESC_FIELD_VALUE(SQLINTEGER, header_.bindType)
      break;
    }
    case SQL_DESC_COUNT: {
      GET_DESC_FIELD_VALUE(SQLSMALLINT, header_.count)
      break;
    }
    case SQL_DESC_ROWS_PROCESSED_PTR: {
      ALLOWED_DESC_TYPE2(type_, fieldId, IRD, IPD)
      GET_DESC_FIELD_VALUE(SQLULEN*, header_.rowsProcessedPtr)
      break;
    }

    case SQL_DESC_AUTO_UNIQUE_VALUE: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLINTEGER, record.autoUniqueValue)
      break;
    }
    case SQL_DESC_BASE_COLUMN_NAME: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.baseColumnName)
      break;
    }
    case SQL_DESC_BASE_TABLE_NAME: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.baseTableName)
      break;
    }
    case SQL_DESC_CASE_SENSITIVE: {
      ALLOWED_DESC_TYPE2(type_, fieldId, IRD, IPD)
      GET_DESC_FIELD_VALUE(SQLINTEGER, record.caseSensitive)
      break;
    }
    case SQL_DESC_CATALOG_NAME: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.catalogName)
      break;
    }
    case SQL_DESC_CONCISE_TYPE: {
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.conciseType)
      break;
    }
    case SQL_DESC_DATA_PTR: {
      ALLOWED_DESC_TYPE2(type_, fieldId, ARD, APD)
      GET_DESC_FIELD_VALUE(SQLPOINTER, record.dataPtr)
      break;
    }
    case SQL_DESC_DATETIME_INTERVAL_CODE: {
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.datetimeIntervalCode)
      break;
    }
    case SQL_DESC_DATETIME_INTERVAL_PRECISION: {
      GET_DESC_FIELD_VALUE(SQLINTEGER, record.datetimeIntervalPrecision)
      break;
    }
    case SQL_DESC_DISPLAY_SIZE: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLLEN, record.displaySize)
      break;
    }
    case SQL_DESC_FIXED_PREC_SCALE: {
      ALLOWED_DESC_TYPE2(type_, fieldId, IRD, IPD)
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.fixedPrecScale)
      break;
    }
    case SQL_DESC_INDICATOR_PTR: {
      ALLOWED_DESC_TYPE2(type_, fieldId, ARD, APD)
      GET_DESC_FIELD_VALUE(SQLLEN*, record.indicatorPtr)
      break;
    }
    case SQL_DESC_LABEL: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.label)
      break;
    }
    case SQL_DESC_LENGTH: {
      GET_DESC_FIELD_VALUE(SQLULEN, record.length)
      break;
    }
    case SQL_DESC_LITERAL_PREFIX: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.literalPrefix)
      break;
    }
    case SQL_DESC_LITERAL_SUFFIX: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.literalSuffix)
      break;
    }
    case SQL_DESC_LOCAL_TYPE_NAME: {
      ALLOWED_DESC_TYPE2(type_, fieldId, IRD, IPD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.localTypeName)
      break;
    }
    case SQL_DESC_NAME: {
      ALLOWED_DESC_TYPE2(type_, fieldId, IRD, IPD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.name)
      break;
    }
    case SQL_DESC_NULLABLE: {
      ALLOWED_DESC_TYPE2(type_, fieldId, IRD, IPD)
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.nullable)
      break;
    }
    case SQL_DESC_NUM_PREC_RADIX: {
      GET_DESC_FIELD_VALUE(SQLINTEGER, record.numPrecRadix)
      break;
    }
    case SQL_DESC_OCTET_LENGTH: {
      GET_DESC_FIELD_VALUE(SQLLEN, record.octetLength)
      break;
    }
    case SQL_DESC_OCTET_LENGTH_PTR: {
      ALLOWED_DESC_TYPE2(type_, fieldId, ARD, APD)
      GET_DESC_FIELD_VALUE(SQLLEN*, record.octetLengthPtr)
      break;
    }
    case SQL_DESC_PARAMETER_TYPE: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IPD)
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.parameterType)
      break;
    }
    case SQL_DESC_PRECISION: {
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.precision)
      break;
    }
    case SQL_DESC_ROWVER: {
      ALLOWED_DESC_TYPE2(type_, fieldId, IRD, IPD)
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.rowver)
      break;
    }
    case SQL_DESC_SCALE: {
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.scale)
      break;
    }
    case SQL_DESC_SCHEMA_NAME: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.schemaName)
      break;
    }
    case SQL_DESC_SEARCHABLE: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.searchable)
      break;
    }
    case SQL_DESC_TABLE_NAME: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.tableName)
      break;
    }
    case SQL_DESC_TYPE: {
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.type)
      break;
    }
    case SQL_DESC_TYPE_NAME: {
      ALLOWED_DESC_TYPE2(type_, fieldId, IRD, IPD)
      GET_DESC_FIELD_VALUE(SQLCHAR*, record.typeName)
      break;
    }
    case SQL_DESC_UNNAMED: {
      ALLOWED_DESC_TYPE2(type_, fieldId, IRD, IPD)
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.unnamed)
      break;
    }
    case SQL_DESC_UNSIGNED: {
      ALLOWED_DESC_TYPE2(type_, fieldId, IRD, IPD)
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.descUnsigned)
      break;
    }
    case SQL_DESC_UPDATABLE: {
      ALLOWED_DESC_TYPE1(type_, fieldId, IRD)
      GET_DESC_FIELD_VALUE(SQLSMALLINT, record.updatable)
      break;
    }
    default:
      std::stringstream ss;
      ss << "Unsupported fieldId " << fieldId;

      AddStatusRecord(SqlState::SHY000_GENERAL_ERROR, ss.str());
      return SqlResult::AI_ERROR;
  }

  return SqlResult::AI_SUCCESS;
}

void Descriptor::CopyDesc(Descriptor* dst) {
  IGNITE_ODBC_API_CALL(InternalCopyDesc(dst));
}

SqlResult::Type Descriptor::InternalCopyDesc(Descriptor* dst) {
  if (dst->GetType() == DescType::IRD) {
    AddStatusRecord(SqlState::SHY016_MODIFY_IRD,
                    "Cannot modify an implementation row descriptor");
    return SqlResult::AI_ERROR;
  }

  // reset destination descriptor
  memset(&(dst->GetHeader()), 0, sizeof(DescriptorHeader));
  dst->GetRecords().clear();

  // copy current descriptor
  if (type_ == ARD || type_ == APD) {
    dst->InitAppHead(true);
  } else {
    dst->InitImpHead();
  }
  dst->SetType(type_);
  dst->SetConnection(conn_);
  dst->SetStatement(stmt_);

  dst->GetRecords() = records_;
  return SqlResult::AI_SUCCESS;
}

bool Descriptor::IsValidConciseType(int value) {
  switch (value) {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_WLONGVARCHAR:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_BIGINT:
    case SQL_TYPE_DATE:
    case SQL_TYPE_TIME:
    case SQL_TYPE_TIMESTAMP:
    case SQL_INTERVAL_YEAR_TO_MONTH:
    case SQL_INTERVAL_DAY_TO_SECOND:
      return true;
    default:
      return false;
  }
}

bool Descriptor::IsValidType(int value) {
  switch (value) {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_WLONGVARCHAR:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_BIGINT:
    case SQL_DATETIME:
    case SQL_INTERVAL:
      return true;
    default:
      return false;
  }
}

bool Descriptor::IsValidIntervalCode(DescriptorRecord& record, int value) {
  if (record.type == SQL_DATETIME) {
    if (value == SQL_CODE_DATE || value == SQL_CODE_TIME
        || value == SQL_CODE_TIMESTAMP) {
      return true;
    }
  } else if (record.type == SQL_INTERVAL) {
    if (value == SQL_CODE_YEAR_TO_MONTH || value == SQL_CODE_DAY_TO_SECOND) {
      return true;
    }
  }
  return false;
}

void Descriptor::SetConciseType(DescriptorRecord& record, int value) {
  record.conciseType = value;
  if (value == SQL_TYPE_DATE || value == SQL_TYPE_TIME
      || value == SQL_TYPE_TIMESTAMP) {
    record.type = SQL_DATETIME;
    if (value == SQL_TYPE_DATE) {
      record.datetimeIntervalCode = SQL_CODE_DATE;
    } else if (value == SQL_TYPE_TIME) {
      record.datetimeIntervalCode = SQL_CODE_TIME;
    } else {
      record.datetimeIntervalCode = SQL_CODE_TIMESTAMP;
    }
  } else if (value == SQL_INTERVAL_YEAR_TO_MONTH
             || value == SQL_INTERVAL_DAY_TO_SECOND) {
    record.type = SQL_INTERVAL;
    if (value == SQL_INTERVAL_YEAR_TO_MONTH) {
      record.datetimeIntervalCode = SQL_CODE_YEAR_TO_MONTH;
    } else {
      record.datetimeIntervalCode = SQL_CODE_DAY_TO_SECOND;
    }
  } else {
    record.type = value;
    record.datetimeIntervalCode = 0;
  }
}

void Descriptor::SetDescType(DescriptorRecord& record, int value) {
  record.type = value;
  if (value != SQL_DATETIME && value != SQL_INTERVAL) {
    record.conciseType = value;
    record.datetimeIntervalCode = 0;
  }

  if (value == SQL_CHAR || value == SQL_VARCHAR) {
    record.length = 1;
    record.precision = 0;
  }

  if (value == SQL_DATETIME) {
    if (record.datetimeIntervalCode == SQL_CODE_DATE
        || record.datetimeIntervalCode == SQL_CODE_TIME) {
      record.precision = 0;
    } else if (record.datetimeIntervalCode == SQL_CODE_TIMESTAMP) {
      record.precision = 6;
    }
  }

  if (value == SQL_DECIMAL || value == SQL_NUMERIC) {
    record.scale = 0;
    record.precision = 15;
  }

  if (value == SQL_FLOAT) {
    record.precision = 6;
  }

  if (value == SQL_INTERVAL) {
    if (record.datetimeIntervalCode == SQL_CODE_YEAR_TO_MONTH) {
      record.precision = 2;
    }
    if (record.datetimeIntervalCode == SQL_CODE_DAY_TO_SECOND) {
      record.precision = 6;
    }
  }
}

std::string Descriptor::DescTypeToString(DescType type) {
  switch (type) {
    case ARD:
      return "ARD";
    case APD:
      return "APD";
    case IRD:
      return "IRD";
    case IPD:
      return "IPD";
    case UNKNOWN:
      return "UNKNOWN";
    default:
      return "";
  }
}

std::string Descriptor::FieldIdToString(int fieldId) {
  switch (fieldId) {
    case SQL_DESC_ALLOC_TYPE:
      return "SQL_DESC_ALLOC_TYPE";
    case SQL_DESC_ARRAY_SIZE:
      return "SQL_DESC_ARRAY_SIZE";
    case SQL_DESC_ARRAY_STATUS_PTR:
      return "SQL_DESC_ARRAY_STATUS_PTR";
    case SQL_DESC_BIND_OFFSET_PTR:
      return "SQL_DESC_BIND_OFFSET_PTR";
    case SQL_DESC_BIND_TYPE:
      return "SQL_DESC_BIND_TYPE";
    case SQL_DESC_COUNT:
      return "SQL_DESC_COUNT";
    case SQL_DESC_ROWS_PROCESSED_PTR:
      return "SQL_DESC_ROWS_PROCESSED_PTR";
    case SQL_DESC_AUTO_UNIQUE_VALUE:
      return "SQL_DESC_AUTO_UNIQUE_VALUE";
    case SQL_DESC_BASE_COLUMN_NAME:
      return "SQL_DESC_BASE_COLUMN_NAME";
    case SQL_DESC_BASE_TABLE_NAME:
      return "SQL_DESC_BASE_TABLE_NAME";
    case SQL_DESC_CASE_SENSITIVE:
      return "SQL_DESC_CASE_SENSITIVE";
    case SQL_DESC_CATALOG_NAME:
      return "SQL_DESC_CATALOG_NAME";
    case SQL_DESC_CONCISE_TYPE:
      return "SQL_DESC_CONCISE_TYPE";
    case SQL_DESC_DATA_PTR:
      return "SQL_DESC_DATA_PTR";
    case SQL_DESC_DATETIME_INTERVAL_CODE:
      return "SQL_DESC_DATETIME_INTERVAL_CODE";
    case SQL_DESC_DATETIME_INTERVAL_PRECISION:
      return "SQL_DESC_DATETIME_INTERVAL_PRECISION";
    case SQL_DESC_DISPLAY_SIZE:
      return "SQL_DESC_DISPLAY_SIZE";
    case SQL_DESC_FIXED_PREC_SCALE:
      return "SQL_DESC_FIXED_PREC_SCALE";
    case SQL_DESC_INDICATOR_PTR:
      return "SQL_DESC_INDICATOR_PTR";
    case SQL_DESC_LABEL:
      return "SQL_DESC_LABEL";
    case SQL_DESC_LENGTH:
      return "SQL_DESC_LENGTH";
    case SQL_DESC_LITERAL_PREFIX:
      return "SQL_DESC_LITERAL_PREFIX";
    case SQL_DESC_LITERAL_SUFFIX:
      return "SQL_DESC_LITERAL_SUFFIX";
    case SQL_DESC_LOCAL_TYPE_NAME:
      return "SQL_DESC_LOCAL_TYPE_NAME";
    case SQL_DESC_NAME:
      return "SQL_DESC_NAME";
    case SQL_DESC_NULLABLE:
      return "SQL_DESC_NULLABLE";
    case SQL_DESC_NUM_PREC_RADIX:
      return "SQL_DESC_NUM_PREC_RADIX";
    case SQL_DESC_OCTET_LENGTH:
      return "SQL_DESC_OCTET_LENGTH";
    case SQL_DESC_OCTET_LENGTH_PTR:
      return "SQL_DESC_OCTET_LENGTH_PTR";
    case SQL_DESC_PARAMETER_TYPE:
      return "SQL_DESC_PARAMETER_TYPE";
    case SQL_DESC_PRECISION:
      return "SQL_DESC_PRECISION";
    case SQL_DESC_ROWVER:
      return "SQL_DESC_ROWVER";
    case SQL_DESC_SCALE:
      return "SQL_DESC_SCALE";
    case SQL_DESC_SCHEMA_NAME:
      return "SQL_DESC_SCHEMA_NAME";
    case SQL_DESC_SEARCHABLE:
      return "SQL_DESC_SEARCHABLE";
    case SQL_DESC_TABLE_NAME:
      return "SQL_DESC_TABLE_NAME";
    case SQL_DESC_TYPE:
      return "SQL_DESC_TYPE";
    case SQL_DESC_TYPE_NAME:
      return "SQL_DESC_TYPE_NAME";
    case SQL_DESC_UNNAMED:
      return "SQL_DESC_UNNAMED";
    case SQL_DESC_UNSIGNED:
      return "SQL_DESC_UNSIGNED";
    case SQL_DESC_UPDATABLE:
      return "SQL_DESC_UPDATABLE";
    default:
      return "UNKNOWN_FIELD_ID";
  }
}

std::string Descriptor::SQLTypeToString(int type) {
  switch (type) {
    case SQL_CHAR:
      return "SQL_CHAR";
    case SQL_VARCHAR:
      return "SQL_VARCHAR";
    case SQL_LONGVARCHAR:
      return "SQL_LONGVARCHAR";
    case SQL_WCHAR:
      return "SQL_WCHAR";
    case SQL_WVARCHAR:
      return "SQL_WVARCHAR";
    case SQL_WLONGVARCHAR:
      return "SQL_WLONGVARCHAR";
    case SQL_DECIMAL:
      return "SQL_DECIMAL";
    case SQL_NUMERIC:
      return "SQL_NUMERIC";
    case SQL_SMALLINT:
      return "SQL_SMALLINT";
    case SQL_INTEGER:
      return "SQL_INTEGER";
    case SQL_REAL:
      return "SQL_REAL";
    case SQL_FLOAT:
      return "SQL_FLOAT";
    case SQL_DOUBLE:
      return "SQL_DOUBLE";
    case SQL_BIT:
      return "SQL_BIT";
    case SQL_TINYINT:
      return "SQL_TINYINT";
    case SQL_BIGINT:
      return "SQL_BIGINT";
    case SQL_TYPE_DATE:
      return "SQL_TYPE_DATE";
    case SQL_TYPE_TIME:
      return "SQL_TYPE_TIME";
    case SQL_TYPE_TIMESTAMP:
      return "SQL_TYPE_TIMESTAMP";
    case SQL_INTERVAL_YEAR:
      return "SQL_INTERVAL_YEAR";
    case SQL_INTERVAL_MONTH:
      return "SQL_INTERVAL_MONTH";
    case SQL_INTERVAL_DAY:
      return "SQL_INTERVAL_DAY";
    case SQL_INTERVAL_HOUR:
      return "SQL_INTERVAL_HOUR";
    case SQL_INTERVAL_MINUTE:
      return "SQL_INTERVAL_MINUTE";
    case SQL_INTERVAL_SECOND:
      return "SQL_INTERVAL_SECOND";
    case SQL_INTERVAL_YEAR_TO_MONTH:
      return "SQL_INTERVAL_YEAR_TO_MONTH";
    case SQL_INTERVAL_DAY_TO_HOUR:
      return "SQL_INTERVAL_DAY_TO_HOUR";
    case SQL_INTERVAL_DAY_TO_MINUTE:
      return "SQL_INTERVAL_DAY_TO_MINUTE";
    case SQL_INTERVAL_DAY_TO_SECOND:
      return "SQL_INTERVAL_DAY_TO_SECOND";
    case SQL_INTERVAL_HOUR_TO_MINUTE:
      return "SQL_INTERVAL_HOUR_TO_MINUTE";
    case SQL_INTERVAL_HOUR_TO_SECOND:
      return "SQL_INTERVAL_HOUR_TO_SECOND";
    case SQL_INTERVAL_MINUTE_TO_SECOND:
      return "SQL_INTERVAL_MINUTE_TO_SECOND";
    case SQL_DATETIME:
      return "SQL_DATETIME";
    case SQL_INTERVAL:
      return "SQL_INTERVAL";
    case SQL_GUID:
      return "SQL_GUID";
    default:
      return std::to_string(type);
  }
}

std::string Descriptor::IntervalCodeToString(int code) {
  switch (code) {
    case SQL_CODE_DATE:
      return "SQL_CODE_DATE";
    case SQL_CODE_TIME:
      return "SQL_CODE_TIME";
    case SQL_CODE_TIMESTAMP:
      return "SQL_CODE_TIMESTAMP";
    case SQL_CODE_DAY_TO_HOUR:
      return "SQL_CODE_DAY_TO_HOUR";
    case SQL_CODE_DAY_TO_MINUTE:
      return "SQL_CODE_DAY_TO_MINUTE";
    case SQL_CODE_DAY_TO_SECOND:
      return "SQL_CODE_DAY_TO_SECOND";
    case SQL_CODE_HOUR:
      return "SQL_CODE_HOUR";
    case SQL_CODE_HOUR_TO_MINUTE:
      return "SQL_CODE_HOUR_TO_MINUTE";
    case SQL_CODE_HOUR_TO_SECOND:
      return "SQL_CODE_HOUR_TO_SECOND";
    case SQL_CODE_MINUTE:
      return "SQL_CODE_MINUTE";
    case SQL_CODE_MINUTE_TO_SECOND:
      return "SQL_CODE_MINUTE_TO_SECOND";
    case SQL_CODE_SECOND:
      return "SQL_CODE_SECOND";
    case SQL_CODE_YEAR_TO_MONTH:
      return "SQL_CODE_YEAR_TO_MONTH";
    default:
      return std::to_string(code);
  }
}
}  // namespace odbc
}  // namespace trino
