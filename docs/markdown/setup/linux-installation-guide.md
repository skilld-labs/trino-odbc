# Amazon Timestream ODBC Driver Setup on Linux 

## Prerequisites
In order to use the Timestream ODBC Driver, [unixODBC](http://www.unixodbc.org/) must be installed.

### Installing on Ubuntu 64 bit

```
sudo apt update
sudo apt install unixodbc
```

### Installing on Ubuntu 32 bit

```
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install unixodbc:i386
```

### Installing on openSUSE 32 bit
```
sudo zypper refresh
sudo zypper install unixODBC
```

### Installing on Amazon Linux 2 64 bit

```
sudo yum update
sudo yum install unixODBC
```

## Install Timestream ODBC Driver
1. Download the Timestream ODBC installer deb package according to your Linux distro and architecture.
2. Install the package

    #### Sample command for Debian 64 bit 
    ```
    sudo dpkg -i AmazonTimestreamODBC_2.0.0_RELEASE_amd64.deb
    ```

    #### Sample command for Debian 32 bit 
    ```
    sudo dpkg -i AmazonTimestreamODBC_2.0.0_RELEASE_i386.deb
    ```

    #### Sample command for RPM based 64 bit
    ```
    sudo rpm -i AmazonTimestreamODBC_2.0.0_RELEASE_x86_64.rpm
    ```

    #### Sample command for RPM based 32 bit
    ```
    sudo rpm -i AmazonTimestreamODBC_2.0.0_RELEASE_i386.rpm
    ```

3. Register the ODBC driver by running the following script.
    #### Command for Linux 32 bit
    ```
    sudo /usr/lib/timestream-odbc32/postinst_unix32.sh
    ```

    #### Command for Linux 64 bit
    ```
    sudo /usr/lib64/timestream-odbc64/postinst_unix64.sh
    ```

## Next Steps

- [Set up the DSN](linux-dsn-configuration.md)

## Uninstall Timestream ODBC Driver
There is no automatical way to uninstall it. It has to be done manually.
1. Remove the package from repository

    #### Command for RPM based 64 bit
    ```
    sudo zypper remove amazontimestreamodbc_2.0.0_release_x86_64
    ```
    or

    ```
    sudo yum remove amazontimestreamodbc_2.0.0_release_x86_64
    ```

    #### Command for RPM based 32 bit
    ```
    sudo zypper remove amazontimestreamodbc_2.0.0_release_i386
    ```
    or

    ```
    sudo yum remove amazontimestreamodbc_2.0.0_release_i386
    ```

    #### Command for Debian 64 bit
    ```
    sudo apt remove amazontimestreamodbc_2.0.0_release_amd64
    ```

    #### Command for Debian 32 bit
    ```
    sudo apt remove amazontimestreamodbc_2.0.0_release_i386
    ```

2. If "Amazon Timestream ODBC Driver" entry still exists in /etc/odbcinst.ini or /etc/unixODBC/odbcinst.ini, remove it.
3. Remove the driver residence directory
    #### Command for Linux 32 bit
    ```
    sudo rm -rf /usr/lib/timestream-odbc32
    ```

    #### Command for Linux 64 bit
    ```
    sudo rm -rf /usr/lib64/timestream-odbc64
    ```

4. Remove the libraries installed to /usr/lib if they exist
    ```
    sudo rm /usr/lib/libtimestream-odbc.so /usr/lib/libtimestream-odbc.so.2.0.0
    ```



## Notes
The following table lists the package build platform and its linux kernel version. The packages could only be used on Linux with equal or higher kernel version than the package is built on. If the package is used on a Linux which has a different Linux distribution with the build platform, the driver may not be installed successfully or work properly. The solution for this problem is to setup development environment and build the driver from source code on the system the driver will be used. If you need an ODBC Driver built for a platform not listed below, please raise a GitHub Issue.

```
-----------------------------------------------------------------------------------
|        Package           |      Build Platform       |    Linux Kernel Version  |
-----------------------------------------------------------------------------------
| Linux 32 bit deb package |  Ubuntu 18.04 LTS 32-bit  |         5.4.0            |        
-----------------------------------------------------------------------------------
| Linux 64 bit deb package |     Ubuntu 22.04 LTS      |         5.15.0           |
-----------------------------------------------------------------------------------
| Linux 32 bit rpm package |  Ubuntu 18.04 LTS 32-bit  |         5.4.0            |
-----------------------------------------------------------------------------------
| Linux 64 bit rpm package |       Amazon Linux 2      |         5.10.135         |
-----------------------------------------------------------------------------------
```
