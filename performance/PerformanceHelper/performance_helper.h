/* Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef PERFORMANCE_HELPER_H
#define PERFORMANCE_HELPER_H

#ifdef __linux__
#include <climits>
#endif

#ifdef WIN32
#include <windows.h>
#include "psapi.h"
#endif

#ifdef __APPLE__
#include <mach/mach.h>
#endif

#ifdef __linux__
#include "stdlib.h"
#include "stdio.h"
#include <cstring>
#endif

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#include <codecvt>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

#define HELPER_SIZEOF(x) (x == nullptr ? 0 : (sizeof(x) / sizeof(x[0])))
#define TO_SQLTCHAR(str) \
  const_cast< SQLTCHAR* >(reinterpret_cast< const SQLTCHAR* >(str))

#ifdef __linux__
typedef std::u16string testString;
#define CREATE_STRING(str) u"" str
#else
typedef std::wstring testString;
#define CREATE_STRING(str) L"" str
#endif  //__linux__

void logDiagnostics(SQLSMALLINT handleType, SQLHANDLE handle, SQLRETURN ret,
                    SQLTCHAR* msgReturn = nullptr, const SQLSMALLINT size = 0);

int parseLine(char* line);
int currentMemUsage();

std::string sqltcharToStr(const SQLTCHAR* sqltchar);
#endif  // PERFORMANCE_HELPER_H
