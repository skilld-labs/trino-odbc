# Tableau - Connecting to an Amazon Trino ODBC data source

## Topics
- [Tableau on Windows](#windows)
- [Tableau on MacOS](#macos)

# Windows

## Prerequisites
- Before you get started, [configure the Amazon Trino ODBC driver DSN](windows-dsn-configuration.md) using the Windows ODBC Data Source Administrator. 

- Amazon Trino ODBC driver reports databases as catalogs by default. But Tableau includes Trino database name in auto-generated queries only if the databases are reported as schemas. The mismatch leads to data not being able to be loaded properly in Tableau. To make Tableau work properly, environment variable [`DATABASE_AS_SCHEMA`](developer-guide.md#database-reporting) needs to be set to `TRUE` before Tableau is started. For details, see [odbc-support-and-limitations](docs/markdown/support/odbc-support-and-limitations.md#sqltables).

## Connecting to an ODBC data source

1. Open Tableau

2. Under section "To a Server", click "Other Databases (ODBC)" -> choose pre-defined DSN (e.g., Trino DSN) -> press "Connect"

If a DSN window pops up at any time, just press the "Ok" button and continue

3. Click "Sign in"

4. You're now successfully connected on Tableau

# MacOS

## Prerequisites
- Before you get started, [configure the Amazon Trino ODBC driver DSN](macOS-dsn-configuration.md) by using the iODBC Driver Manager or editing `odbcinst.ini` file.

- Amazon Trino ODBC driver reports databases as catalogs by default. But Tableau includes Trino database name in auto-generated queries only if the databases are reported as schemas. The mismatch leads to data not being able to be loaded properly in Tableau. To make Tableau work properly, environment variable [`DATABASE_AS_SCHEMA`](developer-guide.md#database-reporting) needs to be set to `TRUE` before Tableau is started. For details, see [odbc-support-and-limitations](docs/markdown/support/odbc-support-and-limitations.md#sqltables).

## Connecting to an ODBC data source

1. Open Tableau

2. Under section "To a Server", click "Other Databases (ODBC)"

![Tableau (Mac)](../images/mac-tableau.png)

3. Choose pre-defined DSN (e.g., Trino-DSN) and click "Sign In"

![Tableau DSN (Mac)](../images/mac-tableau-odbc.png)

4. You're now successfully connected on Tableau
