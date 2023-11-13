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

project(trino-odbc)

set(TARGET ${PROJECT_NAME})
set(CMAKE_VERBOSE_MAKEFILE ON)

include(InstallRequiredSystemLibraries)
include(CPackComponent)

message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(ODBC REQUIRED)
find_package(AWSSDK REQUIRED COMPONENTS core sts trino-query)
find_package(Boost REQUIRED)
if (UNIX)
    find_package(ZLIB REQUIRED)
endif()

if (${CODE_COVERAGE}) 
    add_code_coverage()    
endif()

include_directories(SYSTEM ${ODBC_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${AWSSDK_INCLUDE_DIRS})

include_directories(include)

set(SOURCES src/app/application_data_buffer.cpp
        src/authentication/aad.cpp
        src/authentication/auth_type.cpp
        src/authentication/okta.cpp
        src/authentication/saml.cpp
        src/common_types.cpp
        src/config/configuration.cpp
        src/config/connection_info.cpp
        src/config/connection_string_parser.cpp
        src/connection.cpp
	src/descriptor.cpp
        src/diagnostic/diagnosable_adapter.cpp
        src/diagnostic/diagnostic_record.cpp
        src/diagnostic/diagnostic_record_storage.cpp
        src/dsn_config.cpp
        src/entry_points.cpp
        src/environment.cpp
        src/ignite/common/src/common/big_integer.cpp
        src/ignite/common/src/common/bits.cpp
        src/ignite/common/src/common/concurrent.cpp
        src/ignite/common/src/common/decimal.cpp
        src/ignite/common/src/date.cpp
        src/ignite_error.cpp
        src/interval_day_second.cpp
        src/interval_year_month.cpp
        src/log.cpp
        src/log_level.cpp
        src/meta/column_meta.cpp
        src/meta/table_meta.cpp
        src/odbc.cpp
        src/query/column_metadata_query.cpp
        src/query/column_privileges_query.cpp
        src/query/data_query.cpp
        src/query/foreign_keys_query.cpp
        src/query/primary_keys_query.cpp
        src/query/procedure_columns_query.cpp
        src/query/procedures_query.cpp
        src/query/special_columns_query.cpp
        src/query/statistics_query.cpp
        src/query/table_metadata_query.cpp
        src/query/table_privileges_query.cpp
        src/query/type_info_query.cpp
        src/statement.cpp
        src/time.cpp
        src/timestamp.cpp
        src/trino_column.cpp
        src/trino_cursor.cpp
        src/type_traits.cpp
        src/utility.cpp
        src/utils.cpp)

if (WIN32)
    set(OS_INCLUDE os/ignite/common/os/win/include os/trino/win/include)

    list(APPEND SOURCES 
            os/trino/win/src/system_dsn.cpp
            os/trino/win/src/system/ui/custom_window.cpp
            os/trino/win/src/system/ui/dsn_configuration_window.cpp
            os/trino/win/src/system/ui/window.cpp
            module.def
            os/ignite/common/os/win/src/common/concurrent_os.cpp
            os/ignite/common/os/win/src/common/platform_utils.cpp
            os/ignite/common/os/win/src/common/dynamic_load_os.cpp
    )
else()
    set(OS_INCLUDE os/ignite/common/os/linux/include)

    list(APPEND SOURCES 
            os/ignite/common/os/linux/src/common/concurrent_os.cpp
            os/ignite/common/os/linux/src/common/platform_utils.cpp
            os/ignite/common/os/linux/src/common/dynamic_load_os.cpp
    )
endif ()

include_directories(${OS_INCLUDE})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(${TARGET} SHARED ${SOURCES} version.rc)
if (WIN32)
    # static library created for Windows only for tests linking
    add_library(${TARGET}-static STATIC ${SOURCES} version.rc)

    # ensure our self-build aws sdk library could be found
    set_target_properties(aws-cpp-sdk-core aws-cpp-sdk-sts aws-cpp-sdk-trino-query PROPERTIES
      IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/../build/aws-sdk/install/bin
    )
    set_target_properties(AWS::aws-crt-cpp AWS::aws-c-cal AWS::aws-c-auth AWS::aws-c-common AWS::aws-c-io AWS::aws-checksums AWS::aws-c-event-stream AWS::aws-c-s3  AWS::aws-c-http AWS::aws-c-mqtt AWS::aws-c-compression AWS::aws-c-sdkutils PROPERTIES
      IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/../build/aws-sdk/install/lib
    )
endif ()

set_target_properties(${TARGET} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})

