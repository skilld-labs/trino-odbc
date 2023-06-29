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

cmake_minimum_required(VERSION 3.20)

project(performance_tests)

set(PERFORMANCE_HELPER "${CMAKE_CURRENT_SOURCE_DIR}/PerformanceHelper")
set(RESULTS_PTESTS "${CMAKE_CURRENT_SOURCE_DIR}/PTODBCResults")
set(EXECUTION_PTESTS "${CMAKE_CURRENT_SOURCE_DIR}/PTODBCExecution")
set(CMAKE_CXX_STANDARD 17)

# Projects to build
add_subdirectory(${PERFORMANCE_HELPER})
add_subdirectory(${RESULTS_PTESTS})
add_subdirectory(${EXECUTION_PTESTS})

if (NOT "$ENV{BOOST_ROOT}" STREQUAL "")
    set(BOOST_ROOT "$ENV{BOOST_ROOT}")
endif()

# gtest
include(gtest/googletest.cmake)
if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -fPIC")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -fPIC -Wno-error=maybe-uninitialized")
endif()
fetch_googletest(
  ${PROJECT_SOURCE_DIR}/gtest
  ${PROJECT_BINARY_DIR}/googletest
)

