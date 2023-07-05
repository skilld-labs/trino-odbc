#!/bin/bash

# Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# A copy of the License is located at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# or in the "license" file accompanying this file. This file is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.

#The following line will be replaced by sed in src/odbc/CMakeList.txt 
#to different actual path when build the installer for 32-bit and 64-bit
#ODBC_LIB_PATH

if [[ "$OSTYPE" == "linux"* ]]; then
  ODBC_LIB_FILENAME="$ODBC_LIB_PATH/libtimestream-odbc.so"
elif [[ "$OSTYPE" == "darwin"* ]]; then
  ODBC_LIB_FILENAME="$ODBC_LIB_PATH/libtimestream-odbc.dylib"
fi

if [ ! -f "$ODBC_LIB_FILENAME" ]
then
  echo "Cannot find ODBC library file: $ODBC_LIB_FILENAME"
  exit 1
fi

ODBC_INSTALL_INI="$ODBC_LIB_PATH/timestream-odbc-install.ini"

echo "[Amazon Timestream ODBC Driver]"            > $ODBC_INSTALL_INI
echo "Description=Amazon Timestream ODBC Driver" >> $ODBC_INSTALL_INI
echo "Driver=$ODBC_LIB_FILENAME" >> $ODBC_INSTALL_INI
echo "Setup=$ODBC_LIB_FILENAME"  >> $ODBC_INSTALL_INI
echo "DriverODBCVer=03.00"       >> $ODBC_INSTALL_INI
echo "FileUsage=0"               >> $ODBC_INSTALL_INI

if [[ "$OSTYPE" == "linux"* ]]; then
  odbcinst -i -d -f $ODBC_INSTALL_INI
elif [[ "$OSTYPE" == "darwin"* ]]; then
  export ODBCINSTINI=$ODBC_INSTALL_INI
  echo "Exported ODBCINSTINI=$ODBCINSTINI"
  export DYLD_LIBRARY_PATH=$ODBC_LIB_PATH:$DYLD_LIBRARY_PATH
  echo "Exported DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH"
fi
