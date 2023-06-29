# Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# A copy of the License is located at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# or in the "license" file accompanying this file. This file is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.

project(performance_helper)

find_package(ODBC REQUIRED)

# Source, headers, and include dirs
set(SOURCE_FILES performance_helper.cpp)
set(HEADER_FILES performance_helper.h)	

# Generate dll (SHARED)
add_library(performance_helper  ${SOURCE_FILES} ${HEADER_FILES})
include_directories(${ODBC_INCLUDE_DIRS})

# Library dependencies
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -fPIC")
target_link_libraries(performance_helper gtest_main ${ODBC_LIBRARIES})
target_compile_definitions(performance_helper PUBLIC _UNICODE UNICODE)	

