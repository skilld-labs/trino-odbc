# Amazon Trino ODBC Driver

## Overview

The ODBC driver for the Amazon Trino serverless time series database provides an 
SQL-relational interface for developers and BI tool users.

Full source code is provided. Users should be able build the library for intended platform.

For build instructions please refer to [developer guide](docs/markdown/setup/developer-guide.md).

For details on ODBC driver installation and usage please refer to rest of this document.

## Security

See [CONTRIBUTING](CONTRIBUTING.md#security-issue-notifications) for more information.

## License

This project is licensed under the Apache-2.0 License.

## Documentation

See the [product documentation](docs/markdown/index.md) for more detailed information about this driver, such as setup and configuration.

## Setup and Usage

To setup and use the Trino ODBC driver, follow [these directions](docs/markdown/setup/setup.md).

See [Azure Active Directory set-up guide](/docs/markdown/setup/aad-saml-setup.md) for setting up Azure AD account and connecting to Trino. See [Okta set-up guide](/docs/markdown/setup/Okta-setup.md) for setting up Okta account and connecting to Trino.

See [PowerBI oveview](/docs/markdown/setup/powerbi-connector-setup/powerbi_overview.md) for using PowerBI with Trino ODBC driver and connector.

See [performance testing setup](performance/README.md) for setting up and running performance tests.

## Connection String Syntax

```
DRIVER={Amazon Trino ODBC Driver};DSN={Trino DSN};<option>=<value>;
```

For more information about connecting to an Amazon Trino database using this ODBC driver, see
the [connection string documentation](docs/markdown/setup/connection-string.md) for more details.

## Driver Info
| Platform | Amazon Trino ODBC Driver File 
|----------|-----------------------------------|
| Windows | trino.odbc.dll
| macOS | libtrino-odbc.dylib
| Linux | libtrino-odbc.so

## Known Issues
1. Trino does not support fully qualified table names. Trino ODBC driver reports databases as catalogs instead of schemas by default, as a result, tools like Tableau may not work as expected. For example, Tableau does not include database name in the auto-generated queries to fetch data if databases are reported as catalogs. Users are recommended to use [Trino JDBC driver](https://github.com/awslabs/amazon-trino-driver-jdbc) with Tableau. To make the Trino ODBC driver work with Tableau, the user can set environment variable `DATABASE_AS_SCHEMA` to `TRUE` to make the driver report databases as schemas instead. For details, see [odbc-support-and-limitations](docs/markdown/support/odbc-support-and-limitations.md#sqltables).

2. The Trino ODBC driver is a unicode driver and will convert non-ANSI characters from data sources to ANSI; the environment variable `ANSI_STRING_ONLY` can be set to `TRUE` to indicate a data source only uses ANSI characters and skip conversion. For details, see [unicode-support](docs/markdown/support/unicode-support.md).
