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

#ifndef _TRINO_ODBC_META_COLUMN_META
#define _TRINO_ODBC_META_COLUMN_META

#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <string>

#include "trino/odbc/common_types.h"
#include "trino/odbc/log.h"
#include "trino/odbc/utility.h"
#include "trino/odbc/app/application_data_buffer.h"

/*#*/
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/trino-query/model/ColumnInfo.h>
#include <aws/trino-query/model/ScalarType.h>

using Aws::TrinoQuery::Model::ColumnInfo; /*#*/
using Aws::TrinoQuery::Model::ScalarType; /*#*/

namespace trino {
namespace odbc {
namespace meta {
/**
 * Nullability type.
 */
struct Nullability {
  enum Type {
    NO_NULL = 0,

    NULLABLE = 1,

    NULLABILITY_UNKNOWN = 2
  };

  /**
   * Convert to SQL constant.
   *
   * @param nullability Nullability.
   * @return SQL constant.
   */
  static SqlLen ToSql(boost::optional< int32_t > nullability);
};

using namespace trino::odbc;

/**
 * Column metadata.
 */
class IGNITE_IMPORT_EXPORT ColumnMeta {
 public:
  /**
   * Convert attribute ID to string containing its name.
   * Debug function.
   * @param type Attribute ID.
   * @return Null-terminated string containing attribute name.
   */
  static const char* AttrIdToString(uint16_t id);

  /**
   * Default constructor.
   */
  ColumnMeta()
      : nullability(Nullability::NULLABILITY_UNKNOWN),
        isAutoIncrement("NO"),
        precision(-1),
        scale(-1),
        ordinalPosition(-1) {
    // No-op.
  }

  /**
   * Constructor.
   *
   * @param databaseName Database name.
   * @param tableName Table name.
   */
  ColumnMeta(const std::string& databaseName,
             const boost::optional< std::string >& tableName)
      : tableName(tableName),
        columnName(""),
        dataType(static_cast< int16_t >(ScalarType::NOT_SET)),
        isAutoIncrement("NO"),
        precision(-1),
        decimalDigits(-1),
        scale(-1),
        nullability(Nullability::NULLABILITY_UNKNOWN),
        ordinalPosition(-1) {
    if (DATABASE_AS_SCHEMA)
      schemaName = databaseName;
    else
      catalogName = databaseName;
  }

  /**
   * Constructor.
   *
   * @param databaseName Database name.
   * @param tableName Table name.
   * @param columnName Column name.
   * @param scalarType Data type.
   * @param nullability Nullability
   */
  ColumnMeta(const std::string& databaseName, const std::string& tableName,
             const std::string& columnName, ScalarType scalarType,
             Nullability::Type nullability)
      : tableName(tableName),
        columnName(columnName),
        dataType(static_cast< int16_t >(scalarType)),
        isAutoIncrement("NO"),
        precision(-1),
        decimalDigits(-1),
        scale(-1),
        nullability(nullability),
        ordinalPosition(-1) {
    if (DATABASE_AS_SCHEMA)
      schemaName = databaseName;
    else
      catalogName = databaseName;
  }

  /**
   * Constructor.
   *
   * @param databaseName Database name.
   * @param tableName Table name.
   * @param columnName Column name.
   * @param dataType Data type.
   * @param nullability Nullability
   */
  ColumnMeta(const std::string& databaseName, const std::string& tableName,
             const std::string& columnName, int16_t dataType,
             Nullability::Type nullability)
      : tableName(tableName),
        columnName(columnName),
        dataType(dataType),
        isAutoIncrement("NO"),
        precision(-1),
        decimalDigits(-1),
        scale(-1),
        nullability(nullability),
        ordinalPosition(-1) {
    if (DATABASE_AS_SCHEMA)
      schemaName = databaseName;
    else
      catalogName = databaseName;
  }

  /**
   * Destructor.
   */
  ~ColumnMeta() {
    // No-op.
  }

  /**
   * Copy constructor.
   */
  ColumnMeta(const ColumnMeta& other)
      : columnInfo(other.columnInfo),
        catalogName(other.catalogName),
        schemaName(other.schemaName),
        tableName(other.tableName),
        columnName(other.columnName),
        remarks(other.remarks),
        columnDef(other.columnDef),
        isAutoIncrement(other.isAutoIncrement),
        dataType(other.dataType),
        precision(other.precision),
        decimalDigits(other.decimalDigits),
        scale(other.scale),
        nullability(other.nullability),
        ordinalPosition(other.ordinalPosition) {
    // No-op.
  }

  /**
   * Copy operator.
   */
  ColumnMeta& operator=(const ColumnMeta& other) {
    columnInfo = other.columnInfo;
    catalogName = other.catalogName;
    schemaName = other.schemaName;
    tableName = other.tableName;
    columnName = other.columnName;
    remarks = other.remarks;
    columnDef = other.columnDef;
    isAutoIncrement = other.isAutoIncrement;
    dataType = other.dataType;
    precision = other.precision;
    decimalDigits = other.decimalDigits;
    scale = other.scale;
    nullability = other.nullability;
    ordinalPosition = other.ordinalPosition;

    return *this;
  }

