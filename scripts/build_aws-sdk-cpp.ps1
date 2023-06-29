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

$CONFIGURATION = $args[0]
$WIN_ARCH = $args[1]
$SRC_DIR = $args[2]
$BUILD_DIR = $args[3]
$INSTALL_DIR = $args[4]

Write-Host $args

# Clone the AWS SDK CPP repo
git config --global core.longpaths true
git clone --recurse-submodules -b "1.11.80" "https://github.com/aws/aws-sdk-cpp.git" $SRC_DIR

# Make and move to build directory
New-Item -Path $BUILD_DIR -ItemType Directory -Force | Out-Null
Set-Location $BUILD_DIR

# Configure and build 
cmake $SRC_DIR `
    -A $WIN_ARCH `
    -D CMAKE_INSTALL_PREFIX=$INSTALL_DIR `
    -D CMAKE_BUILD_TYPE=$CONFIGURATION `
    -D BUILD_ONLY="core;sts;timestream-query;timestream-write" `
    -D ENABLE_UNITY_BUILD="ON" `
    -D CUSTOM_MEMORY_MANAGEMENT="OFF" `
    -D ENABLE_RTTI="OFF" `
    -D ENABLE_TESTING="OFF" `
    -D CPP_STANDARD="17"

# Build AWS SDK and install to $INSTALL_DIR 
$msbuild = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | select-object -first 1
&$msbuild ALL_BUILD.vcxproj /m /p:Configuration=$CONFIGURATION
&$msbuild INSTALL.vcxproj /m /p:Configuration=$CONFIGURATION
