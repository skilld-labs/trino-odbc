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

#ifndef _TRINO_ODBC_QUERY_TABLE_METADATA_QUERY
#define _TRINO_ODBC_QUERY_TABLE_METADATA_QUERY

#include "trino/odbc/meta/table_meta.h"
#include "trino/odbc/query/query.h"
#include "trino/odbc/query/data_query.h"
#include "trino/odbc/statement.h"

namespace trino {
namespace odbc {
/** Connection forward-declaration. */
class Connection;

namespace query {
/**
 * Query.
 */
class TableMetadataQuery : public trino::odbc::query::Query {
 public:
  struct ResultColumn {
    enum Type {
      /** Catalog name. NULL if not applicable to the data source. */
      TABLE_CAT = 1,

      /** Schema name. NULL if not applicable to the data source. */
      TABLE_SCHEM,

      /** Table name. */
      TABLE_NAME,

      /** Table type. */
      TABLE_TYPE,

      /** A description of the column. */
      REMARKS
    };
  };

  /**
   * Constructor.
   *
   * @param diag Diagnostics collector.
   * @param connection Associated connection.
   * @param catalog Catalog search pattern.
   * @param schema Schema search pattern.
   * @param table Table search pattern.
   * @param tableType Table type search pattern.
   */
  TableMetadataQuery(diagnostic::DiagnosableAdapter& diag,
                     Connection& connection,
                     const boost::optional< std::string >& catalog,
                     const boost::optional< std::string >& schema,
                     const boost::optional< std::string >& table,
                     const boost::optional< std::string >& tableType);

  /**
   * Destructor.
   */
  virtual ~TableMetadataQuery();

  /**
   * Execute query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Execute();

  /**
   * Cancel query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Cancel();

  /**
   * Get column metadata.
   *
   * @return Column metadata.
   */
  virtual const meta::ColumnMetaVector* GetMeta();

  /**
   * Fetch next result row to application buffers.
   *
   * @return Operation result.
   */
  virtual SqlResult::Type FetchNextRow(app::ColumnBindingMap& columnBindings);

  /**
   * Get data of the specified column in the result set.
   *
   * @param columnIdx Column index.
   * @param buffer Buffer to put column data to.
   * @return Operation result.
   */
  virtual SqlResult::Type GetColumn(uint16_t columnIdx,
                                    app::ApplicationDataBuffer& buffer);

  /**
   * Close query.
   *
   * @return True on success.
   */
  virtual SqlResult::Type Close();

  /**
   * Check if data is available.
   *
   * @return True if data is available.
   */
  virtual bool DataAvailable() const;

  /**
   * Get number of rows affected by the statement.
   *
   * @return Number of rows affected by the statement.
   */
  virtual int64_t AffectedRows() const;

  /**
   * Get row number of the row that the cursor points at.
   * Row number starts at 1.
   *
   * @return Row number of the row that the cursor points at.
   */
  virtual int64_t RowNumber() const;

  /**
   * Move to the next result set.
   *
   * @return Operation result.
   */
  virtual SqlResult::Type NextResultSet();

 private:
  IGNITE_NO_COPY_ASSIGNMENT(TableMetadataQuery);

  /**
   * Make get tables metadata requets and use response to set internal state.
   *
   * @return Operation result.
   */
  SqlResult::Type MakeRequestGetTablesMeta();

  /**
   * Validate and handle database search patterns / identifiers in tables meta
   * retrieval
   *
   * @return Operation result.
   */
  SqlResult::Type getTables();

  /**
   * Get the list of all databases.
   *
   * @return Operation result
   */
  SqlResult::Type getAllDatabases();

  /**
   * Get tables that have exact match with the database and table
   * case-sensitivie identifier
   *
   * @param databasePattern Database pattern
   * @return Operation result
   */
  SqlResult::Type getTablesWithIdentifier(
      const std::string& databaseIdentifier);

  /**
   * Get tables that match the database and table case-insensitivie search
   * patterns
   *
   * @param databasePattern Database pattern
   * @return Operation result
   */
  SqlResult::Type getTablesWithSearchPattern(
      const boost::optional< std::string >& databasePattern);

  /**
   *  Get the database names that match database pattern
   *
   * @param databasePattern Database name search pattern
   * @param databaseNames Vector to store database names
   * @return Operation result
   */
  SqlResult::Type getMatchedDatabases(
      const std::string& databasePattern,
      std::vector< std::string >& databaseNames);

  /**
   *  Get the table names that match table pattern from specified database
   *
   * @param databaseName Database name
   * @param tablePattern Table name search pattern
   * @param tablePattern Vector to store table names
   * @return Operation result
   */
  SqlResult::Type getMatchedTables(const std::string& databaseName,
                                   const std::string& tablePattern,
                                   std::vector< std::string >& c);

  /**
   * Remove outer matching quotes from a string. They can be either single (')
   * or double (") quotes. They must be the left- and right-most characters in
   * the string.
   *
   * @return the string with matched quotes removed.
   */
  std::string dequote(const std::string& s);

  /** Connection associated with the statement. */
  Connection& connection;

  /** Catalog search pattern. */
  boost::optional< std::string > catalog;

  /** Schema search pattern. */
  boost::optional< std::string > schema;

  /** Table search pattern. */
  boost::optional< std::string > table;

  /** Table type search pattern. */
  boost::optional< std::string > tableType;

  /** Query executed. */
  bool executed;

  /** Fetched flag. */
  bool fetched;

  /** Return a list of catalogs flag. */
  bool all_catalogs;

  /** Return a list of schemas flag. */
  bool all_schemas;

  /** Return a list of supported table types flag. */
  bool all_table_types;

  /** Fetched metadata. */
  meta::TableMetaVector meta;

  /** Metadata cursor. */
  meta::TableMetaVector::iterator cursor;

  /** Columns metadata. */
  meta::ColumnMetaVector columnsMeta;

  /** DataQuery pointer for "show" command to run **/
  std::shared_ptr< DataQuery > dataQuery_;
};
}  // namespace query
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_QUERY_TABLE_METADATA_QUERY