  /**
   * Read using reader.
   * @param columnBindings the map containing the data to be read.
   * @param position the ordinal position of the column.
   */
  void Read(trino::odbc::app::ColumnBindingMap& columnBindings,
            int32_t position);

  /**
   * Read using reader.
   * @param trinoVector Vector containing metadata for one row.
   */
  void ReadMetadata(const ColumnInfo& TrinoVector);

  /**
   * Get Aws ColumnInfo.
   * @return Aws ColumnInfo.
   */
  const boost::optional< Aws::TrinoQuery::Model::ColumnInfo >& /*#*/
  GetColumnInfo() const {
    return columnInfo;
  }

  /**
   * Get catalog name.
   * @return Catalog name.
   */
  const boost::optional< std::string >& GetCatalogName() const {
    return catalogName;
  }

  /**
   * Get schema name.
   * @return Schema name.
   */
  const boost::optional< std::string >& GetSchemaName() const {
    return schemaName;
  }

  /**
   * Get table name.
   * @return Table name.
   */
  const boost::optional< std::string >& GetTableName() const {
    return tableName;
  }

  /**
   * Get column name.
   * @return Column name.
   */
  const boost::optional< std::string >& GetColumnName() const {
    return columnName;
  }

  /**
   * Get the remarks.
   * @return Remarks.
   */
  const boost::optional< std::string >& GetRemarks() const {
    return remarks;
  }

  /**
   * Get the column default value.
   * @return Column default value.
   */
  const boost::optional< std::string >& GetColumnDef() const {
    return columnDef;
  }

  /**
   * Get the column is auto increment.
   * @return Column is auto increment.
   */
  const std::string GetIsAutoIncrement() const {
    return isAutoIncrement;
  }

  /**
   * Get data type.
   * @return Data type.
   */
  boost::optional< int16_t > GetDataType() const {
    return dataType;
  }

  /**
   * Get data type.
   * @return ScalarType type.
   */
  ScalarType GetScalarType() const {
    if (!dataType) {
      LOG_WARNING_MSG("dataType is not set. Returning ScalarType::NOT_SET");
      return ScalarType::NOT_SET;
    }

    return static_cast< ScalarType >(*dataType);
  }

  /**
   * Get column precision.
   * @return Column precision.
   */
  boost::optional< int32_t > GetPrecision() const {
    return precision;
  }

  /**
   * Get column decimal digits.
   * @return Column decimal digits.
   */
  boost::optional< int32_t > GetDecimalDigits() const {
    return decimalDigits;
  }

  /**
   * Get column scale.
   * @return Column scale.
   */
  boost::optional< int32_t > GetScale() const {
    return scale;
  }

  /**
   * Get column nullability.
   * @return Column nullability.
   */
  boost::optional< int32_t > GetNullability() const {
    return nullability;
  }

  /**
   * Get column ordinal position.
   * @return Column ordinal position.
   */
  boost::optional< int32_t > GetOrdinalPosition() const {
    return ordinalPosition;
  }

  /**
   * Try to get attribute of a string type.
   *
   * @param fieldId Field ID.
   * @param value Output attribute value.
   * @return True if the attribute supported and false otherwise.
   */
  bool GetAttribute(uint16_t fieldId, std::string& value) const;

  /**
   * Try to get attribute of a integer type.
   *
   * @param fieldId Field ID.
   * @param value Output attribute value.
   * @return True if the attribute supported and false otherwise.
   */
  bool GetAttribute(uint16_t fieldId, SqlLen& value) const;

 private:
  /**
   * Get the scalar type based on string data type.
   *
   * @param dataType data type in string.
   * @return TrinoQuery ScalarType
   */
  ScalarType GetScalarDataType(const std::string& dataType);

  /** columnInfo. */
  boost::optional< Aws::TrinoQuery::Model::ColumnInfo > columnInfo; /*#*/

  /** Catalog name. */
  boost::optional< std::string > catalogName;

  /** Schema name. */
  boost::optional< std::string > schemaName;

  /** Table name. */
  boost::optional< std::string > tableName;

  /** Column name. */
  boost::optional< std::string > columnName;

  /** Remarks */
  boost::optional< std::string > remarks;

  /** Column default value */
  boost::optional< std::string > columnDef;

  /** Column is auto incremented */
  // Trino columns are not auto increment
  std::string isAutoIncrement;

  /** Data type. */
  boost::optional< int16_t > dataType;

  /** Column precision. */
  boost::optional< int32_t > precision;

  /** Column decimal digits. */
  boost::optional< int32_t > decimalDigits;

  /** Column scale. */
  boost::optional< int32_t > scale;

  /** Column nullability. */
  boost::optional< int32_t > nullability;

  /** Column ordinal position. */
  boost::optional< int32_t > ordinalPosition;
};

/** Column metadata vector alias. */
typedef std::vector< ColumnMeta > ColumnMetaVector;
}  // namespace meta
}  // namespace odbc
}  // namespace trino
#endif  //_TRINO_ODBC_META_COLUMN_META
