# Metadata Design

## How column metadata for query execution works
Below is a diagram of how column metadata (type ColumnMeta) is populated using DataQuery class functions.

```mermaid
graph TD
    A[DataQuery::Execute] -->|if metadata not available, eventually call| B[DataQuery::ReadTSColumnMetadataVector]
    G[DataQuery::MakeRequestResultsetMeta] --> B
    B --> |populates| C(column metadata)
    C -->|With| D(Column name)
    C -->|With| E(Column data type)
```
## How table metadata works
Below is a diagram of how table metadata (type TableMeta) is populated using TableMetadataQuery class functions. 

`MetadataId` in the graph is referring to `SQL_ATTR_METADATA_ID`, a supported [connection attribute](odbc-support-and-limitations.md/#supported-connection-attributes) which affects [SQLTables](odbc-support-and-limitations.md/#sqltables). 

```mermaid
graph TD
    a[SQLTables] --> A
    A[TableMetadataQuery::Execute] --> G
    G[TableMetadataQuery::MakeRequestGetTablesMeta] 
    G --> |MetadataId true| B[filter by exact match with case-insensitive identifiers]
    G --> |MetadataId false| C[filter by case-sensitive search patterns]
    C --> D[getTablesWithSearchPattern]
    B --> E[getTablesWithIdentifier]
    F(getMatchedDatabases, called by getTablesWithSearchPattern)
    H(getMatchedTables, called by getTablesWithSearchPattern)
    D --> |1. Get database names that matches database pattern| F
    F --> |2. Get table names matches table pattern| H

    I(getMatchedDatabases, called by getTablesWithIdentifier)
    J(getMatchedTables, called by getTablesWithIdentifier)
    E --> |1. Get all database names|I
    I --> K[Filter database names based on database identifier]
    K --> |2. Get all table names| J
    J --> M[Filter table names based on table identifier]
```

