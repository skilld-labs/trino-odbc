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

#ifndef _TRINO_ODBC_UTILITY
#define _TRINO_ODBC_UTILITY

#ifdef min
#undef min
#endif  // min

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

#include <ignite/common/include/common/decimal.h>
#include <trino/odbc/utils.h>
#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <string>

#include <sqltypes.h>
#include <sql.h>

namespace trino {
namespace odbc {
namespace utility {
/** Using common version of the util. */
using trino::odbc::common::IntoLower;
using namespace ignite::odbc::common;

template < typename T >
T* GetPointerWithOffset(T* ptr, size_t offset) {
  uint8_t* ptrBytes = (uint8_t*)ptr;

  return (T*)(ptrBytes + offset);
}

/**
 * Copy utf-8 string to SQLCHAR buffer of the specific length. It will ensure
 * null terminated result, possibly truncated.
 * @param inBuffer UTF-8, null-terminated string to copy data from.
 * @param outBuffer SQLCHAR buffer to copy data to.
 * @param outBufferLenBytes Length of the output buffer, in bytes.
 * @return isTruncated Reference to indicator of whether the input string was
 * truncated in the output buffer.
 * return value(bytes):
 *   - 0, if the inBuffer is nullptr or outBufferLenBytes is 0 but outBuffer is
 * not nullptr
 *   - the required output buffer length, if outBuffer is nullptr
 *   - copied bytes number, if outBuffer is not nullptr and outBufferLenBytes
 *   is not 0
 */
IGNITE_IMPORT_EXPORT size_t
CopyUtf8StringToSqlCharString(const char* inBuffer, SQLCHAR* outBuffer,
                              size_t outBufferLenBytes, bool& isTruncated);

/**
 * Copy utf-8 string to SQLWCHAR buffer of the specific length. It will ensure
 * null terminated result, possibly truncated.
 * @param inBuffer UTF-8, null-terminated string to copy data from.
 * @param outBuffer SQLWCHAR buffer to copy data to.
 * @param outBufferLenBytes Length of the output buffer, in bytes.
 * @return isTruncated Reference to indicator of whether the input string was
 * truncated in the output buffer.
 * return value(bytes):
 *   - 0, if the inBuffer is nullptr or outBufferLenBytes is 0 but outBuffer is
 * not nullptr
 *   - the required output buffer length, if outBuffer is nullptr
 *   - copied bytes number, if outBuffer is not nullptr and outBufferLenBytes
 *   is not 0
 */
IGNITE_IMPORT_EXPORT size_t
CopyUtf8StringToSqlWcharString(const char* inBuffer, SQLWCHAR* outBuffer,
                               size_t outBufferLenBytes, bool& isTruncated);

/**
 * Copy string to buffer of the specific length.
 * @param str String to copy data from.
 * @param buf Buffer to copy data to.
 * @param buflen Length of the buffer.
 * @return Length of the resulting string in buffer.
 * return value(bytes):
 *   - 0, if the inBuffer is nullptr or outBufferLenBytes is 0 but outBuffer is
 * not nullptr
 *   - the required output buffer length, if outBuffer is nullptr
 *   - copied bytes number, if outBuffer is not nullptr and outBufferLenBytes
 *   is not 0
 */
IGNITE_IMPORT_EXPORT size_t CopyStringToBuffer(const std::string& str,
                                               SQLWCHAR* buf, size_t buflen,
                                               bool& isTruncated,
                                               bool isLenInBytes = false);

/**
 * Convert SQLWCHAR string buffer to std::string.
 *
 * @param sqlStr SQL string buffer.
 * @param sqlStrLen SQL string length.
 * @param isLenInBytes Indicator of whether the length indicates bytes or
 * characters.
 * @return Standard string containing the same data.
 */
IGNITE_IMPORT_EXPORT std::string SqlWcharToString(const SQLWCHAR* sqlStr,
                                                  int32_t sqlStrLen = SQL_NTS,
                                                  bool isLenInBytes = false);

/**
 * Convert SQLWCHAR string buffer to boost::optional< std::string >.
 *
 * @param sqlStr SQL string buffer.
 * @param sqlStrLen SQL string length.
 * @param isLenInBytes Indicator of whether given length is in bytes or
 * characters.
 * @return Standard optional string containing the same data.
 * If sqlStrLen indicates null string, boost::none is returned.
 */
IGNITE_IMPORT_EXPORT boost::optional< std::string > SqlWcharToOptString(
    const SQLWCHAR* sqlStr, int32_t sqlStrLen = SQL_NTS,
    bool isLenInBytes = false);

/**
 * Convert SQL string buffer to std::string.
 *
 * @param sqlStr SQL string buffer.
 * @param sqlStrLen SQL string length.
 * @return Standard string containing the same data.
 */
IGNITE_IMPORT_EXPORT std::string SqlCharToString(const SQLCHAR* sqlStr,
                                                 int32_t sqlStrLen);

/**
 * Convert a wide string to UTF-8 encoded string.
 *
 * @param value wide string value to convert.
 * @return String value converted to UTF-8 encoding.
 */
IGNITE_IMPORT_EXPORT std::string ToUtf8(const std::wstring& value);

/**
 * Convert a wide string to UTF-8 encoded string.
 *
 * @param value wide string value to convert.
 * @return String value converted to UTF-8 encoding.
 */
IGNITE_IMPORT_EXPORT std::string ToUtf8(const wchar_t* value);

/**
 * Convert a UTF-8 encoded string to wide string.
 *
 * @param value UTF-8 encoded string.
 * @return String value converted to UTF-8 encoding.
 */
IGNITE_IMPORT_EXPORT std::wstring FromUtf8(const std::string& value);

/**
 * Convert a UTF-8 encoded string to wide string.
 *
 * @param value Pointer to UTF-8 encoded string.
 * @return String value converted to UTF-8 encoding.
 */
IGNITE_IMPORT_EXPORT std::wstring FromUtf8(const char* value);

/**
 * Convert a UTF-8 string to vector of unsigned short.
 *
 * @param value wide string value to convert.
 * @return String value converted to vector of unsigned short encoding.
 */
IGNITE_IMPORT_EXPORT std::vector< SQLWCHAR > ToWCHARVector(
    const std::string& value);

/**
 * Convert a UTF-8 string to vector of unsigned short.
 *
 * @param value pointer to null-terminated wide string value to convert.
 * @return String value converted to vector of unsigned short encoding.
 */
IGNITE_IMPORT_EXPORT std::vector< SQLWCHAR > ToWCHARVector(const char* value);

/**
 * Convert binary data to hex dump form
 * @param data  pointer to data
 * @param count data length
 * @return standard string containing the formated hex dump
 */
IGNITE_IMPORT_EXPORT std::string HexDump(const void* data, size_t count);

/**
 * Trims leading space from a string.
 * @param s string to be trimmed
 *
 * @return the string with leading spaces trimmed.
 */
IGNITE_IMPORT_EXPORT std::string Ltrim(const std::string& s);

/**
 * Trims trailing space from a string.
 * @param s string to be trimmed
 *
 * @return the string with trailing spaces trimmed.
 */
IGNITE_IMPORT_EXPORT std::string Rtrim(const std::string& s);

/**
 * Trims leading and trailing space from a string.
 * @param s string to be trimmed
 *
 * @return the string with leading and trailing spaces trimmed.
 */
IGNITE_IMPORT_EXPORT std::string Trim(const std::string& s);

/**
 * Converts a string with search patterns to regular expression string.
 * @param pattern Pattern to be converted
 *
 * @return the converted regular expression string.
 */
IGNITE_IMPORT_EXPORT std::string ConvertPatternToRegex(
    const std::string& pattern);

/**
 * Converts a numeric string to int.
 * @param s numeric string to be converted
 *
 * @return the converted int value
 */
IGNITE_IMPORT_EXPORT int StringToInt(const std::string& s, size_t* idx = 0,
                                     int base = 10);

/**
 * Converts a numeric string to long
 * @param s numeric string to be converted
 *
 * @return the converted long value
 */
IGNITE_IMPORT_EXPORT long StringToLong(const std::string& s, size_t* idx = 0,
                                       int base = 10);

/**
 * Checks if an environment variable is set to true. Case insensitive.
 * @param envVar Environment variable name
 *
 * @return true if environment variable is set to "TRUE", false otherwise
 */
IGNITE_IMPORT_EXPORT bool CheckEnvVarSetToTrue(const std::string& envVar);

/**
 * Get driver version based on DRIVER_VERSION_MAJOR, DRIVER_VERSION_MINOR and
 * DRIVER_VERSION_PATCH
 *
 * @return the generated full driver version in string
 */
IGNITE_IMPORT_EXPORT std::string GetFormatedDriverVersion();
}  // namespace utility
}  // namespace odbc
}  // namespace trino

namespace std {
/**
 * Convert wstring to utf-8 encoding.
 */
inline std::ostream& operator<<(std::ostream& out, const std::wstring& value) {
  out << trino::odbc::utility::ToUtf8(value);
  return out;
}
}  // namespace std

#endif  //_TRINO_ODBC_UTILITY