target_link_libraries(${TARGET} ${ODBC_LIBRARIES})
target_link_libraries(${TARGET} ${AWSSDK_LINK_LIBRARIES})

add_definitions(-DUNICODE=1)
add_definitions(-DPROJECT_VERSION=${CMAKE_PROJECT_VERSION})
add_definitions(-DPROJECT_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
add_definitions(-DPROJECT_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
add_definitions(-DPROJECT_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})

if (WIN32)
    target_link_libraries(${TARGET} odbccp32 shlwapi)

    add_definitions(-DTARGET_MODULE_FULL_NAME="$<TARGET_FILE_NAME:${TARGET}>")

    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_link_libraries(${TARGET} legacy_stdio_definitions)
    endif()
                
    set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "trino.odbc")
else()
    target_link_libraries(${TARGET})
endif()

if (WIN32 AND ${WITH_ODBC_MSI})
    find_program(WIX_CANDLE candle)
    if(NOT WIX_CANDLE)
        message(FATAL_ERROR "WIX candle.exe not found! Have you installed WIX Toolset or forgotten to add it to Path?")
    endif()

    find_program(WIX_LIGHT light)
    if(NOT WIX_LIGHT)
        message(FATAL_ERROR "WIX light.exe not found! Have you installed WIX Toolset or forgotten to add it to Path?")
    endif()

    set(WIX_ODBC_LIBRARY_PATH ".\\trino.odbc.dll")
    set(WIX_ODBC_SDK_CORE_PATH ".\\aws-cpp-sdk-core.dll")
    set(WIX_ODBC_SDK_TS_QUERY_PATH ".\\aws-cpp-sdk-trino-query.dll")
    set(WIX_ODBC_SDK_STS_PATH ".\\aws-cpp-sdk-sts.dll")
    set(WIX_ODBC_AWS_C_AUTH ".\\aws-c-auth.dll")
    set(WIX_ODBC_AWS_C_CAL ".\\aws-c-cal.dll")
    set(WIX_ODBC_AWS_C_COMMON ".\\aws-c-common.dll")
    set(WIX_ODBC_AWS_C_COMPRESSION ".\\aws-c-compression.dll")
    set(WIX_ODBC_AWS_C_EVENT_STREAM ".\\aws-c-event-stream.dll")
    set(WIX_ODBC_AWS_CHECKSUMS ".\\aws-checksums.dll")
    set(WIX_ODBC_AWS_C_HTTP ".\\aws-c-http.dll")
    set(WIX_ODBC_AWS_C_IO ".\\aws-c-io.dll")
    set(WIX_ODBC_AWS_C_MQTT ".\\aws-c-mqtt.dll")
    set(WIX_ODBC_AWS_CRT_CPP ".\\aws-crt-cpp.dll")
    set(WIX_ODBC_AWS_C_S3 ".\\aws-c-s3.dll")
    set(WIX_ODBC_AWS_C_SDKUTILS ".\\aws-c-sdkutils.dll")

    set(WIX_PROJECT_NAME "Amazon Trino")
    set(WIX_MANUFACTURER "Amazon Web Services")
    set(WIX_COPYRIGHT_COMMENT "Apache, Apache Ignite, the Apache feather and the Apache Ignite logo are trademarks of The Apache Software Foundation.")

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(WIX_BIT_SUFFIX "64-bit")
        set(WIX_PROGRAM_FILES_FOLDER_ID "Program Files (x86)")
        set(WIX_COMPONENT_IS_WIN64 "yes")
        set(WIX_INSTALLER_BUNDLE_SRC "${CMAKE_CURRENT_LIST_DIR}/install/trino-odbc-bundle.wxs.in.x64")

        if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") 
            set(WIX_PRODUCT_ID "942F102A-BCF5-46BE-A568-8A0A544F6456")
            set(WIX_UPGRADE_CODE "F17BF88F-9571-47FE-A642-5B0ECA26DFBD")
            set(WIX_COMPONENT_ID_ODBC_LIBRARY "AA8B007D-C9B9-4F1D-9C9F-E9384401E214")
            set(WIX_COMPONENT_ID_SDK_CORE "9AAED0BC-281E-454A-8DDB-76787A46573D")
            set(WIX_COMPONENT_ID_SDK_TS_QUERY "268BB95C-696C-46BC-89F9-7129380A3825")
            set(WIX_COMPONENT_ID_SDK_STS "ECEEF57F-ECEC-4B80-A040-A5394C7B56FC")

            set(WIX_COMPONENT_ID_AWS_C_AUTH "ADCC5700-9051-449B-9C7E-9C765458BE3E")
            set(WIX_COMPONENT_ID_AWS_C_CAL "577B1441-D084-4100-9A39-317701DA8908")
            set(WIX_COMPONENT_ID_AWS_C_COMMON "8413E31F-8FA1-4E58-AF19-128EF8F71E8A")
            set(WIX_COMPONENT_ID_AWS_C_COMPRESSION "7E16BE95-F650-498D-918E-0A53186D6D44")
            set(WIX_COMPONENT_ID_AWS_C_EVENT_STREAM "2955CF86-507C-41E7-A681-A9FBC8CE0AE0")
            set(WIX_COMPONENT_ID_AWS_CHECKSUMS "2D9C91D3-1D14-40E0-91A7-E9F1F6BCCB0D")
            set(WIX_COMPONENT_ID_AWS_C_HTTP "0E0EB1A2-3150-4978-86DD-210CE49BBF73")
            set(WIX_COMPONENT_ID_AWS_C_IO "BD661102-356F-426E-B5C2-47A9A2C23E13")
            set(WIX_COMPONENT_ID_AWS_C_MQTT "21D27FB0-7C0A-4E80-881D-DEA2DBD085FC")
            set(WIX_COMPONENT_ID_AWS_CRT_CPP "475234A5-AF23-4E7C-80AA-066B30522C58")
            set(WIX_COMPONENT_ID_AWS_C_S3 "E35D33AB-3C50-469E-8A60-4E4B266D03A9")
	    set(WIX_COMPONENT_ID_AWS_C_SDKUTILS "A9E68859-0566-4D3B-836A-89AA61472D54")
            set(WIX_BUNDLE_UPGRADE_CODE "C77C052F-CBEF-4EC1-8030-C4A6F6F7094A")

            set(WIX_PACKAGE_PLATFORM "arm64")
        else()
            set(WIX_PRODUCT_ID "CA6C75A4-8BC9-49CE-A71A-4CEE813119E1")
            set(WIX_UPGRADE_CODE "6954CE47-89EB-4F4F-A925-225A7A94531E")
            set(WIX_COMPONENT_ID_ODBC_LIBRARY "6B9E12BF-7914-43A7-99C6-2821950EE249")
            set(WIX_COMPONENT_ID_SDK_CORE "82EE01C2-5D7E-4FE4-838B-34E10D6F1F71")
            set(WIX_COMPONENT_ID_SDK_TS_QUERY "42B2CA3E-5314-4D1D-9491-F945B6B33B5A")
            set(WIX_COMPONENT_ID_SDK_STS "B55B759D-88B3-47C0-9ABA-60F1504C0DA8")

            set(WIX_COMPONENT_ID_AWS_C_AUTH "D584F5F1-CCB1-47D8-9EB5-1854399B2715")
            set(WIX_COMPONENT_ID_AWS_C_CAL "13A2A675-4C39-4C02-874A-AC0BE18054B3")
            set(WIX_COMPONENT_ID_AWS_C_COMMON "B9CD76F0-F26C-4033-9CB8-FEC33C4E5CBC")
            set(WIX_COMPONENT_ID_AWS_C_COMPRESSION "EEB47CFB-3A73-48AE-BC2B-B548145CA919")
            set(WIX_COMPONENT_ID_AWS_C_EVENT_STREAM "522AA652-D426-43D9-A9E1-B4F7ACAC5981")
            set(WIX_COMPONENT_ID_AWS_CHECKSUMS "0E6D8BEA-C4C8-4CC5-A264-17D7B24DB55E")
            set(WIX_COMPONENT_ID_AWS_C_HTTP "5AF8104B-2E21-44DB-A4A3-D551C818B118")
            set(WIX_COMPONENT_ID_AWS_C_IO "20DBED24-121E-4533-B6AE-A49019F78705")
            set(WIX_COMPONENT_ID_AWS_C_MQTT "C420CFFC-2289-4F3C-9FA7-0A1D9271DF90")
            set(WIX_COMPONENT_ID_AWS_CRT_CPP "811269B1-D54D-486D-BED3-F0A2A74D4E12")
            set(WIX_COMPONENT_ID_AWS_C_S3 "05FB1CFF-6993-4924-A872-322B1250C608")
	    set(WIX_COMPONENT_ID_AWS_C_SDKUTILS "BD17E26C-ED17-4064-B7F2-8B1BDF965AC7")
            set(WIX_BUNDLE_UPGRADE_CODE "BA03135B-F78E-41E5-8D70-C7FD62A4757A")

            set(WIX_PACKAGE_PLATFORM "x64")
        endif()

    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(WIX_BIT_SUFFIX "32-bit")
        set(WIX_PROGRAM_FILES_FOLDER_ID "Program Files")
        set(WIX_COMPONENT_IS_WIN64 "no")
	set(WIX_INSTALLER_BUNDLE_SRC "${CMAKE_CURRENT_LIST_DIR}/install/trino-odbc-bundle.wxs.in.x86")
        

        set(WIX_PRODUCT_ID "9FAACB44-B1D9-4E9B-B22E-44FB20C083E2")
        set(WIX_UPGRADE_CODE "7BDC7F9E-BB0D-4288-B404-96A78AB0B69D")
        set(WIX_COMPONENT_ID_ODBC_LIBRARY "F0B4778F-43BD-447C-B882-E70D9B16AEB6")
        set(WIX_COMPONENT_ID_SDK_CORE "1CD3936E-F8CF-42B9-90F9-7F33BC3E1CC1")
        set(WIX_COMPONENT_ID_SDK_TS_QUERY "4E88F45B-7623-4A80-9985-0AC2F81F371E")
        set(WIX_COMPONENT_ID_SDK_STS "E3E41090-D480-40DE-A7E2-62C038D936BD")

        set(WIX_COMPONENT_ID_AWS_C_AUTH "5376FD5A-D940-4FAC-B666-F9F37BBCA80E")
        set(WIX_COMPONENT_ID_AWS_C_CAL "875C5826-F555-4929-BFA5-819096E96437")
        set(WIX_COMPONENT_ID_AWS_C_COMMON "0DDAC317-5752-4391-9D5C-0B855F964DF9")
        set(WIX_COMPONENT_ID_AWS_C_COMPRESSION "34DE86DB-D1D5-4FE1-B13B-3C0BAD06C784")
        set(WIX_COMPONENT_ID_AWS_C_EVENT_STREAM "325A20AC-E459-4ABC-BFC5-E1680951EEE4")
        set(WIX_COMPONENT_ID_AWS_CHECKSUMS "7A70A03F-7128-4AAC-9556-0FDB536FEFED")
        set(WIX_COMPONENT_ID_AWS_C_HTTP "53D36BAF-F663-4799-8B75-BD8C643BB451")
        set(WIX_COMPONENT_ID_AWS_C_IO "922C1B88-9A36-4CE5-A770-EB10F7EF1FD9")
        set(WIX_COMPONENT_ID_AWS_C_MQTT "8F43AF91-BB7B-489D-9707-292497BBB21E")
        set(WIX_COMPONENT_ID_AWS_CRT_CPP "DB6DCE34-B717-43C1-A53C-EC36227B1B01")
        set(WIX_COMPONENT_ID_AWS_C_S3 "E382CA06-AEE1-48D3-95D4-D712A832AC4D")
	set(WIX_COMPONENT_ID_AWS_C_SDKUTILS "327859D5-75D3-4E2B-AC08-5E33F4372E4B")
        set(WIX_BUNDLE_UPGRADE_CODE "D788EB72-42FC-49EB-A47A-D65087A83C1A")

        set(WIX_PACKAGE_PLATFORM "x86")
    endif()

    set(WIX_PRODUCT_NAME "${WIX_PROJECT_NAME} ODBC ${WIX_BIT_SUFFIX} Driver")
    set(WIX_VC_RUNTIME_NAME "VC_redist.${WIX_PACKAGE_PLATFORM}.exe")
    set(WIX_INSTALLER_PREFIX ${CMAKE_BUILD_DIR}/trino-odbc-${WIX_PACKAGE_PLATFORM})
    set(WIX_INSTALLER_BUNDLE_PREFIX ${CMAKE_BUILD_DIR}/trino-odbc-bundle-${WIX_PACKAGE_PLATFORM})
    set(WIX_INSTALLER_EXE_PREFIX ${CMAKE_BUILD_DIR}/trino-odbc-installer-${WIX_PACKAGE_PLATFORM}-${ODBC_DRIVER_VERSION})

    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/install/trino-odbc.wxs.in"
        "${WIX_INSTALLER_PREFIX}.wxs"
        @ONLY
    )

    configure_file(
	"${WIX_INSTALLER_BUNDLE_SRC}"
	"${WIX_INSTALLER_BUNDLE_PREFIX}.wxs"
	@ONLY
    )

    file(DOWNLOAD 
         "https://aka.ms/vs/17/release/${WIX_VC_RUNTIME_NAME}"
    	 "${CMAKE_BUILD_DIR}/${WIX_VC_RUNTIME_NAME}"
    )

    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/install/images/awslogo.png"
	"${CMAKE_BUILD_DIR}/awslogo.png"
        COPYONLY
    )

    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        COMMAND ${WIX_CANDLE} ${WIX_INSTALLER_PREFIX}.wxs -out ${WIX_INSTALLER_PREFIX}.wxobj
	COMMAND ${WIX_LIGHT} ${WIX_INSTALLER_PREFIX}.wxobj -out ${WIX_INSTALLER_PREFIX}.msi
	COMMAND ${WIX_CANDLE} ${WIX_INSTALLER_BUNDLE_PREFIX}.wxs -ext WixBalExtension -ext WixUtilExtension -out ${WIX_INSTALLER_BUNDLE_PREFIX}.wixobj
	COMMAND ${WIX_LIGHT} ${WIX_INSTALLER_BUNDLE_PREFIX}.wixobj -ext WixBalExtension -ext WixUtilExtension -out ${WIX_INSTALLER_EXE_PREFIX}.exe
    )
