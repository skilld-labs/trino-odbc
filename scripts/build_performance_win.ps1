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

$WORKING_DIR = $args[0] # Assumed to be root of driver repo
$SRC_DIR = "$WORKING_DIR\performance"
$WIN_ARCH = $args[1]
$BUILD_DIR = "$WORKING_DIR\performance\build"
$INSTALL_DIR = "$BUILD_DIR\bin"
if ($WIN_ARCH -eq "x64") {
    $VCPKG_TARGET_TRIPLET="x64-windows"
} else {
    $VCPKG_TARGET_TRIPLET="x86-windows"
}

if ($null -eq $env:VCPKG_ROOT) {
    $env:VCPKG_ROOT = 'c:/vcpkg'
}

# Create build directory; remove if exists
New-Item -Path $BUILD_DIR -ItemType Directory -Force | Out-Null

$CMAKE_TOOLCHAIN_FILE = -join($env:VCPKG_ROOT, "/scripts/buildsystems/vcpkg.cmake")

cmake -S $SRC_DIR `
    -A $WIN_ARCH `
    -B $BUILD_DIR `
    -D CMAKE_INSTALL_PREFIX=$INSTALL_DIR `
    -D CMAKE_BUILD_TYPE=Release `
    -D WITH_ODBC=ON `
    -D WITH_ODBC_MSI=ON `
    -D VCPKG_TARGET_TRIPLET=$VCPKG_TARGET_TRIPLET `
    -D CMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE

if ($?) {
    # Build Project
    cmake --build $BUILD_DIR --target install --config Release --parallel 4 
}

# Check if either the first CMake command or the second CMake command fails
$RET_CODE = $LastExitCode

if ( $RET_CODE -ne 0 ) {
   Write-Output "Error occurred while building project. Exiting."
   exit $RET_CODE
}

