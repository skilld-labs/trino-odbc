# Amazon Timestream ODBC Driver Setup on MacOS

## Supported versions
macOS 11 (Big Sur) and above on 64-bit Intel processor. Apple M1 is not currently supported.

## Prerequisites
In order to use the Timestream ODBC Driver, [libiodbc](https://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/WelcomeVisitors) must be installed.
```
brew install libiodbc
```

## Install Timestream ODBC Driver
1. Download the Timestream ODBC driver installer AmazonTimestreamODBC-[version].pkg.
2. Double-click the installer to install by GUI or use the following command line to install it directly.
```
sudo installer -pkg AmazonTimestreamODBC-[version].pkg -target /
```
3. Follow the instructions and finish the installation if you install by GUI.

## Next Steps

- [Set up the DSN](macOS-dsn-configuration.md)

## Uninstall Timestream ODBC Driver
There is no automatical way to uninstall it. It has to be done manually.
1. Remove "Amazon Timestream ODBC Driver" entry from /Library/ODBC/odbcinst.ini
2. Remove the driver residence direcotry /Library/ODBC/timestream-odbc
```
sudo rm -rf /Library/ODBC/timestream-odbc
```
