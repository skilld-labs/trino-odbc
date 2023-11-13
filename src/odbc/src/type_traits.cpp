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

#include "trino/odbc/type_traits.h"

#include "trino/odbc/system/odbc_constants.h"

namespace trino {
namespace odbc {
namespace type_traits {
const std::string SqlTypeName::INTEGER("INTEGER");

const std::string SqlTypeName::DOUBLE("DOUBLE");

const std::string SqlTypeName::BIT("BIT");

const std::string SqlTypeName::BIGINT("BIGINT");

const std::string SqlTypeName::VARCHAR("VARCHAR");

const std::string SqlTypeName::DATE("DATE");

const std::string SqlTypeName::TIMESTAMP("TIMESTAMP");

const std::string SqlTypeName::TIME("TIME");

const std::string SqlTypeName::INTERVAL_DAY_TO_SECOND("INTERVAL_DAY_TO_SECOND");

const std::string SqlTypeName::INTERVAL_YEAR_TO_MONTH("INTERVAL_YEAR_TO_MONTH");

const std::string SqlTypeName::NOT_SET("NOT_SET");

const std::string SqlTypeName::UNKNOWN("UNKNOWN");

#ifdef _DEBUG

#define DBG_STR_CASE(x) \
  case x:               \
    return #x

const char* StatementAttrIdToString(long id) {
  switch (id) {
    DBG_STR_CASE(SQL_ATTR_APP_PARAM_DESC);
    DBG_STR_CASE(SQL_ATTR_APP_ROW_DESC);
    DBG_STR_CASE(SQL_ATTR_ASYNC_ENABLE);
    DBG_STR_CASE(SQL_ATTR_CONCURRENCY);
    DBG_STR_CASE(SQL_ATTR_CURSOR_SCROLLABLE);
    DBG_STR_CASE(SQL_ATTR_CURSOR_SENSITIVITY);
    DBG_STR_CASE(SQL_ATTR_CURSOR_TYPE);
    DBG_STR_CASE(SQL_ATTR_ENABLE_AUTO_IPD);
    DBG_STR_CASE(SQL_ATTR_FETCH_BOOKMARK_PTR);
    DBG_STR_CASE(SQL_ATTR_IMP_PARAM_DESC);
    DBG_STR_CASE(SQL_ATTR_IMP_ROW_DESC);
    DBG_STR_CASE(SQL_ATTR_KEYSET_SIZE);
    DBG_STR_CASE(SQL_ATTR_MAX_LENGTH);
    DBG_STR_CASE(SQL_ATTR_MAX_ROWS);
    DBG_STR_CASE(SQL_ATTR_METADATA_ID);
    DBG_STR_CASE(SQL_ATTR_NOSCAN);
    DBG_STR_CASE(SQL_ATTR_PARAM_BIND_OFFSET_PTR);
    DBG_STR_CASE(SQL_ATTR_PARAM_BIND_TYPE);
    DBG_STR_CASE(SQL_ATTR_PARAM_OPERATION_PTR);
    DBG_STR_CASE(SQL_ATTR_PARAM_STATUS_PTR);
    DBG_STR_CASE(SQL_ATTR_PARAMS_PROCESSED_PTR);
    DBG_STR_CASE(SQL_ATTR_PARAMSET_SIZE);
    DBG_STR_CASE(SQL_ATTR_QUERY_TIMEOUT);
    DBG_STR_CASE(SQL_ATTR_RETRIEVE_DATA);
    DBG_STR_CASE(SQL_ATTR_ROW_ARRAY_SIZE);
    DBG_STR_CASE(SQL_ATTR_ROW_BIND_OFFSET_PTR);
    DBG_STR_CASE(SQL_ATTR_ROW_BIND_TYPE);
    DBG_STR_CASE(SQL_ATTR_ROW_NUMBER);
    DBG_STR_CASE(SQL_ATTR_ROW_OPERATION_PTR);
    DBG_STR_CASE(SQL_ATTR_ROW_STATUS_PTR);
    DBG_STR_CASE(SQL_ATTR_ROWS_FETCHED_PTR);
    DBG_STR_CASE(SQL_ATTR_SIMULATE_CURSOR);
    DBG_STR_CASE(SQL_ATTR_USE_BOOKMARKS);
    default:
      break;
  }
  return "<< UNKNOWN ID >>";
}

#undef DBG_STR_CASE
#endif  // _DEBUG

const boost::optional< std::string > BinaryTypeToSqlTypeName(
    boost::optional< int16_t > binaryType) {
  if (!binaryType)
    return boost::none;

  ScalarType scalarType = static_cast< ScalarType >(*binaryType);

  switch (scalarType) {
    case ScalarType::BOOLEAN:
      return SqlTypeName::BIT;

    case ScalarType::INTEGER:
      return SqlTypeName::INTEGER;

    case ScalarType::BIGINT:
      return SqlTypeName::BIGINT;

    case ScalarType::DOUBLE:
      return SqlTypeName::DOUBLE;

    case ScalarType::DATE:
      return SqlTypeName::DATE;

    case ScalarType::TIME:
      return SqlTypeName::TIME;

    case ScalarType::TIMESTAMP:
      return SqlTypeName::TIMESTAMP;

    case ScalarType::INTERVAL_DAY_TO_SECOND:
      return SqlTypeName::INTERVAL_DAY_TO_SECOND;

    case ScalarType::INTERVAL_YEAR_TO_MONTH:
      return SqlTypeName::INTERVAL_YEAR_TO_MONTH;

    case ScalarType::VARCHAR:
      return SqlTypeName::VARCHAR;

    case ScalarType::NOT_SET:
      return SqlTypeName::NOT_SET;

    case ScalarType::UNKNOWN:
      return SqlTypeName::UNKNOWN;

    default:
      return SqlTypeName::VARCHAR;
  }
}

bool IsApplicationTypeSupported(boost::optional< int16_t > type) {
  if (type)
    return ToDriverType(*type) != OdbcNativeType::AI_UNSUPPORTED;
  else
    return false;
}

bool IsSqlTypeSupported(boost::optional< int16_t > type) {
  if (!type)
    return false;
  switch (*type) {
    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_SMALLINT:
    case SQL_BIGINT:
    case SQL_INTEGER:
    case SQL_FLOAT:
    case SQL_REAL:
    case SQL_DOUBLE:
    case SQL_NUMERIC:
    case SQL_DECIMAL:
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_WLONGVARCHAR:
    case SQL_TYPE_DATE:
    case SQL_TYPE_TIMESTAMP:
    case SQL_TYPE_TIME:
    case SQL_TYPE_NULL:
    case SQL_INTERVAL_YEAR_TO_MONTH:
    case SQL_INTERVAL_DAY_TO_SECOND:
      return true;

    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
    case SQL_GUID:
    case SQL_INTERVAL_MONTH:
    case SQL_INTERVAL_YEAR:
    case SQL_INTERVAL_DAY:
    case SQL_INTERVAL_HOUR:
    case SQL_INTERVAL_MINUTE:
    case SQL_INTERVAL_SECOND:
    case SQL_INTERVAL_DAY_TO_HOUR:
    case SQL_INTERVAL_DAY_TO_MINUTE:
    case SQL_INTERVAL_HOUR_TO_MINUTE:
    case SQL_INTERVAL_HOUR_TO_SECOND:
    case SQL_INTERVAL_MINUTE_TO_SECOND:
    default:
      return false;
  }
}

ScalarType SqlTypeToBinary(boost::optional< int16_t > sqlType) {
  if (!sqlType)
    return ScalarType::UNKNOWN;

  switch (*sqlType) {
    case SQL_BIT:
      return ScalarType::BOOLEAN;

    case SQL_INTEGER:
      return ScalarType::INTEGER;

    case SQL_BIGINT:
      return ScalarType::BIGINT;

    case SQL_DOUBLE:
      return ScalarType::DOUBLE;

    case SQL_TYPE_DATE:
      return ScalarType::DATE;

    case SQL_TYPE_TIME:
      return ScalarType::TIME;

    case SQL_TYPE_TIMESTAMP:
      return ScalarType::TIMESTAMP;

    case SQL_INTERVAL_DAY_TO_SECOND:
      return ScalarType::INTERVAL_DAY_TO_SECOND;

    case SQL_INTERVAL_YEAR_TO_MONTH:
      return ScalarType::INTERVAL_YEAR_TO_MONTH;

    case SQL_VARCHAR:
      return ScalarType::VARCHAR;

    default:
      return ScalarType::UNKNOWN;
  }
}

OdbcNativeType::Type ToDriverType(int16_t type) {
  switch (type) {
    case SQL_C_CHAR:
      return OdbcNativeType::AI_CHAR;

    case SQL_C_WCHAR:
      return OdbcNativeType::AI_WCHAR;

    case SQL_C_SSHORT:
    case SQL_C_SHORT:
      return OdbcNativeType::AI_SIGNED_SHORT;

    case SQL_C_USHORT:
      return OdbcNativeType::AI_UNSIGNED_SHORT;

    case SQL_C_SLONG:
    case SQL_C_LONG:
      return OdbcNativeType::AI_SIGNED_LONG;

    case SQL_C_ULONG:
      return OdbcNativeType::AI_UNSIGNED_LONG;

    case SQL_C_FLOAT:
      return OdbcNativeType::AI_FLOAT;

    case SQL_C_DOUBLE:
      return OdbcNativeType::AI_DOUBLE;

    case SQL_C_BIT:
      return OdbcNativeType::AI_BIT;

    case SQL_C_STINYINT:
    case SQL_C_TINYINT:
      return OdbcNativeType::AI_SIGNED_TINYINT;

    case SQL_C_UTINYINT:
      return OdbcNativeType::AI_UNSIGNED_TINYINT;

    case SQL_C_SBIGINT:
      return OdbcNativeType::AI_SIGNED_BIGINT;

    case SQL_C_UBIGINT:
      return OdbcNativeType::AI_UNSIGNED_BIGINT;

    case SQL_C_BINARY:
      return OdbcNativeType::AI_BINARY;

    case SQL_C_DATE:
    case SQL_C_TYPE_DATE:
      return OdbcNativeType::AI_TDATE;

    case SQL_C_TIME:
    case SQL_C_TYPE_TIME:
      return OdbcNativeType::AI_TTIME;

    case SQL_C_TIMESTAMP:
    case SQL_C_TYPE_TIMESTAMP:
      return OdbcNativeType::AI_TTIMESTAMP;

    case SQL_C_NUMERIC:
      return OdbcNativeType::AI_NUMERIC;

    case SQL_C_DEFAULT:
      return OdbcNativeType::AI_DEFAULT;

    case SQL_C_INTERVAL_YEAR:
      return OdbcNativeType::AI_INTERVAL_YEAR;

    case SQL_C_INTERVAL_MONTH:
      return OdbcNativeType::AI_INTERVAL_MONTH;

    case SQL_C_INTERVAL_DAY:
      return OdbcNativeType::AI_INTERVAL_DAY;

    case SQL_C_INTERVAL_HOUR:
      return OdbcNativeType::AI_INTERVAL_HOUR;

    case SQL_C_INTERVAL_MINUTE:
      return OdbcNativeType::AI_INTERVAL_MINUTE;

    case SQL_C_INTERVAL_SECOND:
      return OdbcNativeType::AI_INTERVAL_SECOND;

    case SQL_C_INTERVAL_DAY_TO_HOUR:
      return OdbcNativeType::AI_INTERVAL_DAY_TO_HOUR;

    case SQL_C_INTERVAL_DAY_TO_MINUTE:
      return OdbcNativeType::AI_INTERVAL_DAY_TO_MINUTE;

    case SQL_C_INTERVAL_HOUR_TO_MINUTE:
      return OdbcNativeType::AI_INTERVAL_HOUR_TO_MINUTE;

    case SQL_C_INTERVAL_HOUR_TO_SECOND:
      return OdbcNativeType::AI_INTERVAL_HOUR_TO_SECOND;

    case SQL_C_INTERVAL_MINUTE_TO_SECOND:
      return OdbcNativeType::AI_INTERVAL_MINUTE_TO_SECOND;

    case SQL_C_INTERVAL_YEAR_TO_MONTH:
      return OdbcNativeType::AI_INTERVAL_YEAR_TO_MONTH;

    case SQL_C_INTERVAL_DAY_TO_SECOND:
      return OdbcNativeType::AI_INTERVAL_DAY_TO_SECOND;

    default:
      return OdbcNativeType::AI_UNSUPPORTED;
  }
}

boost::optional< int16_t > BinaryToSqlType(
    boost::optional< int16_t > binaryType) {
  if (!binaryType)
    return boost::none;

  ScalarType scalarType = static_cast< ScalarType >(*binaryType);

  switch (scalarType) {
    case ScalarType::BOOLEAN:
      return SQL_BIT;

    case ScalarType::INTEGER:
      return SQL_INTEGER;

    case ScalarType::BIGINT:
      return SQL_BIGINT;

    case ScalarType::DOUBLE:
      return SQL_DOUBLE;

    case ScalarType::DATE:
      return SQL_TYPE_DATE;

    case ScalarType::TIME:
      return SQL_TYPE_TIME;

    case ScalarType::TIMESTAMP:
      return SQL_TYPE_TIMESTAMP;

    case ScalarType::INTERVAL_DAY_TO_SECOND:
      return SQL_INTERVAL_DAY_TO_SECOND;

    case ScalarType::INTERVAL_YEAR_TO_MONTH:
      return SQL_INTERVAL_YEAR_TO_MONTH;

    case ScalarType::VARCHAR:
    case ScalarType::NOT_SET:
    case ScalarType::UNKNOWN:
    default:
      return SQL_VARCHAR;
  }
}

int16_t BinaryTypeNullability(int16_t) {
  return SQL_NULLABLE_UNKNOWN;
}

boost::optional< std::string > NullabilityToIsNullable(
    boost::optional< int32_t > nullability) {
  if (!nullability)
    return boost::none;
  switch (*nullability) {
    case SQL_NO_NULLS:
      return std::string("NO");

    case SQL_NULLABLE:
      return std::string("YES");

    default:
      return boost::none;
  }
}

boost::optional< int32_t > SqlTypeDisplaySize(boost::optional< int16_t > type) {
  if (!type)
    return boost::none;
  switch (*type) {
    case SQL_VARCHAR:
    case SQL_WVARCHAR:
    case SQL_CHAR:
    case SQL_WCHAR:
    case SQL_LONGVARCHAR:
    case SQL_WLONGVARCHAR:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
      return TRINO_SQL_MAX_LENGTH;

    case SQL_BIT:
    case SQL_TYPE_NULL:
      return 1;

    case SQL_TINYINT:
      return 4;

    case SQL_SMALLINT:
      return 6;

    case SQL_INTEGER:
      return 11;

    case SQL_BIGINT:
      return 20;

    case SQL_REAL:
      return 14;

    case SQL_FLOAT:
    case SQL_DOUBLE:
      return 24;

    case SQL_TYPE_DATE:
      return 10;

    case SQL_TYPE_TIME:
      return 8;

    case SQL_TYPE_TIMESTAMP:
      return 20;

    case SQL_INTERVAL_DAY_TO_SECOND:
      return 25;

    case SQL_INTERVAL_YEAR_TO_MONTH:
      return 12;

    case SQL_GUID:
      return 36;

    // Binary types are not supported in Trino, return 0
    default:
      return 0;
  }
}

boost::optional< int32_t > BinaryTypeDisplaySize(
    boost::optional< int16_t > type) {
  boost::optional< int16_t > sqlType = BinaryToSqlType(type);

  return SqlTypeDisplaySize(sqlType);
}

boost::optional< int32_t > SqlTypeColumnSize(boost::optional< int16_t > type) {
  if (!type)
    return boost::none;
  switch (*type) {
    case SQL_VARCHAR:
    case SQL_WVARCHAR:
    case SQL_CHAR:
    case SQL_WCHAR:
    case SQL_LONGVARCHAR:
    case SQL_WLONGVARCHAR:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
      return TRINO_SQL_MAX_LENGTH;

    case SQL_BIT:
    case SQL_TYPE_NULL:
      return 1;

    case SQL_TINYINT:
      return 3;

    case SQL_SMALLINT:
      return 5;

    case SQL_INTEGER:
      return 10;

    case SQL_BIGINT:
      return 19;

    case SQL_REAL:
      return 7;

    case SQL_FLOAT:
    case SQL_DOUBLE:
      return 15;

    case SQL_TYPE_DATE:
      return 10;

    case SQL_TYPE_TIME:
      return 8;

    case SQL_TYPE_TIMESTAMP:
      return 19;

    case SQL_GUID:
      return 36;

    case SQL_INTERVAL_DAY_TO_SECOND:
      return 25;

    case SQL_INTERVAL_YEAR_TO_MONTH:
      return 12;

    // Binary types are not supported in Trino, return 0
    default:
      return 0;
  }
}

boost::optional< int32_t > BinaryTypeColumnSize(
    boost::optional< int16_t > type) {
  boost::optional< int16_t > sqlType = BinaryToSqlType(type);

  return SqlTypeColumnSize(sqlType);
}

boost::optional< int32_t > SqlTypeTransferLength(
    boost::optional< int16_t > type) {
  if (!type)
    return boost::none;
  switch (*type) {
    case SQL_VARCHAR:
    case SQL_WVARCHAR:
    case SQL_CHAR:
    case SQL_WCHAR:
    case SQL_LONGVARCHAR:
    case SQL_WLONGVARCHAR:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
      return TRINO_SQL_MAX_LENGTH;

    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_TYPE_NULL:
      return 1;

    case SQL_SMALLINT:
      return 2;

    case SQL_INTEGER:
      return 4;

    case SQL_BIGINT:
      return 8;

    case SQL_FLOAT:
    case SQL_REAL:
      return 4;

    case SQL_DOUBLE:
      return 8;

    case SQL_TYPE_DATE:
    case SQL_TYPE_TIME:
      return 6;

    case SQL_TYPE_TIMESTAMP:
      return 16;

    case SQL_GUID:
      return 16;

    case SQL_INTERVAL_DAY_TO_SECOND:
    case SQL_INTERVAL_YEAR_TO_MONTH:
      return 34;

    // Binary types are not supported in Trino, return 0
    default:
      return 0;
  }
}

boost::optional< int32_t > SqlTypePrecision(boost::optional< int16_t > type) {
  if (!type)
    return boost::none;
  switch (*type) {
    case SQL_DECIMAL:
    case SQL_NUMERIC:
      return 15;

    case SQL_TINYINT:
      return 3;

    case SQL_SMALLINT:
      return 5;

    case SQL_INTEGER:
      return 10;

    case SQL_BIGINT:
      return 19;

    case SQL_FLOAT:
    case SQL_REAL:
    case SQL_DOUBLE:
      return 15;

    case SQL_TYPE_TIME:
      return 6;

    case SQL_INTERVAL_DAY_TO_SECOND:
      return 11;
    case SQL_INTERVAL_YEAR_TO_MONTH:
      return 9;

    // return 0 for all other cases
    default:
      return 0;
  }
}

boost::optional< int32_t > SqlTypeScale(boost::optional< int16_t > type) {
  if (!type)
    return boost::none;
  switch (*type) {
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_FLOAT:
    case SQL_REAL:
    case SQL_DOUBLE:
      return 15;

    // return 0 for all other cases
    default:
      return 0;
  }
}

boost::optional< int32_t > BinaryTypeTransferLength(
    boost::optional< int16_t > type) {
  boost::optional< int16_t > sqlType = BinaryToSqlType(type);

  return SqlTypeTransferLength(sqlType);
}

boost::optional< int32_t > SqlTypeNumPrecRadix(
    boost::optional< int16_t > type) {
  if (!type)
    return boost::none;
  switch (*type) {
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
      return 2;

    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIGINT:
      return 10;

    default:
      return 0;
  }
}

boost::optional< int32_t > BinaryTypeNumPrecRadix(
    boost::optional< int16_t > type) {
  boost::optional< int16_t > sqlType = BinaryToSqlType(type);

  return SqlTypeNumPrecRadix(sqlType);
}

boost::optional< int16_t > SqlTypeDecimalDigits(
    boost::optional< int16_t > type) {
  if (!type)
    return boost::none;
  switch (*type) {
    case SQL_TINYINT:
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIGINT:
      return 0;

    case SQL_DOUBLE:
      return 15;

    default:
      return -1;
  }
}

boost::optional< int16_t > BinaryTypeDecimalDigits(
    boost::optional< int16_t > type) {
  boost::optional< int16_t > sqlType = BinaryToSqlType(type);

  return SqlTypeDecimalDigits(sqlType);
}

boost::optional< int32_t > SqlTypeCharOctetLength(
    boost::optional< int16_t > type) {
  if (!type)
    return boost::none;
  switch (*type) {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
      return TRINO_SQL_MAX_LENGTH;

    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_WLONGVARCHAR:
      return sizeof(SQLWCHAR) * TRINO_SQL_MAX_LENGTH;

    // Binary types are not supported in Trino, return 0
    default:
      return 0;
  }
}

boost::optional< int32_t > BinaryTypeCharOctetLength(
    boost::optional< int16_t > type) {
  boost::optional< int16_t > sqlType = BinaryToSqlType(type);

  return SqlTypeCharOctetLength(sqlType);
}

bool SqlTypeUnsigned(boost::optional< int16_t > type) {
  if (!type)
    return false;
  switch (*type) {
    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIGINT:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
      return false;

    default:
      return true;
  }
}

bool BinaryTypeUnsigned(boost::optional< int16_t > type) {
  boost::optional< int16_t > sqlType = BinaryToSqlType(type);

  return SqlTypeUnsigned(sqlType);
}
}  // namespace type_traits
}  // namespace odbc
}  // namespace trino
