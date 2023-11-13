# Amazon Trino ODBC Driver Setup on Windows

## Supported versions
Windows 10 22H2, Windows 11 and Windows Server 2022

`14.34.31938.0` Microsoft Visual C++ Redistributable (MSVC). Other versions of MSVC packages may work, but they are not tested with Trino ODBC Driver.

## Install Trino ODBC Driver
1. Download the Trino ODBC installer exe.
2. Double-click the installer
3. Follow the instructions and finish the installation.

## Notes and Limitations
### Microsoft Visual C++ Redistributable (MSVC)
Microsoft Visual C++ Redistributable (MSVC) Package version 14 will be installed automatically if you do not have already installed in your System.

If you already have the MSVC Package installed in your system, the Trino ODBC driver installer will **not** 
upgrade the package for you. If you want to upgrade the package to the MSVC version that comes with the Trino ODBC driver 
during driver installation, please uninstall the existing MSVC package first, and then run the driver installer.

After the Trino ODBC driver is uninstalled, MSVC package is kept as it might be needed by other applications. If it is not needed, user could manually uninstall it separately. 

### Installation Over Pre-Existing Deployment Is Not Supported
The installer doesn't allow installation over pre-existing deployment. If a user would like to install a driver with newer or older 
release and there is already a Trino ODBC driver installed on their machine, then the user will need to manually uninstall the 
existing driver before attempting to install a driver with a different version. 

## Debug Version Installer
When there is a need for debug version driver, user could build a debug version installer using `build_win_debug32.ps1` or `build_win_debug64.ps1` following [developer guide](developer-guide.md#windows). If the debug version driver will be used on another machine, it needs Microsoft Visual Studio to work successfully. The latest MSVC, C++ ATL and Windows SDK need to be installed as part of Desktop Development for C++ in Visual Studio.

## Next Steps

- [Set up the DSN](windows-dsn-configuration.md)
