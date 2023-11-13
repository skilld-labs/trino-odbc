# Troubleshooting Guide

## Topics
- [Logs](#logs)
- [PowerBI Desktop cannot load the Trino ODBC driver library](#powerbi-desktop-cannot-load-the-trino-odbc-driver-library)
- [Cannot connect on Linux using user DSN](#cannot-connect-on-linux-using-user-dsn)
- [Root cause of "INVALID_ENDPOINT: Failed to discover endpoint"](#root-cause-of-invalid_endpoint-failed-to-discover-endpoint)
- [Driver has issue with C++ Redistributable package on Windows](#driver-has-issue-with-c-redistributable-package-on-windows)

## Logs

When troubleshooting, it can be helpful to view the logs so that you might be able 
to resolve the issue on your own or at least have more context to provide when seeking support. 
On Windows, the driver's logs will be written to the user's home directory (`%USERPROFILE%` or `%HOMEDRIVE%%HOMEPATH%`) by default.
On Mac/Linux/Unix, the default log path is also the user's home directory(`$HOME` or `getpwuid(getuid())->pw_dir`), whichever one is available.
 - `getpwuid(getuid())->pw_dir` and `$HOME` may contain different values depending on the system and application. For example, on macOS Excel, `$HOME` is `/Users/<username>/Library/Containers/com.microsoft.Excel/Data`, and `getpwuid(getuid())->pw_dir` is `/Users/<username>`

On Windows, you may change the default path in the DSN configuration window.
In any platform, you may pass your log path / log level in the connection string.
The log path indicates the path to store the log file. The log file name has `trino_odbc_YYYYMMDD.log` format, 
where `YYYYMMDD` (e.g., 20220225 <= Feb 25th, 2022)
is the timestamp at the first log message.
The keyword for log path is `logOutput` and the keyword for log level is `logLevel`. 

### Setting Logging Level and Location
There are the following levels of logging:

| Property Value | Description |
|--------|-------------|
| `1` (means ERROR) | Shows messages classified as ERROR.|
| `2` (means WARNING) | Shows messages classified as WARNING and ERROR.|
| `3` (means INFO) | Shows messages classified as INFO, WARNING and ERROR.|
| `4` (means DEBUG) | Shows messages classified as DEBUG, INFO, WARNING and ERROR. </br > **Warning:** personal information can be logged by the driver when using the driver in **DEBUG** mode.|
| `0` (means OFF) | No log messages displayed.|

| Property Name | Description | Platform | Default |
|--------|-------------|--------|---------------|
| `logLevel` | The log level for all sources/appenders. | All Platforms | `2` (means WARNING) |
| `logOutput` | The location for file logging. | Windows | `%USERPROFILE%`, or if not available, `%HOMEDRIVE%%HOMEPATH%` |
| `logOutput` | The location for file logging. | macOS (except Excel) | `$HOME`, or if not available, use the field `pw_dir` from C++ function `getpwuid(getuid())` return value  |
| `logOutput` | The location for file logging. | macOS Excel | `/Users/<username>/Library/Containers/com.microsoft.Excel/Data` |
| `logOutput` | The location for file logging. | Linux/Unix | `$HOME`, or if not available, use the field `pw_dir` from C++ function `getpwuid(getuid())` return value |

To set these properties, use the connection string with the following format 
`<property-name>=<property-value>`. The user should **not** have a slash at the end of the log path. 

For example: (Note: The capitalization does not matter.)
- In Windows, append `logOutput="C:\Users\Name\Desktop\Trino ODBC Driver";logLevel=4;` 
to your connection string to use log level DEBUG.
    * You can also set the log path and log level from the configuration window in the Microsoft ODBC Administrator. 
    * Click on the drop menu for setting the log level
    * Enter the desired log folder path in the field next to the label `Log Path`. The user needs to ensure that the directory mentioned in the log folder path does exist, or driver will ignore user's passed value and create the log file in the default log path.

- In MacOS/Linux/Unix, append `logOutput="~/path/to/log/file";logLevel=1;` to your connection string to use log level ERROR, or append
`logOutput` and `logLevel` as keywords in the ODBC manager. 

- If you just want to change the log level, append `logLevel=<desired-log-level>;` to your connection string.

### AWS Log File Location

Trino ODBC driver is using AWS SDK C++ to connect to AWS Trino. AWS SDK has its own log files. When there is a problem, you may need to access AWS SDK logs. The AWS SDK log file name has `aws_sdk_year-month-day-hour.log` format. The AWS SDK log location is your executable directory, that is where you run your application. 

Tips for AWS log file location
- ODBC Data Sources(64-bit) on Windows, it is `%windir%\system32\`.
- Excel on macOS, it is `/Users/<username>/Library/Containers/com.microsoft.Excel/Data`.

## PowerBI Desktop cannot load the Trino ODBC driver library

If you downloaded Power BI Desktop from the Microsoft Store, you may be unable to use the Amazon Trino ODBC driver due to a loading issue. To address this, download Power BI Desktop from the [Download Center](https://www.microsoft.com/download/details.aspx?id=58494) instead of the Microsoft Store.

## Cannot connect on Linux using user DSN

If you have set the user DSN on Linux in `~/.odbc.ini`, but cannot connect via `isql`, try creating a system DSN in `/etc/odbc.ini` instead. Different flavors of Linux recognize different DSN files, and user DSNs might not be supported on some flavors.

## Root cause of "INVALID_ENDPOINT: Failed to discover endpoint"

The error message `INVALID_ENDPOINT: Failed to discover endpoint` can occur for several reasons, including:

- Authentication failure. If invalid AWS Access Key Id or Secret Access Key is entered, this error message can appear.

- Incorrect region. If the region specified in the connection string or DSN configuration is different from the region the client is trying to access, this error message can appear.

- Incorrect endpoint. If the endpoint specified in the connection string or DSN configuration is incorrect, this error message can appear.

- Unavailable endpoint. If the endpoint is temporarily down or experiencing a service interruption, this error message can appear.

- Network connectivity issues. If there is a network issue that is preventing the client from connecting to the endpoint, this error message can appear.

- AWS Trino service issue. If there is a problem with the AWS Trino service that the client is trying to access, this error message can appear.

When this error message is seen, you can check the aws sdk log `aws_sdk_year_month_date-hour.log` (e.g., `aws_sdk_2023-02-01-23.log`)  in your executable directory and search the pattern `Failed to discover endpoints` to identify the real root cause. For example, the following log line shows the `Failed to discover endpoints` is caused by `The request signature we calculated does not match the signature you provided`, which means the credentials are invalid.

```
[ERROR] 2023-02-07 19:28:43.944 Query [140737321622080] Failed to discover endpoints HTTP response code: 403
Resolved remote host IP address: 44.233.100.2
Request ID: d0f595b2-c971-4718-b56b-c06d073d4e8a
Exception name: InvalidSignatureException
Error message: The request signature we calculated does not match the signature you provided. Check your AWS Secret Access Key and signing method. Consult the service documentation for details.
```

## Driver has issue with C++ Redistributable package on Windows
The Trino ODBC driver has only been tested with `14.34.31938.0` version of Microsoft Visual C++ Redistributable (MSVC) package, and untested versions may not work with the driver. If you encounter undocumented problems with the driver, please first uninstall the current version of MSVC package on your machine, and then reinstall the driver using the driver installer. The driver installer will install the version of MSVC package that works with the driver onto the user's computer. For details, please see [Windows installation guide](../setup/windows-installation-guide.md).
