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

project(timestream-odbc-unit-tests)

set(TARGET ${PROJECT_NAME})

if (WIN32)
    set(Boost_USE_STATIC_LIBS ON)
endif()

find_package(Boost 1.53 REQUIRED COMPONENTS unit_test_framework chrono thread system regex)

find_package(AWSSDK REQUIRED COMPONENTS core sts timestream-query)
if (UNIX)
    find_package(ZLIB REQUIRED)
endif()

find_package(Threads REQUIRED)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(include ../../odbc/include ../integration-test/include ${AWSSDK_INCLUDE_DIRS})
if (WIN32)
    include_directories(../../odbc/os/ignite/common/os/win/include ../../odbc/os/timestream/win/include)
else ()
    include_directories(../../odbc/os/ignite/common/os/linux/include ../../odbc/os/timestream/linux/include)
endif()

set(SOURCES 
	 src/column_meta_test.cpp
	 src/configuration_test.cpp
	 src/log_test.cpp
	 src/unit_connection_string_parser_test.cpp
	 src/unit_connection_test.cpp
	 src/unit_data_query_test.cpp
	 src/utility_test.cpp
	 src/odbc_unit_test_suite.cpp
	 src/mock/mock_environment.cpp
         src/mock/mock_connection.cpp
         src/mock/mock_httpclient.cpp
         src/mock/mock_statement.cpp
         src/mock/mock_stsclient.cpp
         src/mock/mock_timestream_query_client.cpp
         src/mock/mock_timestream_service.cpp
        )

add_executable(${TARGET} ${SOURCES})
add_dependencies(timestream-odbc-unit-tests timestream-odbc)

target_link_libraries(${TARGET} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${Boost_LIBRARIES})

if (WIN32)
    # ensure our self-build aws sdk library could be found
    set_target_properties(aws-cpp-sdk-core aws-cpp-sdk-sts aws-cpp-sdk-timestream-query PROPERTIES
      IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/../build/aws-sdk/install/bin
    )
    set_target_properties(AWS::aws-crt-cpp AWS::aws-c-cal AWS::aws-c-auth AWS::aws-c-common AWS::aws-c-io AWS::aws-checksums AWS::aws-c-event-stream AWS::aws-c-s3  AWS::aws-c-http AWS::aws-c-mqtt AWS::aws-c-compression AWS::aws-c-sdkutils PROPERTIES
      IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/../build/aws-sdk/install/lib
    )

    target_link_libraries(${TARGET} ${AWSSDK_LINK_LIBRARIES})
    target_link_libraries(${TARGET} ${CMAKE_BINARY_DIR}/odbc/${CMAKE_BUILD_TYPE}/timestream-odbc-static.lib)
elseif(APPLE)
    target_link_libraries(${TARGET} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtimestream-odbc.dylib)
else()
    target_link_libraries(${TARGET} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtimestream-odbc.so)
endif()

if (${CODE_COVERAGE}) 
    target_code_coverage(${TARGET} AUTO)
endif()

add_definitions(-DUNICODE=1)
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

set(TEST_TARGET IgniteOdbcUnitTest)

add_test(NAME ${TEST_TARGET} COMMAND ${TARGET} --catch_system_errors=no --log_level=all)

set_tests_properties(${TEST_TARGET} PROPERTIES ENVIRONMENT IGNITE_NATIVE_TEST_ODBC_CONFIG_PATH=${PROJECT_SOURCE_DIR}/config)
