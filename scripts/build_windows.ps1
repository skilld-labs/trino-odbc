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

# Build AWS SDK
$CURRENT_DIR = Get-Location

# WORKING_DIR would be the root to odbc repo
$WORKING_DIR = $args[0]
$CONFIGURATION = $args[1]
$BITNESS = $args[2]
if ($BITNESS -eq "64") {
    $WIN_ARCH = "x64"
}
else {
    $WIN_ARCH = "Win32"
}

# Create build directory; remove if exists
$BUILD_DIR = "${WORKING_DIR}\build"
# $BUILD_DIR = "${WORKING_DIR}\build\${CONFIGURATION}${BITNESS}"
New-Item -Path $BUILD_DIR -ItemType Directory -Force | Out-Null

$SDK_SOURCE_DIR = "${WORKING_DIR}\src\aws-sdk-cpp"
$SDK_BUILD_DIR = "${BUILD_DIR}\aws-sdk\build"
$SDK_INSTALL_DIR = "${BUILD_DIR}\aws-sdk\install"

.\scripts\build_aws-sdk-cpp.ps1 `
    $CONFIGURATION $WIN_ARCH `
    $SDK_SOURCE_DIR $SDK_BUILD_DIR $SDK_INSTALL_DIR
Set-Location $CURRENT_DIR

# Build driver
$DRIVER_SOURCE_DIR = "${WORKING_DIR}\src"
$DRIVER_BUILD_DIR = "${BUILD_DIR}\odbc\cmake"
$DRIVER_BIN_DIR = "${BUILD_DIR}\odbc\bin\$CONFIGURATION"

.\scripts\build_driver.ps1 `
    $CONFIGURATION $WIN_ARCH `
    $DRIVER_SOURCE_DIR $DRIVER_BUILD_DIR $DRIVER_BIN_DIR
Set-Location $CURRENT_DIR

# Move driver dependencies to bin directory for testing
New-Item -Path $DRIVER_BIN_DIR -ItemType Directory -Force | Out-Null

if (Test-Path -Path $DRIVER_BUILD_DIR\$CONFIGURATION) {
    Copy-Item $DRIVER_BUILD_DIR\$CONFIGURATION\* $DRIVER_BIN_DIR -force -recurse
}
