# Amazon Trino ODBC Driver Setup

## Topics
- [Prerequisites](#prerequisites)
    - [AWS account](#aws-account)
    - [Trino ODBC Driver](#trino-odbc-driver)
- [DSN Configuration](#dsn-configuration)
- [Driver Setup in BI Applications](#driver-setup-in-bi-applications)
    
## Prerequisites

### AWS Account
An AWS IAM account is required to connect to Trino using the Trino ODBC Driver. 

You need to do the following:

1. [Sign up for AWS](https://docs.aws.amazon.com/trino/latest/developerguide/accessing.html#SettingUp.Q.SignUpForAWS).

2. Create an IAM user with Trino access and use the IAM credentials with the driver. For more information, see [Create an IAM user with Trino access](https://docs.aws.amazon.com/trino/latest/developerguide/accessing.html#getting-started.prereqs.iam-user).

### Trino ODBC Driver Download
Download the Trino ODBC driver [here](https://github.com/awslabs/amazon-trino-odbc-driver/releases). Choose the proper installer
(e.g., `trino-odbc-installer-amd64-2.0.0.exe`). 

### Trino ODBC Driver Installation
- [Windows Installation Guide](windows-installation-guide.md)
- [MacOS Installation Guide](macOS-installation-guide.md)
- [Linux Installation Guide](linux-installation-guide.md)

### DSN Configuration
Click on the following links for DSN configuration help in specifc platforms.

- [Windows DSN](windows-dsn-configuration.md)
- [MacOS DSN](macOS-dsn-configuration.md)
- [Linux DSN](linux-dsn-configuration.md)

## Driver Setup in BI Applications
The ODBC driver is compatible with a number of BI tools. Instructions are outlined here for:
1. [Microsoft Power BI](microsoft-power-bi.md)
2. [Microsoft Excel](microsoft-excel.md)
2. [Tableau](tableau.md)

For other BI tools, please refer to that tool's product documentation.
