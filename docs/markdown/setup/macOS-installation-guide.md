# Amazon Trino ODBC Driver Setup on MacOS

## Supported versions
macOS 11 (Big Sur) and above on 64-bit Intel processor. Apple M1 is not currently supported.

## Prerequisites
In order to use the Trino ODBC Driver, [libiodbc](https://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/WelcomeVisitors) must be installed.
```
brew install libiodbc
```

## Install Trino ODBC Driver
1. Download the Trino ODBC driver installer AmazonTrinoODBC-[version].pkg.
2. Double-click the installer to install by GUI or use the following command line to install it directly.
```
sudo installer -pkg AmazonTrinoODBC-[version].pkg -target /
```
3. Follow the instructions and finish the installation if you install by GUI.

## Next Steps

- [Set up the DSN](macOS-dsn-configuration.md)

## Uninstall Trino ODBC Driver
There is no automatical way to uninstall it. It has to be done manually.
1. Remove "Amazon Trino ODBC Driver" entry from /Library/ODBC/odbcinst.ini
2. Remove the driver residence direcotry /Library/ODBC/trino-odbc
```
sudo rm -rf /Library/ODBC/trino-odbc
```
