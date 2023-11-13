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

#ifndef _TRINO_ODBC_TYPE_TRAITS
#define _TRINO_ODBC_TYPE_TRAITS

#ifndef TRINO_SQL_MAX_LENGTH
#define TRINO_SQL_MAX_LENGTH 65532
#endif  // TRINO_SQL_MAX_LENGTH

#include <stdint.h>

#include <string>

#include <boost/optional.hpp>
#include <ignite/common/common.h>
/*@*/
#include <aws/trino-query/model/ScalarType.h>

using Aws::TrinoQuery::Model::ScalarType;

namespace trino {
namespace odbc {
namespace type_traits {
#ifdef _DEBUG
/**
 * Convert statement attribute ID to string containing its name.
 * Debug function.
 * @param id Attribute ID.
 * @return Null-terminated string containing attribute name.
 */
const char* StatementAttrIdToString(long id);
#endif  //_DEBUG

/**
 * ODBC type aliases.
 * We use these so we will not be needed to include system-specific
 * headers in our header files.
 */
struct OdbcNativeType {
  enum Type {
    /** Alias for the SQL_C_CHAR type. */
    AI_CHAR,

    /** Alias for the SQL_C_WCHAR type. */
    AI_WCHAR,

    /** Alias for the SQL_C_SSHORT type. */
    AI_SIGNED_SHORT,

    /** Alias for the SQL_C_USHORT type. */
    AI_UNSIGNED_SHORT,

    /** Alias for the SQL_C_SLONG type. */
    AI_SIGNED_LONG,

    /** Alias for the SQL_C_ULONG type. */
    AI_UNSIGNED_LONG,

    /** Alias for the SQL_C_FLOAT type. */
    AI_FLOAT,

    /** Alias for the SQL_C_DOUBLE type. */
    AI_DOUBLE,

    /** Alias for the SQL_C_BIT type. */
    AI_BIT,

    /** Alias for the SQL_C_STINYINT type. */
    AI_SIGNED_TINYINT,

    /** Alias for the SQL_C_UTINYINT type. */
    AI_UNSIGNED_TINYINT,

    /** Alias for the SQL_C_SBIGINT type. */
    AI_SIGNED_BIGINT,

    /** Alias for the SQL_C_UBIGINT type. */
    AI_UNSIGNED_BIGINT,

    /** Alias for the SQL_C_BINARY type. */
    AI_BINARY,

    /** Alias for the SQL_C_TDATE type. */
    AI_TDATE,

    /** Alias for the SQL_C_TTIME type. */
    AI_TTIME,

    /** Alias for the SQL_C_TTIMESTAMP type. */
    AI_TTIMESTAMP,

    /** Alias for the SQL_C_NUMERIC type. */
    AI_NUMERIC,

    /** Alias for the SQL_C_INTERVAL_YEAR type. */
    AI_INTERVAL_YEAR,

    /** Alias for the SQL_C_INTERVAL_MONTH type. */
    AI_INTERVAL_MONTH,

    /** Alias for the SQL_C_INTERVAL_DAY type. */
    AI_INTERVAL_DAY,

    /** Alias for the SQL_C_INTERVAL_HOUR type. */
    AI_INTERVAL_HOUR,

    /** Alias for the SQL_C_INTERVAL_MINUTE type. */
    AI_INTERVAL_MINUTE,

    /** Alias for the SQL_C_INTERVAL_SECOND type. */
    AI_INTERVAL_SECOND,

    /** Alias for the SQL_C_INTERVAL_DAY_TO_HOUR type. */
    AI_INTERVAL_DAY_TO_HOUR,

    /** Alias for the SQL_C_INTERVAL_DAY_TO_MINUTE type. */
    AI_INTERVAL_DAY_TO_MINUTE,

    /** Alias for the SQL_C_INTERVAL_HOUR_TO_MINUTE type. */
    AI_INTERVAL_HOUR_TO_MINUTE,

    /** Alias for the SQL_C_INTERVAL_HOUR_TO_SECOND type. */
    AI_INTERVAL_HOUR_TO_SECOND,

    /** Alias for the SQL_C_INTERVAL_MINUTE_TO_SECOND type. */
    AI_INTERVAL_MINUTE_TO_SECOND,

    /** Alias for the SQL_C_INTERVAL_YEAR_MONTH type. */
    AI_INTERVAL_YEAR_TO_MONTH,

    /** Alias for the SQL_C_INTERVAL_DAY_SECOND type. */
    AI_INTERVAL_DAY_TO_SECOND,

    /** Alias for the SQL_DEFAULT. */
    AI_DEFAULT,

    /** Alias for all unsupported types. */
    AI_UNSUPPORTED
  };
};

/**
 * SQL type name constants.
 */
class IGNITE_IMPORT_EXPORT SqlTypeName {
 public:
  /** INTEGER SQL type name constant. */
  static const std::string INTEGER;

  /** DOUBLE SQL type name constant. */
  static const std::string DOUBLE;

  /** BIT SQL type name constant. */
  static const std::string BIT;

  /** BIGINT SQL type name constant. */
  static const std::string BIGINT;

  /** VARCHAR SQL type name constant. */
  static const std::string VARCHAR;

  /** DATE SQL type name constant. */
  static const std::string DATE;

  /** TIMESTAMP SQL type name constant. */
  static const std::string TIMESTAMP;

  /** TIME SQL type name constant. */
  static const std::string TIME;

  /** INTERVAL_DAY_TO_SECOND SQL type name constant. */
  static const std::string INTERVAL_DAY_TO_SECOND;

  /** INTERVAL_YEAR_TO_MONTH SQL type name constant. */
  static const std::string INTERVAL_YEAR_TO_MONTH;

