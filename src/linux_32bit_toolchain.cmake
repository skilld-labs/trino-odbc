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

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR "i386")

# compilers to use for C and C++
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-m32 -Wno-error=deprecated-declarations")
set(CMAKE_SHARED_LINKER_FLAGS -m32)

# here is the target environment location 
set(CMAKE_FIND_ROOT_PATH  /usr/i386-linux-gnu)
set(CMAKE_LIBRARY_PATH "/usr/lib/i386-linux-gnu")
include_directories(BEFORE /usr/include/i386-linux-gnu)

