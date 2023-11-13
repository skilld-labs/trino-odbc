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

# Update version number in ODBC_DRIVER_VERSION.txt file.
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/ODBC_DRIVER_VERSION.txt" ODBC_DRIVER_VERSION)
string(STRIP ${ODBC_DRIVER_VERSION} ODBC_DRIVER_VERSION)
project(Trino-ODBC VERSION ${ODBC_DRIVER_VERSION})

# Mac
if (UNIX AND APPLE)
    set(AWS_CMAKE_DIR ${CMAKE_SOURCE_DIR}/aws-sdk-cpp/install/lib)
endif()

# Linux 
if (UNIX AND NOT APPLE)
    if (${INSTALLER_TYPE} STREQUAL "RPM" AND ${BITNESS} EQUAL 64)
        set(AWS_CMAKE_DIR ${CMAKE_SOURCE_DIR}/aws-sdk-cpp/install/lib64)
    else()
        set(AWS_CMAKE_DIR ${CMAKE_SOURCE_DIR}/aws-sdk-cpp/install/lib)
    endif()
endif()

# Windows
if (WIN32)
    set(AWS_CMAKE_DIR ${CMAKE_SOURCE_DIR}/../build/aws-sdk/install/lib)

    set(CMAKE_BUILD_DIR ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set(AWS_DLL_DIR ${CMAKE_SOURCE_DIR}/../build/aws-sdk/install/bin)
    set(AWS_LIB_DIR ${CMAKE_SOURCE_DIR}/../build/aws-sdk/install/lib)

    # copy AWS DLL to ${CMAKE_BUILD_DIR} as they are needed for Windows building and test running
    configure_file(${AWS_DLL_DIR}/aws-cpp-sdk-core.dll ${CMAKE_BUILD_DIR}/aws-cpp-sdk-core.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-cpp-sdk-trino-query.dll ${CMAKE_BUILD_DIR}/aws-cpp-sdk-trino-query.dll COPYONLY)
    # Trino-write dll is needed for data writer tool
    configure_file(${AWS_DLL_DIR}/aws-cpp-sdk-trino-write.dll ${CMAKE_BUILD_DIR}/aws-cpp-sdk-trino-write.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-cpp-sdk-sts.dll ${CMAKE_BUILD_DIR}/aws-cpp-sdk-sts.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-c-auth.dll ${CMAKE_BUILD_DIR}/aws-c-auth.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-c-cal.dll ${CMAKE_BUILD_DIR}/aws-c-cal.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-c-common.dll ${CMAKE_BUILD_DIR}/aws-c-common.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-c-compression.dll ${CMAKE_BUILD_DIR}/aws-c-compression.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-c-event-stream.dll ${CMAKE_BUILD_DIR}/aws-c-event-stream.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-checksums.dll ${CMAKE_BUILD_DIR}/aws-checksums.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-c-http.dll ${CMAKE_BUILD_DIR}/aws-c-http.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-c-io.dll ${CMAKE_BUILD_DIR}/aws-c-io.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-c-mqtt.dll ${CMAKE_BUILD_DIR}/aws-c-mqtt.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-crt-cpp.dll ${CMAKE_BUILD_DIR}/aws-crt-cpp.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-c-s3.dll ${CMAKE_BUILD_DIR}/aws-c-s3.dll COPYONLY)
    configure_file(${AWS_DLL_DIR}/aws-c-sdkutils.dll ${CMAKE_BUILD_DIR}/aws-c-sdkutils.dll COPYONLY)
endif()

# ensure AWS SDK libraries built by us could be found by find_package
set(s2n_DIR "${AWS_CMAKE_DIR}/s2n/cmake")
set(aws-cpp-sdk-trino-query_DIR "${AWS_CMAKE_DIR}/cmake/aws-cpp-sdk-trino-query")
# Trino-write directory value is needed for data writer tool
set(aws-cpp-sdk-trino-write_DIR "${AWS_CMAKE_DIR}/cmake/aws-cpp-sdk-trino-write")
set(aws-cpp-sdk-sts_DIR "${AWS_CMAKE_DIR}/cmake/aws-cpp-sdk-sts")
set(aws-cpp-sdk-core_DIR "${AWS_CMAKE_DIR}/cmake/aws-cpp-sdk-core")
set(aws-crt-cpp_DIR "${AWS_CMAKE_DIR}/aws-crt-cpp/cmake")
set(aws-c-http_DIR "${AWS_CMAKE_DIR}/aws-c-http/cmake")
set(aws-c-io_DIR "${AWS_CMAKE_DIR}/aws-c-io/cmake")
set(aws-c-common_DIR "${AWS_CMAKE_DIR}/aws-c-common/cmake")
set(aws-c-cal_DIR "${AWS_CMAKE_DIR}/aws-c-cal/cmake")
set(aws-c-compression_DIR "${AWS_CMAKE_DIR}/aws-c-compression/cmake")
set(aws-c-auth_DIR "${AWS_CMAKE_DIR}/aws-c-auth/cmake")
set(aws-c-mqtt_DIR "${AWS_CMAKE_DIR}/aws-c-mqtt/cmake")
set(aws-checksums_DIR "${AWS_CMAKE_DIR}/aws-checksums/cmake")
set(aws-c-event-stream_DIR "${AWS_CMAKE_DIR}/aws-c-event-stream/cmake")
set(aws-c-s3_DIR "${AWS_CMAKE_DIR}/aws-c-s3/cmake")
set(aws-c-sdkutils_DIR "${AWS_CMAKE_DIR}/aws-c-sdkutils/cmake")
set(AWSSDK_DIR "${AWS_CMAKE_DIR}/cmake/AWSSDK")

include("${CMAKE_CURRENT_SOURCE_DIR}/modules/code-coverage.cmake")
if (NOT "$ENV{BOOST_ROOT}" STREQUAL "")
    set(BOOST_ROOT "$ENV{BOOST_ROOT}")
endif()
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIGNITE_IMPL -DIGNITE_FRIEND -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS")

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)

if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

add_definitions(-DUNICODE=1)
if (WIN32)

    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build/odbc/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build/odbc/lib")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build/odbc/bin")
endif()

if (MSVC)
    add_compile_options(/source-charset:utf-8 /execution-charset:utf-8)

    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()

option (WITH_ODBC OFF)
option (WITH_ODBC_MSI OFF)
option (WITH_THIN_CLIENT OFF)
option (WITH_TESTS OFF)
option (WARNINGS_AS_ERRORS OFF)

if (${WARNINGS_AS_ERRORS})
    if (MSVC)
        add_compile_options(/WX)
    else()
        add_compile_options(-Wall -Wextra -Werror -Wno-variadic-macros)
    endif()
endif()

if (${WITH_ODBC})
    add_subdirectory(odbc)
endif()

if (${WITH_TESTS})
    enable_testing()

    if (EXISTS ${CMAKE_SOURCE_DIR}/tests)
        add_subdirectory(tests)
    endif()
endif()

