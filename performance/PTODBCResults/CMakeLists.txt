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

project(performance_results)

find_package(Boost 1.71 REQUIRED COMPONENTS thread)
find_package(ODBC REQUIRED)
# Source, headers, and include dirs
set(SOURCE_FILES performance_odbc_results.cpp)
include_directories(${PERFORMANCE_HELPER} SYSTEM ${ODBC_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

# Generate executable
add_executable(performance_results ${SOURCE_FILES})

# Library dependencies
target_link_libraries(performance_results performance_helper gtest_main ${ODBC_LIBRARIES} ${Boost_LIBRARIES})
target_compile_definitions(performance_results PUBLIC _UNICODE UNICODE)