elseif(APPLE)
    set(CPACK_PACKAGE_FILE_NAME "AmazonTrinoODBC-${ODBC_DRIVER_VERSION}")
    set(CPACK_PACKAGE_NAME "Amazon Trino ODBC Driver ${ODBC_DRIVER_VERSION}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
    set(CPACK_GENERATOR "productbuild")

    # This script will be run once the Driver component has finished installing.
    set(CPACK_POSTFLIGHT_DRIVER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/install/mac/postinstall")

    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/install/mac/README.txt")
    set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/install/mac/Welcome.txt")
elseif(UNIX)
    if (${INSTALLER_TYPE} STREQUAL "DEB")
        if (${BITNESS} EQUAL 32)
            set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
        else()
            set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
        endif()
        set(CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_FILE_NAME "AmazonTrinoODBC_${ODBC_DRIVER_VERSION}_${CMAKE_BUILD_TYPE}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")

        # Deb installers fail if there's a space in the directory or package name
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "AmazonTrinoODBCDriver${BITNESS}-bit")
	set(CPACK_PACKAGE_NAME "AmazonTrinoODBC_${ODBC_DRIVER_VERSION}_${CMAKE_BUILD_TYPE}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

        # TODO: Need to update both of these.
        set(CPACK_PACKAGE_CONTACT "user@sample.com")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "User Name")
        set(CPACK_PACKAGE_DESCRIPTION "AmazonTrinoODBCDriver-${CMAKE_BUILD_TYPE}-${BITNESS}-bit")

        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    elseif(${INSTALLER_TYPE} STREQUAL "RPM")
        if (${BITNESS} EQUAL 32)
            set(CPACK_RPM_PACKAGE_ARCHITECTURE i386)
        else()
            set(CPACK_RPM_PACKAGE_ARCHITECTURE x86_64)
        endif()
        set(CPACK_GENERATOR RPM)

        set(CPACK_PACKAGE_INSTALL_DIRECTORY "Amazon Trino ODBC Driver ${BITNESS}-bit")
        set(CPACK_PACKAGE_NAME "AmazonTrinoODBC_${ODBC_DRIVER_VERSION}_${CMAKE_BUILD_TYPE}_${CPACK_RPM_PACKAGE_ARCHITECTURE}")
        set(CPACK_RPM_FILE_NAME "${CPACK_PACKAGE_NAME}.rpm")

        set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
        set(CPACK_RPM_COMPONENT_INSTALL ON)
    else()
        message(FATAL_ERROR "INSTALLER_TYPE '${INSTALLER_TYPE}' must be DEB or RPM for Linux installer." )
    endif()
endif()

if(UNIX)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CPACK_PACKAGE_NAME}")
    set(CPACK_PACKAGE_VENDOR "Amazon")
    set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")

    # CPack doesn't allow extensionless licenses, need to make a copy with an extension, .txt is appropriate
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_ROOT}/LICENSE.txt")
    # Prepend LICENSE file with NOTICE
    configure_file("${PROJECT_ROOT}/NOTICE" "${PROJECT_ROOT}/LICENSE.txt" COPYONLY)
    file(APPEND "${PROJECT_ROOT}/LICENSE.txt" "\n=====================================================\n")
    file(READ "${PROJECT_ROOT}/LICENSE" ODBC_LICENSE)
    file(APPEND "${PROJECT_ROOT}/LICENSE.txt" ${ODBC_LICENSE})

    # Set up components for installer
    cpack_add_component(Docs
	    DISPLAY_NAME "Documentation"
	    DESCRIPTION "Documentation about Amazon Trino ODBC Driver"
    )
    cpack_add_component(Driver
	    DISPLAY_NAME "Driver"
	    DESCRIPTION "Library files for running the Amazon Trino ODBC Driver"
        REQUIRED 
    )

    if(APPLE)
        install(TARGETS ${TARGET} DESTINATION "trino-odbc/lib" COMPONENT "Driver")
    
        # Install documentation files
        install(FILES "${PROJECT_ROOT}/LICENSE.txt" DESTINATION "trino-odbc/doc" COMPONENT "Docs")
        install(FILES "${PROJECT_ROOT}/docs/markdown/setup/connection-string.md" DESTINATION "trino-odbc/doc" COMPONENT "Docs")
        install(FILES "${PROJECT_ROOT}/docs/markdown/setup/macOS-dsn-configuration.md" DESTINATION "trino-odbc/doc" COMPONENT "Docs")
        install(FILES "${PROJECT_ROOT}/docs/markdown/setup/microsoft-excel.md" DESTINATION "trino-odbc/doc" COMPONENT "Docs")
    else()
        if (${BITNESS} EQUAL 32)
	    set(LIBROOT lib)
	    add_custom_target(run ALL COMMAND sed -e "s@\#ODBC_LIB_PATH@ODBC_LIB_PATH=\/usr\/lib\/trino-odbc32@" ${PROJECT_ROOT}/scripts/postinst_unix.sh > ${PROJECT_ROOT}/scripts/postinst_unix${BITNESS}.sh)
	else()
	    set(LIBROOT lib64)
	    add_custom_target(run ALL COMMAND sed -e "s@\#ODBC_LIB_PATH@ODBC_LIB_PATH=\/usr\/lib64\/trino-odbc64@" ${PROJECT_ROOT}/scripts/postinst_unix.sh > ${PROJECT_ROOT}/scripts/postinst_unix${BITNESS}.sh)
	endif()

        # Install library and shell script 

        install(TARGETS ${TARGET} DESTINATION ${LIBROOT}/trino-odbc${BITNESS} COMPONENT "Driver")
	install(FILES "${PROJECT_ROOT}/scripts/postinst_unix${BITNESS}.sh" DESTINATION ${LIBROOT}/trino-odbc${BITNESS} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_EXECUTE COMPONENT "Driver")

        # Install documentation files
        install(FILES "${PROJECT_ROOT}/LICENSE.txt" DESTINATION "${LIBROOT}/trino-odbc${BITNESS}" COMPONENT "Docs")
        install(FILES "${PROJECT_ROOT}/docs/markdown/setup/linux-dsn-configuration.md" DESTINATION "${LIBROOT}/trino-odbc${BITNESS}" COMPONENT "Docs")
    endif()
endif()

target_include_directories(${TARGET} INTERFACE include)

install(TARGETS ${TARGET} 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

if (WIN32)
    if (MSVC_VERSION GREATER_EQUAL 1900)
        target_link_libraries(${TARGET} legacy_stdio_definitions odbccp32 shlwapi)
    endif()
elseif(APPLE)
    target_link_libraries(${TARGET} iodbcinst)
else()
    target_link_libraries(${TARGET} odbcinst)
endif()

set(VERSIONINFO ${CMAKE_PROJECT_VERSION_MAJOR},${CMAKE_PROJECT_VERSION_MINOR},${CMAKE_PROJECT_VERSION_PATCH})
set(VERSIONINFO_STR ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH})
configure_file("version.rc.in" "version.rc" @ONLY)

include(CPack)