  /** INTERVAL_YEAR_TO_MONTH SQL type name constant. */
  static const std::string NOT_SET;

  /** INTERVAL_YEAR_TO_MONTH SQL type name constant. */
  static const std::string UNKNOWN;
};

/**
 * Get SQL type name for the binary type.
 *
 * @param binaryType Binary type.
 * @return Corresponding SQL type name.
 */
const boost::optional< std::string > BinaryTypeToSqlTypeName(
    boost::optional< int16_t > binaryType);

/**
 * Check if the C type supported by the current implementation.
 *
 * @param type Application type.
 * @return True if the type is supported.
 */
bool IsApplicationTypeSupported(boost::optional< int16_t > type);

/**
 * Check if the SQL type supported by the current implementation.
 *
 * @param type Application type.
 * @return True if the type is supported.
 */
bool IsSqlTypeSupported(boost::optional< int16_t > type);

/**
 * Get corresponding binary type for ODBC SQL type.
 *
 * @param sqlType SQL type.
 * @return Binary type.
 */
ScalarType SqlTypeToBinary(boost::optional< int16_t > sqlType);

/**
 * Convert ODBC type to driver type alias.
 *
 * @param type ODBC type;
 * @return Internal driver type.
 */
OdbcNativeType::Type ToDriverType(int16_t type);

/**
 * Convert binary data type to SQL data type.
 *
 * @param binaryType Binary data type.
 * @return SQL data type.
 */
boost::optional< int16_t > BinaryToSqlType(
    boost::optional< int16_t > binaryType);

/**
 * Get binary type SQL nullability.
 *
 * @param binaryType Binary data type.
 * @return SQL_NO_NULLS if the column could not include NULL values.
 *         SQL_NULLABLE if the column accepts NULL values.
 *         SQL_NULLABLE_UNKNOWN if it is not known whether the
 *         column accepts NULL values.
 */
int16_t BinaryTypeNullability(int16_t);

/**
 * Get binary type SQL nullability.
 *
 * @param binaryType Binary data type.
 * @return "NO" if the column does not include NULLs.
 *         "YES" if the column could include NULLs.
 *         zero-length string if it is not known whether the
 *         column accepts NULL values.
 */
boost::optional< std::string > NullabilityToIsNullable(
    boost::optional< int32_t > nullability);

/**
 * Get SQL type display size.
 *
 * @param type SQL type.
 * @return Display size.
 */
boost::optional< int32_t > SqlTypeDisplaySize(boost::optional< int16_t > type);

/**
 * Get binary type display size.
 *
 * @param type Binary type.
 * @return Display size.
 */
boost::optional< int32_t > BinaryTypeDisplaySize(
    boost::optional< int16_t > type);

/**
 * Get SQL type column size.
 *
 * @param type SQL type.
 * @return Column size.
 */
boost::optional< int32_t > SqlTypeColumnSize(boost::optional< int16_t > type);

/**
 * Get binary type column size.
 *
 * @param type Binary type.
 * @return Column size.
 */
boost::optional< int32_t > BinaryTypeColumnSize(
    boost::optional< int16_t > type);

/**
 * Get SQL type transfer octet length.
 *
 * @param type SQL type.
 * @return Transfer octet length.
 */
boost::optional< int32_t > SqlTypeTransferLength(
    boost::optional< int16_t > type);

/**
 * Get SQL type precision.
 *
 * @param type SQL type.
 * @return Precision.
 */
boost::optional< int32_t > SqlTypePrecision(boost::optional< int16_t > type);

/**
 * Get SQL type scale.
 *
 * @param type SQL type.
 * @return Scale.
 */
boost::optional< int32_t > SqlTypeScale(boost::optional< int16_t > type);

/**
 * Get binary type transfer octet length.
 *
 * @param type Binary type.
 * @return Transfer octet length.
 */
boost::optional< int32_t > BinaryTypeTransferLength(
    boost::optional< int16_t > type);

/**
 * Get SQL type numeric precision radix.
 *
 * @param type SQL type.
 * @return Numeric precision radix.
 */
boost::optional< int32_t > SqlTypeNumPrecRadix(boost::optional< int16_t > type);

/**
 * Get binary type numeric precision radix.
 *
 * @param type Binary type.
 * @return Numeric precision radix.
 */
boost::optional< int32_t > BinaryTypeNumPrecRadix(
    boost::optional< int16_t > type);

/**
 * Get SQL type decimal digits.
 *
 * @param type SQL type.
 * @return Decimal digits.
 */
boost::optional< int16_t > SqlTypeDecimalDigits(
    boost::optional< int16_t > type);

/**
 * Get binary type decimal digits.
 *
 * @param type Binary type.
 * @return Decimal digits.
 */
boost::optional< int16_t > BinaryTypeDecimalDigits(
    boost::optional< int16_t > type);

/**
 * Get SQL type char octet length.
 *
 * @param type SQL type.
 * @return Char octet length.
 */
boost::optional< int32_t > SqlTypeCharOctetLength(
    boost::optional< int16_t > type);

/**
 * Get binary type char octet length.
 *
 * @param type Binary type.
 * @return Char octet length.
 */
boost::optional< int32_t > BinaryTypeCharOctetLength(
    boost::optional< int16_t > type);

/**
 * Checks if the SQL type is unsigned.
 *
 * @param type SQL type.
 * @return True if unsigned or non-numeric.
 */
bool SqlTypeUnsigned(boost::optional< int16_t > type);

/**
 * Checks if the binary type is unsigned.
 *
 * @param type Binary type.
 * @return True if unsigned or non-numeric.
 */
bool BinaryTypeUnsigned(boost::optional< int16_t > type);
}  // namespace type_traits
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_TYPE_TRAITS
