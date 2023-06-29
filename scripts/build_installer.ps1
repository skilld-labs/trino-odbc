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

cmake -S $SRC_DIR `
    -B $BUILD_DIR `
    -A $WIN_ARCH `
    -D CMAKE_BUILD_TYPE=$CONFIGURATION `
    -D CMAKE_INSTALL_PREFIX=$INSTALL_DIR `
    -D BUILD_WITH_TESTS=OFF

# # Build Project
msbuild $BUILD_DIR\PACKAGE.vcxproj -m -p:Configuration=Release 
