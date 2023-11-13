#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Modifications Copyright Amazon.com, Inc. or its affiliates.
# SPDX-License-Identifier: Apache-2.0
#

project(trino-odbc-integration-tests)

set(TARGET ${PROJECT_NAME})

if (WIN32)
    set(Boost_USE_STATIC_LIBS ON)
endif()

find_package(Boost 1.53 REQUIRED COMPONENTS unit_test_framework chrono thread system regex)

find_package(ODBC REQUIRED)
find_package(AWSSDK REQUIRED COMPONENTS core sts trino-query)
if (UNIX)
    find_package(ZLIB REQUIRED)
endif()

find_package(Threads REQUIRED)

include_directories(SYSTEM ${ODBC_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${AWSSDK_INCLUDE_DIRS})
include_directories(include ../../odbc/include)
if (WIN32)
     include_directories(../../odbc/os/ignite/common/os/win/include ../../odbc/os/trino/win/include)
else ()
    include_directories(../../odbc/os/ignite/common/os/linux/include ../../odbc/os/trino/linux/include)
endif()

set(SOURCES 
	src/api_robustness_test.cpp
	src/application_data_buffer_test.cpp
	src/attributes_test.cpp
	src/connection_info_test.cpp
        src/connection_test.cpp
	src/descriptors_test.cpp
	src/meta_queries_test.cpp
        src/odbc_test_suite.cpp
	src/odbc_execution_test.cpp
	src/queries_test.cpp
	src/sql_get_info_test.cpp
        src/test_utils.cpp
        )

if (WIN32)
    list(APPEND SOURCES
        ../../odbc/os/ignite/common/os/win/src/common/concurrent_os.cpp
        ../../odbc/os/ignite/common/os/win/src/common/platform_utils.cpp
        ../../odbc/os/ignite/common/os/win/src/common/dynamic_load_os.cpp
        ../../odbc/os/trino/win/src/system_dsn.cpp
        ../../odbc/os/trino/win/src/system/ui/custom_window.cpp
        ../../odbc/os/trino/win/src/system/ui/dsn_configuration_window.cpp
        ../../odbc/os/trino/win/src/system/ui/window.cpp
    )
else()
    list(APPEND SOURCES
        ../../odbc/os/ignite/common/os/linux/src/common/concurrent_os.cpp
        ../../odbc/os/ignite/common/os/linux/src/common/platform_utils.cpp
	../../odbc/os/ignite/common/os/linux/src/common/dynamic_load_os.cpp
    )
endif()

add_executable(${TARGET} ${SOURCES})
add_dependencies(trino-odbc-integration-tests trino-odbc)

target_link_libraries(${TARGET} ${ODBC_LIBRARIES})
target_link_libraries(${TARGET} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${Boost_LIBRARIES})

if (WIN32)
    # ensure our self-build aws sdk library could be found
    set_target_properties(aws-cpp-sdk-core aws-cpp-sdk-sts aws-cpp-sdk-trino-query PROPERTIES
      IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/../build/aws-sdk/install/bin
    )
    set_target_properties(AWS::aws-crt-cpp AWS::aws-c-cal AWS::aws-c-auth AWS::aws-c-common AWS::aws-c-io AWS::aws-checksums AWS::aws-c-event-stream AWS::aws-c-s3  AWS::aws-c-http AWS::aws-c-mqtt AWS::aws-c-compression AWS::aws-c-sdkutils PROPERTIES
      IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/../build/aws-sdk/install/lib
    )

    target_link_libraries(${TARGET} ${AWSSDK_LINK_LIBRARIES})
    target_link_libraries(${TARGET} ${CMAKE_BINARY_DIR}/odbc/${CMAKE_BUILD_TYPE}/trino-odbc-static.lib)
elseif(APPLE)
    target_link_libraries(${TARGET} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtrino-odbc.dylib)
else()
    target_link_libraries(${TARGET} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtrino-odbc.so)
endif()

if (${CODE_COVERAGE}) 
    target_code_coverage(${TARGET} AUTO)
endif()

add_definitions(-DUNICODE=1)
add_definitions(-DPROJECT_VERSION=${CMAKE_PROJECT_VERSION})
add_definitions(-DPROJECT_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
add_definitions(-DPROJECT_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
add_definitions(-DPROJECT_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})

if (WIN32)
    add_definitions(-DTARGET_MODULE_FULL_NAME="$<TARGET_FILE_NAME:${TARGET}>")
    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_link_libraries(${TARGET} legacy_stdio_definitions odbccp32 shlwapi)
    endif()
elseif(APPLE)
    add_definitions(-DBOOST_TEST_DYN_LINK)
    target_link_libraries(${TARGET} iodbcinst ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
else()
    add_definitions(-DBOOST_TEST_DYN_LINK)
    target_link_libraries(${TARGET} odbcinst ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

set(TEST_TARGET IgniteOdbcTest)

add_test(NAME ${TEST_TARGET} COMMAND ${TARGET} --catch_system_errors=no --log_level=all)

set_tests_properties(${TEST_TARGET} PROPERTIES ENVIRONMENT IGNITE_NATIVE_TEST_ODBC_CONFIG_PATH=${PROJECT_SOURCE_DIR}/config)
