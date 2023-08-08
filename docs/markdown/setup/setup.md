# Amazon Timestream ODBC Driver Setup

## Topics
- [Prerequisites](#prerequisites)
    - [AWS account](#aws-account)
    - [Timestream ODBC Driver](#timestream-odbc-driver)
- [DSN Configuration](#dsn-configuration)
- [Driver Setup in BI Applications](#driver-setup-in-bi-applications)
    
## Prerequisites

### AWS Account
An AWS IAM account is required to connect to Timestream using the Timestream ODBC Driver. 

You need to do the following:

1. [Sign up for AWS](https://docs.aws.amazon.com/timestream/latest/developerguide/accessing.html#SettingUp.Q.SignUpForAWS).

2. Create an IAM user with Timestream access and use the IAM credentials with the driver. For more information, see [Create an IAM user with Timestream access](https://docs.aws.amazon.com/timestream/latest/developerguide/accessing.html#getting-started.prereqs.iam-user).

### Timestream ODBC Driver Download
Download the Timestream ODBC driver [here](https://github.com/awslabs/amazon-timestream-odbc-driver/releases). Choose the proper installer
(e.g., `timestream-odbc-installer-amd64-2.0.0.exe`). 

### Timestream ODBC Driver Installation
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
