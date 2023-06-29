# ODBC Support and Limitations

## Topics
- [ODBC API Support](#odbc-api-support)
- [Supported Connection Information Types](#supported-connection-information-types)
- [Supported Connection Attributes](#supported-connection-attributes)
- [Supported Connection Options for SQLSetConnectOption](#Supported-Connection-Options-for-SQLSetConnectOption)
- [Supported Connection Options for SQLGetConnectOption](#Supported-Connection-Options-for-SQLGetConnectOption)
- [Supported Statements Attributes](#supported-statements-attributes)
- [Supported Statements Options for SQLGetStmtOption](#supported-statements-options-for-sqlgetstmtoption) 
- [SQLPrepare, SQLExecute and SQLExecDirect](#sqlprepare-sqlexecute-and-sqlexecdirect)
- [SQLTables](#sqltables)
- [Database Reporting Differences Between Timestream JDBC Driver and ODBC Driver](#database-reporting-differences-between-timestream-jdbc-driver-and-odbc-driver)
- [Timestream Data Types](#timestream-data-types)
- [Microsoft Excel on macOS](#microsoft-excel-on-macos)

## ODBC API Support
Note that the following table describes the planned functionality for the GA release. As the driver is still in development, not all functions marked as "yes" under the "Support" column below are supported at this time.

| Function | Support? <br /> yes/error/empty | Notes |
|----------|---------------------------------|-------|
| SQLAllocHandle | yes |
| SQLAllocConnect | yes |
| SQLAllocEnv | yes |
| SQLAllocStmt | yes |
| SQLBrowseConnect | no |
| SQLConnect | yes |
| SQLDriverConnect | yes |
| SQLColumnPrivileges | no (empty results) |
| SQLColumns | yes |
| SQLForeignKeys | no (empty results) |
| SQLPrimaryKeys | no (empty results) |
| SQLProcedureColumns | no (empty results) |
| SQLProcedures | no (empty results) |
| SQLSpecialColumns | no (empty results) |
| SQLStatistics | no (empty results) |
| SQLTablePrivileges | no (empty results) |
| SQLTables | yes |
| SQLCopyDesc | yes |
| SQLGetDescField | yes |
| SQLGetDescRec | no |
| SQLSetDescField | yes |
| SQLSetDescRec | no |
| SQLGetConnectAttr | yes |
| SQLGetEnvAttr | yes |
| SQLGetStmtAttr | yes |
| SQLSetConnectAttr | yes |
| SQLSetStmtAttr | yes |
| SQLGetFunctions | yes | Supported by driver manger for Windows and Linux, supported by driver for macOS as iODBC driver manager does not provide support for it on Big Sur
| SQLGetInfo | yes |
| SQLGetTypeInfo | yes |
| SQLDisconnect | yes |
| SQLCancel | yes |
| SQLCancelHandle | no | ODBC 3.8+ only
| SQLCloseCursor | yes |
| SQLEndTran | no (error) | Driver will not support transactions
| SQLFreeHandle | yes |
| SQLDescribeParam | no (empty results) | No parameter support
| SQLExecDirect | yes |
| SQLExecute | yes |
| SQLNativeSql | yes | Will return same SQL
| SQLNumParams | no (empty results) | No parameter support
| SQLParamData | no (error) | No parameter support
| SQLPutData | no (error) | No parameter support
| SQLBindParameter | no (error) | No parameter support
| SQLGetCursorName | yes |
| SQLPrepare | yes | No parameter support, so will just ready query for execution
| SQLSetCursorName | yes |
| SQLBindCol | yes |
| SQLBulkOperations | no |
| SQLColAttribute | yes |
| SQLDescribeCol | yes |
| SQLFetch | yes |
| SQLFetchScroll | yes | Only forward-only cursors supported, so this will match the behavior of SQLFetch
| SQLGetData | yes |
| SQLGetDiagField | yes |
| SQLGetDiagRec | yes |
| SQLMoreResults | yes |
| SQLNumResultCols | yes |
| SQLSetPos | no (error) |
| SQLColAttributes | yes |
| SQLError | yes |
| SQLExtendedFetch | yes | Only forward-only cursors supported, so this will match the behavior of SQLFetch
| SQLFreeConnect | yes |
| SQLFreeEnv | yes |
| SQLFreeStmt | yes |
| SQLGetConnectOption | yes |
| SQLGetStmtOption | yes |
| SQLParamOptions | yes |
| SQLRowCount | yes | Defined as SQL_ROWCOUNT_UNKNOWN
| SQLSetConnectOption | yes |
| SQLSetParam | no (error) | No parameter support
| SQLSetScrollOptions | no | Deprecated
| SQLSetStmtOption | yes |
| SQLTransact | no (error) | Driver does not support transactions

## Supported Connection Information Types
| Connection Information Types | Default | Support Value Change|
|--------|------|-------|
| SQL_DRIVER_NAME | 'timestream.odbc.dll' on Windows, 'libtimestream-odbc.dylib' on MacOS, and 'libtimestream-odbc.so' on Linux | no |
| SQL_DBMS_NAME | 'Amazon Timestream' | no |
| SQL_DRIVER_ODBC_VER | '03.00' | no |
| SQL_DRIVER_VER | MM.mm.pppp (MM = major, mm = minor, pppp = patch) | no |
| SQL_DBMS_VER | MM.mm.pppp (MM = major, mm = minor, pppp = patch) | no |
| SQL_COLUMN_ALIAS | 'Y' | no |
| SQL_IDENTIFIER_QUOTE_CHAR | '\"' | no |
| SQL_CATALOG_NAME_SEPARATOR | '.' | no |
| SQL_SPECIAL_CHARACTERS | '_' | no |
| SQL_CATALOG_TERM | 'database' (default case). If [`DATABASE_AS_SCHEMA`](../setup/developer-guide.md/#database-reporting) set to `TRUE`, it's '' | no |
| SQL_TABLE_TERM | 'table' | no |
| SQL_SCHEMA_TERM | '' (default case). If [`DATABASE_AS_SCHEMA`](../setup/developer-guide.md/#database-reporting) set to `TRUE`, it's 'schema' | no |
| SQL_NEED_LONG_DATA_LEN | 'N' | no |
| SQL_ACCESSIBLE_PROCEDURES | 'N' | no |
| SQL_ACCESSIBLE_TABLES | 'N' | no |
| SQL_CATALOG_NAME | 'Y' (default case). If [`DATABASE_AS_SCHEMA`](../setup/developer-guide.md/#database-reporting) set to `TRUE`, it's 'N' | no |
| SQL_COLLATION_SEQ | '' | no |
| SQL_DATA_SOURCE_NAME | '\<dsn\>" (if available) | no |
| SQL_DATA_SOURCE_READ_ONLY | 'Y' | no |
| SQL_DATABASE_NAME | '\<database\>' | no |
| SQL_DESCRIBE_PARAMETER | 'N' | no |
| SQL_EXPRESSIONS_IN_ORDERBY | 'Y' | no |
| SQL_INTEGRITY | 'N' | no |
| SQL_KEYWORDS | '' | no |
| SQL_LIKE_ESCAPE_CLAUSE | 'Y' | no |
| SQL_MAX_ROW_SIZE_INCLUDES_LONG | 'Y' | no |
| SQL_MULT_RESULT_SETS | 'N' | no |
| SQL_MULTIPLE_ACTIVE_TXN | 'Y' | no |
| SQL_ORDER_BY_COLUMNS_IN_SELECT | 'Y' | no |
| SQL_PROCEDURE_TERM | '' | no |
| SQL_PROCEDURES | 'N' | no |
| SQL_ROW_UPDATES | 'N' | no |
| SQL_SEARCH_PATTERN_ESCAPE | '' | no |
| SQL_SERVER_NAME | 'AWS Timestream' | no |
| SQL_USER_NAME | '\<user\>' | no |
| SQL_ASYNC_DBC_FUNCTIONS | SQL_ASYNC_DBC_NOT_CAPABLE | no |
| SQL_ASYNC_MODE | SQL_AM_NONE | no |
| SQL_ASYNC_NOTIFICATION | SQL_ASYNC_NOTIFICATION_NOT_CAPABLE | no |
| SQL_BATCH_ROW_COUNT | 0 (not supported) | no |
| SQL_BATCH_SUPPORT | 0 (not supported) | no |
| SQL_BOOKMARK_PERSISTENCE | 0 (not supported) | no |
| SQL_CATALOG_LOCATION | SQL_CL_START (default case). If [`DATABASE_AS_SCHEMA`](../setup/developer-guide.md/#database-reporting) set to `TRUE`, it's 0 (not supported) | no |
| SQL_QUALIFIER_LOCATION | 0 (not supported) | no |
| SQL_GETDATA_EXTENSIONS | SQL_GD_ANY_COLUMN, SQL_GD_ANY_ORDER, SQL_GD_BOUND, SQL_GD_BLOCK | no |
| SQL_ODBC_INTERFACE_CONFORMANCE | SQL_OIC_CORE | no |
| SQL_SQL_CONFORMANCE | SQL_SC_SQL92_ENTRY | no |
| SQL_CATALOG_USAGE | SQL_CU_DML_STATEMENTS (default case). If [`DATABASE_AS_SCHEMA`](../setup/developer-guide.md/#database-reporting) set to `TRUE`, it's 0 (not supported) | no |
| SQL_SCHEMA_USAGE | 0 (not supported) (default case). If [`DATABASE_AS_SCHEMA`](../setup/developer-guide.md/#database-reporting) set to `TRUE`, it's SQL_SU_DML_STATEMENTS, SQL_SU_TABLE_DEFINITION, SQL_SU_PRIVILEGE_DEFINITION, SQL_SU_INDEX_DEFINITION | no |
| SQL_AGGREGATE_FUNCTIONS | SQL_AF_ALL | no |
| SQL_NUMERIC_FUNCTIONS | SQL_FN_NUM_ABS, SQL_FN_NUM_ATAN, SQL_FN_NUM_ATAN2 , SQL_FN_NUM_COS, SQL_FN_NUM_COT, SQL_FN_NUM_DEGREES, SQL_FN_NUM_FLOOR, SQL_FN_NUM_LOG, SQL_FN_NUM_LOG10, SQL_FN_NUM_PI, SQL_FN_NUM_POWER, SQL_FN_NUM_RADIANS, SQL_FN_NUM_ROUND, SQL_FN_NUM_SIGN, SQL_FN_NUM_SIN, SQL_FN_NUM_SQRT, SQL_FN_NUM_TAN | no |
| SQL_STRING_FUNCTIONS | SQL_FN_STR_ASCII, SQL_FN_STR_LENGTH, SQL_FN_STR_LTRIM, SQL_FN_STR_REPLACE, SQL_FN_STR_RTRIM, SQL_FN_STR_SUBSTRING | no |
| SQL_TIMEDATE_FUNCTIONS | SQL_FN_TD_CURDATE, SQL_FN_TD_DAYOFMONTH, SQL_FN_TD_MONTH, SQL_FN_TD_MONTHNAME, SQL_FN_TD_NOW, SQL_FN_TD_YEAR | no |
| SQL_TIMEDATE_ADD_INTERVALS | SQL_FN_TSI_FRAC_SECOND,  SQL_FN_TSI_SECOND, SQL_FN_TSI_MINUTE, SQL_FN_TSI_HOUR, SQL_FN_TSI_DAY, SQL_FN_TSI_WEEK, SQL_FN_TSI_MONTH, SQL_FN_TSI_QUARTER, SQL_FN_TSI_YEAR | no |
| SQL_TIMEDATE_DIFF_INTERVALS | SQL_FN_TSI_FRAC_SECOND, SQL_FN_TSI_SECOND, SQL_FN_TSI_MINUTE, SQL_FN_TSI_HOUR, SQL_FN_TSI_DAY, SQL_FN_TSI_WEEK, SQL_FN_TSI_MONTH, SQL_FN_TSI_QUARTER, SQL_FN_TSI_YEAR | no |
| SQL_DATETIME_LITERALS | 0 (not supported) | no |
| SQL_SYSTEM_FUNCTIONS | SQL_FN_SYS_IFNULL | no |
| SQL_CONVERT_FUNCTIONS | SQL_FN_CVT_CAST | no |
| SQL_OJ_CAPABILITIES | SQL_OJ_LEFT, SQL_OJ_RIGHT, SQL_OJ_NOT_ORDERED, SQL_OJ_ALL_COMPARISON_OPS | no |
| SQL_POS_OPERATIONS | 0 (not supported) | no |
| SQL_SQL92_NUMERIC_VALUE_FUNCTIONS | 0 (not supported) | no |
| SQL_SQL92_STRING_FUNCTIONS | SQL_SSF_CONVERT, SQL_SSF_LOWER, SQL_SSF_UPPER, SQL_SSF_SUBSTRING, SQL_SSF_TRANSLATE, SQL_SSF_TRIM_BOTH, SQL_SSF_TRIM_LEADING, SQL_SSF_TRIM_TRAILING | no |
| SQL_SQL92_DATETIME_FUNCTIONS | SQL_SDF_CURRENT_DATE, SQL_SDF_CURRENT_TIME, SQL_SDF_CURRENT_TIMESTAMP | no |
| SQL_SQL92_VALUE_EXPRESSIONS | SQL_SVE_CASE, SQL_SVE_CAST | no |
| SQL_SQL92_PREDICATES | SQL_SP_BETWEEN, SQL_SP_COMPARISON, SQL_SP_IN, SQL_SP_ISNULL, SQL_SP_LIKE | no |
| SQL_SQL92_RELATIONAL_JOIN_OPERATORS | SQL_SRJO_CROSS_JOIN, SQL_SRJO_INNER_JOIN, SQL_SRJO_LEFT_OUTER_JOIN, SQL_SRJO_RIGHT_OUTER_JOIN | no |
| SQL_STATIC_CURSOR_ATTRIBUTES1 | SQL_CA1_NEXT, SQL_CA1_ABSOLUTE, SQL_CA1_RELATIVE, SQL_CA1_BOOKMARK, SQL_CA1_LOCK_NO_CHANGE, SQL_CA1_POS_POSITION, SQL_CA1_POS_REFRESH | no |
| SQL_STATIC_CURSOR_ATTRIBUTES2 | SQL_CA2_READ_ONLY_CONCURRENCY, SQL_CA2_CRC_EXACT | no |
| SQL_CONVERT_BIGINT | SQL_CVT_BIGINT, SQL_CVT_DOUBLE | no |
| SQL_CONVERT_BINARY | 0 (not supported) | no |
| SQL_CONVERT_BIT | 0 (not supported) | no |
| SQL_CONVERT_CHAR | 0 (not supported) | no |
| SQL_CONVERT_VARCHAR | SQL_CVT_VARCHAR | no |
| SQL_CONVERT_LONGVARCHAR | 0 (not supported) | no |
| SQL_CONVERT_WCHAR | 0 (not supported) | no |
| SQL_CONVERT_WVARCHAR | 0 (not supported) | no |
| SQL_CONVERT_WLONGVARCHAR | 0 (not supported) | no |
| SQL_CONVERT_GUID | 0 (not supported) | no |
| SQL_CONVERT_DATE | SQL_CVT_DATE | no |
| SQL_CONVERT_DECIMAL | 0 (not supported) | no |
| SQL_CONVERT_DOUBLE | SQL_CVT_INTEGER, SQL_CVT_BIGINT, SQL_CVT_DOUBLE | no |
| SQL_CONVERT_FLOAT | 0 (not supported) | no |
| SQL_CONVERT_REAL | 0 (not supported) | no |
| SQL_CONVERT_INTEGER | SQL_CVT_INTEGER, SQL_CVT_BIGINT, SQL_CVT_DOUBLE | no |
| SQL_CONVERT_NUMERIC | 0 (not supported) | no |
| SQL_CONVERT_SMALLINT | 0 (not supported) | no |
| SQL_CONVERT_TINYINT | 0 (not supported) | no |
| SQL_CONVERT_TIME | SQL_CVT_TIME | no |
| SQL_CONVERT_TIMESTAMP | SQL_CVT_TIMESTAMP | no |
| SQL_CONVERT_INTERVAL_DAY_TIME | 0 (not supported) | no |
| SQL_CONVERT_INTERVAL_YEAR_MONTH | 0 (not supported) | no |
| SQL_CONVERT_VARBINARY | 0 (not supported) | no |
| SQL_CONVERT_LONGVARBINARY | 0 (not supported) | no |
| SQL_PARAM_ARRAY_ROW_COUNTS | SQL_PARC_BATCH | no |
| SQL_PARAM_ARRAY_SELECTS | SQL_PAS_NO_BATCH | no |
| SQL_SCROLL_OPTIONS | SQL_SO_FORWARD_ONLY, SQL_SO_STATIC | no |
| SQL_ALTER_DOMAIN | 0 (not supported) | no |
| SQL_ALTER_TABLE | 0 (not supported) | no |
| SQL_CREATE_ASSERTION | 0 (not supported) | no |
| SQL_CREATE_CHARACTER_SET | 0 (not supported) | no |
| SQL_CREATE_COLLATION | 0 (not supported) | no |
| SQL_CREATE_DOMAIN | 0 (not supported) | no |
| SQL_CREATE_SCHEMA | 0 (not supported) | no |
| SQL_CREATE_TABLE | 0 (not supported) | no |
| SQL_CREATE_TRANSLATION | 0 (not supported) | no |
| SQL_CREATE_VIEW | 0 (not supported) | no |
| SQL_CURSOR_COMMIT_BEHAVIOR | SQL_CB_CLOSE (Set on Linux/Apple only) | no |
| SQL_CURSOR_ROLLBACK_BEHAVIOR | SQL_CB_PRESERVE (Set on Linux/Apple only) | no |
| SQL_CURSOR_SENSITIVITY | SQL_INSENSITIVE | no |
| SQL_DDL_INDEX | SQL_DI_CREATE_INDEX, SQL_DI_DROP_INDEX | no |
| SQL_DEFAULT_TXN_ISOLATION | 0 (not supported) | no |
| SQL_DROP_ASSERTION | 0 (not supported) | no |
| SQL_DROP_CHARACTER_SET | 0 (not supported) | no |
| SQL_DROP_COLLATION | 0 (not supported) | no |
| SQL_DROP_DOMAIN | 0 (not supported) | no |
| SQL_DROP_SCHEMA | 0 (not supported) | no |
| SQL_DROP_TABLE | 0 (not supported) | no |
| SQL_DROP_TRANSLATION | 0 (not supported) | no |
| SQL_DROP_VIEW | 0 (not supported) | no |
| SQL_DYNAMIC_CURSOR_ATTRIBUTES1 | 0 (not supported) | no |
| SQL_DYNAMIC_CURSOR_ATTRIBUTES2 | 0 (not supported) | no |
| SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1 | SQL_CA1_NEXT | no |
| SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2 | SQL_CA2_READ_ONLY_CONCURRENCY, SQL_CA2_CRC_EXACT | no |
| SQL_INDEX_KEYWORDS | SQL_IK_NONE | no |
| SQL_INFO_SCHEMA_VIEWS | 0 (not supported) | no |
| SQL_INSERT_STATEMENT | 0 (not supported) | no |
| SQL_KEYSET_CURSOR_ATTRIBUTES1 | 0 (not supported) | no |
| SQL_KEYSET_CURSOR_ATTRIBUTES2 | 0 (not supported) | no |
| SQL_MAX_ASYNC_CONCURRENT_STATEMENTS | 0 (unknown) | no |
| SQL_MAX_BINARY_LITERAL_LEN | 0 (no maximum) | no |
| SQL_MAX_CATALOG_NAME_LEN | 0 (no maximum) | no |
| SQL_MAX_CHAR_LITERAL_LEN | 0 (no maximum) | no |
| SQL_MAX_INDEX_SIZE | 0 (no maximum) | no |
| SQL_MAX_ROW_SIZE | 0 (no maximum) | no |
| SQL_MAX_STATEMENT_LEN | 0 (no maximum) | no |
| SQL_SQL92_FOREIGN_KEY_DELETE_RULE | 0 (not supported) | no |
| SQL_SQL92_FOREIGN_KEY_UPDATE_RULE | 0 (not supported) | no |
| SQL_SQL92_GRANT | 0 (not supported) | no |
| SQL_SQL92_REVOKE | 0 (not supported) | no |
| SQL_SQL92_ROW_VALUE_CONSTRUCTOR | SQL_SRVC_VALUE_EXPRESSION, SQL_SRVC_NULL | no |
| SQL_STANDARD_CLI_CONFORMANCE | 0 (not supported) | no |
| SQL_SUBQUERIES | SQL_SQ_QUANTIFIED, SQL_SQ_IN, SQL_SQ_EXISTS, SQL_SQ_COMPARISON | no |
| SQL_TXN_ISOLATION_OPTION | 0 (not supported) | no |
| SQL_UNION | SQL_U_UNION, SQL_U_UNION_ALL | no |
| SQL_FETCH_DIRECTION | SQL_FD_FETCH_NEXT, SQL_FD_FETCH_FIRST, SQL_FD_FETCH_LAST, SQL_FD_FETCH_PRIOR, SQL_FD_FETCH_ABSOLUTE, SQL_FD_FETCH_RELATIVE, SQL_FD_FETCH_BOOKMARK | no |
| SQL_LOCK_TYPES | SQL_LCK_NO_CHANGE | no |
| SQL_ODBC_API_CONFORMANCE | SQL_OAC_LEVEL1 | no |
| SQL_ODBC_SQL_CONFORMANCE | SQL_OSC_CORE | no |
| SQL_POSITIONED_STATEMENTS | 0 (not supported) | no |
| SQL_SCROLL_CONCURRENCY | SQL_SCCO_READ_ONLY | no |
| SQL_STATIC_SENSITIVITY | 0 (not supported) | no |
| SQL_MAX_CONCURRENT_ACTIVITIES | 0 (no limit) | no |
| SQL_TXN_CAPABLE | SQL_TC_NONE | no |
| SQL_QUOTED_IDENTIFIER_CASE | SQL_IC_SENSITIVE | no |
| SQL_ACTIVE_ENVIRONMENTS | 0 (no limit) | no |
| SQL_CONCAT_NULL_BEHAVIOR | SQL_CB_NULL | no |
| SQL_CORRELATION_NAME | SQL_CN_ANY | no |
| SQL_FILE_USAGE | SQL_FILE_NOT_SUPPORTED | no |
| SQL_GROUP_BY | SQL_GB_GROUP_BY_CONTAINS_SELECT | no |
| SQL_IDENTIFIER_CASE | SQL_IC_SENSITIVE | no |
| SQL_MAX_COLUMN_NAME_LEN | 64 | no |
| SQL_MAX_COLUMNS_IN_GROUP_BY | 0 (no limit) | no |
| SQL_MAX_COLUMNS_IN_INDEX | 0 (no limit) | no |
| SQL_MAX_COLUMNS_IN_ORDER_BY | 0 (no limit) | no |
| SQL_MAX_COLUMNS_IN_SELECT | 0 (no limit) | no |
| SQL_MAX_COLUMNS_IN_TABLE | 0 (no limit) | no |
| SQL_MAX_CURSOR_NAME_LEN | 32 | no |
| SQL_MAX_DRIVER_CONNECTIONS | 0 (no limit) | no |
| SQL_MAX_IDENTIFIER_LEN | 64 | no |
| SQL_MAX_PROCEDURE_NAME_LEN | 0 (no limit) | no |
| SQL_MAX_SCHEMA_NAME_LEN | 64 | no |
| SQL_MAX_TABLE_NAME_LEN | 64 | no |
| SQL_MAX_TABLES_IN_SELECT | 0 (no limit) | no |
| SQL_MAX_USER_NAME_LEN | 0 (no limit) | no |
| SQL_NON_NULLABLE_COLUMNS | SQL_NNC_NON_NULL | no |
| SQL_NULL_COLLATION | SQL_NC_HIGH | no |

## Supported Connection Attributes
| Connection Information Types | Default | Support Value Change|
|--------|------|-------|
| SQL_ATTR_ANSI_APP | SQL_ERROR | no |
| SQL_ATTR_ASYNC_ENABLE | SQL_ASYNC_ENABLE_OFF | no |
| SQL_ATTR_AUTO_IPD | false | no |
| SQL_ATTR_AUTOCOMMIT | true | yes |
| SQL_ATTR_CONNECTION_DEAD | - | no |
| SQL_ATTR_CONNECTION_TIMEOUT | 0 | no |
| SQL_ATTR_TSLOG_DEBUG | - | yes |
| SQL_ATTR_METADATA_ID | false | yes |

Note: SQL_ATTR_TSLOG_DEBUG is an internal connection attribute. It can be used to change logging level after a connection is established.

## Supported Connection Options for SQLSetConnectOption
| Connection Options |
|--------|
| SQL_BIND_TYPE |
| SQL_CONCURRENCY |
| SQL_CURSOR_TYPE |
| SQL_RETRIEVE_DATA |
| SQL_ROWSET_SIZE |
| SQL_AUTOCOMMIT |

Note: SQLSetConnectOption is an ODBC 2.x function. It also supports [connection attributes](#supported-connection-attributes) if it is called from an ODBC 3 application.

## Supported Connection Options for SQLGetConnectOption
| Connection Information Types | Default |
|--------|------|
| SQL_AUTOCOMMIT | - |

Note: SQLGetConnectOption is an ODBC 2.x function. It also supports [connection attributes](#supported-connection-attributes) if it is called from an ODBC 3 application.

## Supported Statements Attributes 
Table of statement attributes supported by the Amazon Timestream ODBC driver.\
In both `SQLSetStmtAttr` and `SQLGetStmtAttr`
| Statement attribute | Default | Support Value Change|
|--------|------|-------|
|SQL_ATTR_CONCURRENCY| SQL_CONCUR_READ_ONLY| no |
|SQL_ATTR_CURSOR_TYPE|SQL_CURSOR_FORWARD_ONLY| no |
|SQL_ATTR_RETRIEVE_DATA|SQL_RD_ON| no |
|SQL_ATTR_METADATA_ID|SQL_FALSE| yes |
|SQL_ATTR_PARAM_BIND_TYPE| SQL_BIND_BY_COLUMN | no |
|SQL_ATTR_ROW_ARRAY_SIZE| 1 | yes |
|SQL_ATTR_ROW_BIND_OFFSET_PTR| column bind offset pointer | yes |
|SQL_ATTR_ROW_BIND_TYPE| SQL_BIND_BY_COLUMN | no |
|SQL_ATTR_ROW_STATUS_PTR| row status pointer | yes| 
|SQL_ATTR_ROWS_FETCHED_PTR| row fetched pointer | yes |

Attributes that are only supported in `SQLGetStmtAttr`
| Statement attribute | Return value |
|--------|------|
|SQL_ATTR_APP_ROW_DESC| pointer to statement |
|SQL_ATTR_APP_PARAM_DESC| pointer to statement |
|SQL_ATTR_IMP_ROW_DESC| pointer to statement |
|SQL_ATTR_IMP_PARAM_DESC| pointer to statement |
|SQL_ATTR_CURSOR_SCROLLABLE| SQL_NONSCROLLABLE |
|SQL_ATTR_CURSOR_SENSITIVITY| SQL_INSENSITIVE |
|SQL_ATTR_ENABLE_AUTO_IPD|SQL_FALSE|
|SQL_ATTR_ROW_NUMBER| current row number, 0 if cannot be determined |

## Supported Statements Options for SQLGetStmtOption 
| Statement attribute | Return value |
|--------|------|
|SQL_BIND_TYPE| SQL_BIND_BY_COLUMN |
|SQL_CONCURRENCY| SQL_CONCUR_READ_ONLY |
|SQL_CURSOR_TYPE| SQL_CURSOR_FORWARD_ONLY |
|SQL_RETRIEVE_DATA| SQL_RD_ON |
|SQL_ROWSET_SIZE| number of rows in the rowset |

Note: SQLGetStmtOption is an ODBC 2.x function. It also supports [statement attributes](#supported-statements-attributes) if it is called from an ODBC 3 application.

## Supported Descriptor Fields 
Table of descriptor fields supported by the Amazon Timestream ODBC driver.\
Currently only ARD is supported, these are fields supported in `SQLSetDescField`
| Descriptor Field |
|--------|
|SQL_DESC_CONCISE_TYPE|
|SQL_DESC_DATA_PTR|
|SQL_DESC_DATETIME_INTERVAL_CODE|
|SQL_DESC_DATETIME_INTERVAL_PRECISION|
|SQL_DESC_INDICATOR_PTR|
|SQL_DESC_OCTET_LENGTH_PTR|
|SQL_DESC_OCTET_LENGTH|
|SQL_DESC_LENGTH|
|SQL_DESC_TYPE|

Currently only ARD is supported, these are fields supported in `SQLGetDescField`
| Descriptor Field | Default |
|--------|------|
|SQL_DESC_CONCISE_TYPE| SQL_C_DEFAULT |
|SQL_DESC_DATA_PTR| Null ptr |
|SQL_DESC_DATETIME_INTERVAL_CODE| No default |
|SQL_DESC_DATETIME_INTERVAL_PRECISION| No default |
|SQL_DESC_INDICATOR_PTR| Null ptr |
|SQL_DESC_LENGTH| No default |
|SQL_DESC_NUM_PREC_RADIX| No default |
|SQL_DESC_OCTET_LENGTH| No default |
|SQL_DESC_OCTET_LENGTH_PTR| Null ptr |
|SQL_DESC_PRECISION| No default |
|SQL_DESC_SCALE| No default |
|SQL_DESC_TYPE| SQL_C_DEFAULT |

## SQLColumns

The driver supports catalog patterns for SQLColumns for both ODBC ver 2.0 and ODBC ver 3.0. 
When `SQL_ATTR_METADATA_ID` is set to `false` (default), it means schema name, catalog name, column name, database name, and table name need to be treated as case-sensitive search patterns. Parameters passed as nullptr has same meaning as "%" (search pattern that match everything). Read more about search patterns [here](https://learn.microsoft.com/en-us/sql/odbc/reference/develop-app/pattern-value-arguments?view=sql-server-ver16).

When `SQL_ATTR_METADATA_ID` is set to `true`, it means schema name, catalog name, column name, database name and table name need to be treated as case-insensitive identifiers. In this case, if schema name, catalog name, column name, database name, or table name are passed as nullptr to the driver, then the driver would give HY009: invalid use of null pointer error. Read more about identifiers [here](https://learn.microsoft.com/en-us/sql/odbc/reference/develop-app/identifier-arguments?view=sql-server-ver16).

## SQLPrepare, SQLExecute and SQLExecDirect

To support BI tools that may use the SQLPrepare interface in auto-generated queries, the driver
supports the use of SQLPrepare. However, the use of parameters in queries (values left as ?) is not supported in SQLPrepare, SQLExecute and SQLExecDirect. 

Timestream does not support SQL queries with ";", so SQLExecDirect does work with SQL queries with ";" at the end. For the types of SQL queries supported by Timestream, visit the official Timestream query [language support page](https://docs.aws.amazon.com/timestream/latest/developerguide/reference.html).

## SQLTables
Similarly to SQLColumns, the driver supports catalog patterns for SQLTables for both ODBC ver 2.0 and ODBC ver 3.0. 
When `SQL_ATTR_METADATA_ID` is set to `false` (default), it means database name and table name need to be treated as case-sensitive search patterns. Parameters passed as nullptr has same meaning as "%" (search pattern that match everything). Read more about search patterns [here](https://learn.microsoft.com/en-us/sql/odbc/reference/develop-app/pattern-value-arguments?view=sql-server-ver16).

When `SQL_ATTR_METADATA_ID` is set to `true`, it means database name and table name need to be treated as case-insensitive identifiers. In this case, if database name/table name are passed as nullptr to the driver, then the driver would give HY009: invalid use of null pointer error. Read more about identifiers [here](https://learn.microsoft.com/en-us/sql/odbc/reference/develop-app/identifier-arguments?view=sql-server-ver16).

|`SQL_ATTR_METADATA_ID` is set to `true`| CatalogName null value allowed? | SchemaName null value allowed? | TableName null value allowed? 
|---------------------------------------|---------------------------------|--------------------------------|------------------------------|
|Driver supports catalog only ([`DATABASE_AS_SCHEMA`](../setup/developer-guide.md/#database-reporting) not set)      | no | yes | no |
|Driver supports schema only ([`DATABASE_AS_SCHEMA`](../setup/developer-guide.md/#database-reporting) set to `TRUE`) | yes | no | no |

## Database Reporting Differences Between Timestream JDBC Driver and ODBC Driver
| --- | [Timestream JDBC Driver](https://github.com/awslabs/amazon-timestream-driver-jdbc) | Timestream ODBC Driver |
|-----|------------------------------------------------------------------------------------|------------------------|
| Database reporting | Databases reported as schemas. This behavior is not configurable. | Databases reported as catalogs by default. This behavior is configurable by setting [`DATABASE_AS_SCHEMA`](docs/markdown/setup/developer-guide.md/#database-reporting) environment variable
| Reasons for the design decisions  | Databases are reported as schemas for driver to work on Tableau, as Tableau will not include database names in auto-generated queries if databases are reported as catalogs, and Timestream server does not work with queries without database names. | Databases are reported as catalogs for driver to work on macOS Excel, as not all databases show properly on macOS Excel when databases are reported as schemas.

## Timestream Data Types
Timestream SQL support scalar types int, bigint, boolean, double, varchar, date, time, timestamp, interval_year_month and interval_day_second. Their values could be fetched based using their corresponding SQL data types or as a string/unicode string.

Besides regular data types, complex data types are also supported. Complex types include Array, Row and Timeseries. These complex types data could only be fetched as a string using our driver. 

### Timeseries format
Wrapped in "[]". Each item is wrapped in "{}" and separated from each other by ",". 

#### Example
```
"[{time: 2019-12-04 19:00:00.000000000, value: 35.2},{time: 2019-12-04 19:01:00.000000000, value: 38.2},{time: 2019-12-04 19:02:00.000000000, value: 45.3}]";
```

### Array Format
Wrapped in "[]". Each element is separated by ",".

#### Example
```
[1,2,3]
```
### Row Format
Wrapped in "()". Each element is separated by ",".

#### Example
```
(1,2,3)
```

For null it could only be fetched as a string and the result is "-".

Please refer to [Timestream Data Types](https://docs.aws.amazon.com/timestream/latest/developerguide/supported-data-types.html) for more info.

## Microsoft Excel on macOS
Timestream uses Unicode character set, but Microsoft Excel on macOS is using SQLCHAR to fetch strings instead of SQLWCHAR. Timestream ODBC driver therefore does a SQLWCHAR to SQLCHAR conversion for characters when used with Microsoft Excel on macOS. The unmapped characters from Unicode to ANSI is displayed as "?". 