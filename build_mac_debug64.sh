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

#build AWS SDK static library
cd src
git clone --recurse-submodules -b "1.11.80" "https://github.com/aws/aws-sdk-cpp.git"
cd aws-sdk-cpp
mkdir install
mkdir build
cd build
cmake ../ -DCMAKE_INSTALL_PREFIX="../install" -DTARGET_ARCH="APPLE" -DCMAKE_BUILD_TYPE="Debug" -DBUILD_ONLY="core;sts;timestream-query;timestream-write" -DCUSTOM_MEMORY_MANAGEMENT="OFF" -DENABLE_TESTING="OFF" -DBUILD_SHARED_LIBS="OFF"
make -j 4
make install
cd ../../../

BUILD_DIR=cmake-build64
BUILD_TYPE=Debug
PROJECT_DIR=$(pwd)
DRIVER_BIN_DIR="$PROJECT_DIR/build/odbc/bin"
DRIVER_LOG_DIR="$PROJECT_DIR/build/odbc/logs"

mkdir $DRIVER_LOG_DIR

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ../src -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCODE_COVERAGE="ON" -DBUILD_SHARED_LIBS="OFF" -DWITH_TESTS="ON" -DWITH_ODBC="ON"
make  -j 4

RET_CODE=$?

if [ $RET_CODE -ne 0 ]; then
   echo "Error occurred while building macOS x64 project. Exiting."
   exit $RET_CODE
fi

make package

RET_CODE=$?

if [ $RET_CODE -ne 0 ]; then
   echo "Error occurred while building macOS x64 installer. Exiting."
   exit $RET_CODE
fi

cd ..

