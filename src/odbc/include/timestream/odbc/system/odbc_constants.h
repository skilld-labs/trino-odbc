/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modifications Copyright Amazon.com, Inc. or its affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _TIMESTREAM_ODBC_SYSTEM_ODBC_CONSTANTS
#define _TIMESTREAM_ODBC_SYSTEM_ODBC_CONSTANTS

#ifdef _WIN32

#define _WINSOCKAPI_
#include <windows.h>

#ifdef min
#undef min
#endif  // min

#endif  //_WIN32

#define ODBCVER 0x0351

// PROJECT_VERSION taken from CMakeLists.txt
#define DRIVER_VERSION PROJECT_VERSION
#define DRIVER_VERSION_MAJOR PROJECT_VERSION_MAJOR
#define DRIVER_VERSION_MINOR PROJECT_VERSION_MINOR
#define DRIVER_VERSION_PATCH PROJECT_VERSION_PATCH

#ifndef NAMEDATALEN
#define NAMEDATALEN 64
#endif /* NAMEDATALEN */

#ifndef MAX_CURSOR_LEN
#define MAX_CURSOR_LEN 32
#endif /* MAX_CURSOR_LEN */

// Internal SQL connection attribute to set log level
#define SQL_ATTR_TSLOG_DEBUG 65536

// Internal flag to use database as catalog or schema
// true if databases are reported as catalog, false if databases are reported as
// schema
#define DATABASE_AS_SCHEMA \
  timestream::odbc::utility::CheckEnvVarSetToTrue("DATABASE_AS_SCHEMA")

#define ANSI_STRING_ONLY \
  timestream::odbc::utility::CheckEnvVarSetToTrue("ANSI_STRING_ONLY")

#include <odbcinst.h>
#include <sqlext.h>

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif  // UNREFERENCED_PARAMETER

// For ODBC2 all functions
#define SQL_API_ALL_FUNCTIONS_SIZE 100

#endif  //_TIMESTREAM_ODBC_SYSTEM_ODBC_CONSTANTS
