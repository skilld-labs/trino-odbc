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

#include "trino/odbc/utility.h"

#include <codecvt>
#include <regex>
#include <iomanip>

#include "trino/odbc/system/odbc_constants.h"
#include "trino/odbc/log.h"

namespace trino {
namespace odbc {
namespace utility {
using namespace ignite::odbc::common;

size_t CopyUtf8StringToSqlCharString(const char* inBuffer, SQLCHAR* outBuffer,
                                     size_t outBufferLenBytes,
                                     bool& isTruncated) {
  LOG_DEBUG_MSG(
      "CopyUtf8StringToSqlCharString is called with outBufferLenBytes is "
      << outBufferLenBytes);
  if (!inBuffer || (outBuffer && outBufferLenBytes == 0))
    return 0;

  if (ANSI_STRING_ONLY) {
    // the inBuffer contains ANSI characters only
    // If user are sure the strings in data source have only ANSI characters,
    // this function will copy the UTF8 characters from data source to user
    // buffer directly without converting UTF8 to wstring and then do a mapping
    // from unicode to ANSI characters.
    size_t inBufLen = strlen(inBuffer);
    if (inBufLen >= outBufferLenBytes) {
      strncpy(reinterpret_cast< char* >(outBuffer), inBuffer,
              outBufferLenBytes - 1);
      outBuffer[outBufferLenBytes - 1] = 0;
      isTruncated = true;
    } else {
      strncpy(reinterpret_cast< char* >(outBuffer), inBuffer, inBufLen);
      outBuffer[inBufLen] = 0;
      isTruncated = false;
    }
    return std::min(inBufLen, outBufferLenBytes - 1);
  } else {
    // the inBuffer may contain unicode characters
    // Need to convert input string to wide-char to get the
    // length in characters - as well as get .narrow() to work, as expected
    // Otherwise, it would be impossible to safely determine the
    // output buffer length needed.
    static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
        converter;
    std::wstring inString = converter.from_bytes(inBuffer);
    size_t inBufferLenChars = inString.size();
    LOG_DEBUG_MSG("inBufferLenChars is " << inBufferLenChars);

    // If no output buffer, return REQUIRED length.
    if (!outBuffer)
      return inBufferLenChars;

    size_t outBufferLenActual =
        std::min(inBufferLenChars, outBufferLenBytes - 1);

    std::locale currentLocale("");
    std::use_facet< std::ctype< wchar_t > >(currentLocale)
        .narrow(inString.data(), inString.data() + outBufferLenActual, '?',
                reinterpret_cast< char* >(outBuffer));

    outBuffer[outBufferLenActual] = 0;
    isTruncated = (outBufferLenActual < inBufferLenChars);

    LOG_DEBUG_MSG("outBufferLenActual is " << outBufferLenActual);
    return outBufferLenActual;
  }
}

template < typename OutCharT >
size_t CopyUtf8StringToWcharString(const char* inBuffer, OutCharT* outBuffer,
                                   size_t outBufferLenBytes,
                                   bool& isTruncated) {
  LOG_DEBUG_MSG(
      "CopyUtf8StringToWcharString is called with outBufferLenBytes is "
      << outBufferLenBytes);
  if (!inBuffer || (outBuffer && outBufferLenBytes == 0))
    return 0;

  size_t wCharSize = sizeof(SQLWCHAR);
  // This is intended to convert to the SQLWCHAR. Ensure we have the same size.
  assert(sizeof(OutCharT) == wCharSize);
  assert((outBufferLenBytes % wCharSize) == 0);

  // The number of characters that can be safely transfered, excluding the
  // null terminating character.
  size_t outBufferLenChars;
  // Find the lenght (in bytes) of the input string.
  // This does NOT include the null-terminating character.
  size_t inBufferLen = std::strlen(inBuffer);
  OutCharT* pOutBuffer;
  std::vector< OutCharT > targetProxy;

  if (outBuffer) {
    pOutBuffer = reinterpret_cast< OutCharT* >(outBuffer);
    outBufferLenChars = (outBufferLenBytes / wCharSize) - 1;
  } else {
    // Creates a proxy buffer so we can determine the required length.
    // This buffer will be ignored and automatically deleted after use.
    targetProxy.resize(inBufferLen + 1);
    pOutBuffer = targetProxy.data();
    outBufferLenChars = inBufferLen;
  }
  LOG_DEBUG_MSG("inBufferLen is " << inBufferLen << ", outBufferLenChars is "
                                  << outBufferLenChars);

  // Setup conversion facet.
  const std::codecvt_utf8< OutCharT > convFacet;
  std::mbstate_t convState = std::mbstate_t();
  // Pointer to next for input.
  const char* pInBufferNext;
  // Pointer to next for output.
  OutCharT* pOutBufferNext;

  // translate characters:
  const char* inBufferEnd = (inBuffer + inBufferLen);
  std::codecvt_base::result result =
      convFacet.in(convState, inBuffer, inBufferEnd, pInBufferNext, pOutBuffer,
                   pOutBuffer + outBufferLenChars, pOutBufferNext);
  LOG_DEBUG_MSG("result is " << result);

  size_t lenConverted = 0;
  switch (result) {
    case std::codecvt_base::ok:
    case std::codecvt_base::partial:
      // The number of characters converted (in OutCharT)
      lenConverted = pOutBufferNext - pOutBuffer;
      // null-terminate target string, if room
      pOutBuffer[lenConverted] = 0;

      isTruncated = (result == std::codecvt_base::partial
                     || (inBufferEnd != pInBufferNext));
      break;
    case std::codecvt_base::error:
      // Error returned if unable to convert character.
      LOG_ERROR_MSG("Unable to convert character '" << *pInBufferNext << "'");

      // The number of characters converted (in OutCharT)
      lenConverted = pOutBufferNext - pOutBuffer;
      // null-terminate target string, if room.
      pOutBuffer[lenConverted] = 0;

      isTruncated = true;
      break;
    default:
      // This situation occurs if the source and target are the same encoding.
      // Impossible?
      LOG_ERROR_MSG("Unexpected error converting string '" << inBuffer << "'");
      assert(false);
      break;
  }

  LOG_DEBUG_MSG("lenConverted is " << lenConverted);

  // Return the number of bytes transfered or required.
  return lenConverted * wCharSize;
}

size_t CopyUtf8StringToSqlWcharString(const char* inBuffer, SQLWCHAR* outBuffer,
                                      size_t outBufferLenBytes,
                                      bool& isTruncated) {
  LOG_DEBUG_MSG(
      "CopyUtf8StringToWcharString is called with outBufferLenBytes is "
      << outBufferLenBytes);
  if (!inBuffer)
    return 0;

  // Handles SQLWCHAR if either UTF-16 and UTF-32
  size_t wCharSize = sizeof(SQLWCHAR);
  LOG_DEBUG_MSG("wCharSize is " << wCharSize);
  switch (wCharSize) {
    case 2:
      return CopyUtf8StringToWcharString(
          inBuffer, reinterpret_cast< char16_t* >(outBuffer), outBufferLenBytes,
          isTruncated);
    case 4:
      return CopyUtf8StringToWcharString(
          inBuffer, reinterpret_cast< char32_t* >(outBuffer), outBufferLenBytes,
          isTruncated);
    default:
      LOG_ERROR_MSG("Unexpected error converting string '" << inBuffer << "'");
      assert(false);
      return 0;
  }
}

// High-level entry point to handle buffer size in either bytes or characters
size_t CopyStringToBuffer(const std::string& str, SQLWCHAR* buf, size_t buflen,
                          bool& isTruncated, bool isLenInBytes) {
  LOG_DEBUG_MSG("CopyStringToBuffer is called with buflen is "
                << buflen << ", isLenInBytes is " << isLenInBytes);
  size_t wCharSize = sizeof(SQLWCHAR);

  // Ensure non-zero length in bytes is a multiple of wide char size.
  assert(!isLenInBytes || (buflen % wCharSize == 0));

  // Convert buffer length to bytes.
  size_t bufLenInBytes = isLenInBytes ? buflen : buflen * wCharSize;
  isTruncated = false;
  size_t bytesWritten = CopyUtf8StringToSqlWcharString(
      str.c_str(), buf, bufLenInBytes, isTruncated);

  LOG_DEBUG_MSG("wCharSize is " << wCharSize << ", bufLenInBytes is "
                                << bufLenInBytes << ", bytesWritten is "
                                << bytesWritten);
  return isLenInBytes ? bytesWritten : bytesWritten / wCharSize;
}

std::string SqlWcharToString(const SQLWCHAR* sqlStr, int32_t sqlStrLen,
                             bool isLenInBytes) {
  LOG_DEBUG_MSG("SqlWcharToString is called with sqlStrLen is "
                << sqlStrLen << ", isLenInBytes is " << isLenInBytes);
  if (!sqlStr)
    return std::string();

  size_t char_size = sizeof(SQLWCHAR);

  assert(char_size == sizeof(wchar_t) || char_size == 2);

  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  std::wstring sqlStr0;
  if (sqlStrLen == SQL_NTS) {
    for (int i = 0; sqlStr[i] != 0; i++) {
      sqlStr0.push_back(sqlStr[i]);
    }
  } else if (sqlStrLen > 0) {
    size_t charsToCopy = isLenInBytes ? (sqlStrLen / char_size) : sqlStrLen;
    sqlStr0.reserve(charsToCopy + 1);
    for (int i = 0; i < charsToCopy && sqlStr[i] != 0; i++) {
      sqlStr0.push_back(sqlStr[i]);
    }
  }
  return converter.to_bytes(sqlStr0);
}

boost::optional< std::string > SqlWcharToOptString(const SQLWCHAR* sqlStr,
                                                   int32_t sqlStrLen,
                                                   bool isLenInBytes) {
  LOG_DEBUG_MSG("SqlWcharToOptString is called with sqlStrLen is "
                << sqlStrLen << ", isLenInBytes is " << isLenInBytes);
  if (!sqlStr)
    return boost::none;

  return SqlWcharToString(sqlStr, sqlStrLen, isLenInBytes);
}

// test sqlStr null case
std::string SqlCharToString(const SQLCHAR* sqlStr, int32_t sqlStrLen) {
  LOG_DEBUG_MSG("SqlCharToString is called with sqlStr is "
                << sqlStr << ", sqlStrLen is " << sqlStrLen);
  std::string res;

  const char* sqlStrC = reinterpret_cast< const char* >(sqlStr);

  if (!sqlStr || !sqlStrLen)
    return res;

  if (sqlStrLen == SQL_NTS)
    res.assign(sqlStrC);
  else if (sqlStrLen > 0)
    res.assign(sqlStrC, sqlStrLen);

  return res;
}

std::string ToUtf8(const std::wstring& value) {
  return ToUtf8(value.c_str());
}

std::string ToUtf8(const wchar_t* value) {
  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  return converter.to_bytes(value);
}

std::wstring FromUtf8(const std::string& value) {
  return FromUtf8(value.c_str());
}

std::wstring FromUtf8(const char* value) {
  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  return converter.from_bytes(value);
}

std::vector< SQLWCHAR > ToWCHARVector(const std::string& value) {
  return ToWCHARVector(value.c_str());
}

std::vector< SQLWCHAR > ToWCHARVector(const char* value) {
  size_t wCharSize = sizeof(SQLWCHAR);
  size_t inBufferLenBytes = std::strlen(value);
  // Handle worst-case scenario where there is a one-to-one mapping.
  std::vector< SQLWCHAR > outBuffer(inBufferLenBytes + 1);
  bool isTruncated = false;
  size_t length = CopyUtf8StringToSqlWcharString(
      value, outBuffer.data(), outBuffer.size() * wCharSize, isTruncated);
  outBuffer.resize((length / wCharSize) + 1);
  return outBuffer;
}

std::string HexDump(const void* data, size_t count) {
  std::stringstream dump;
  size_t cnt = 0;
  for (const uint8_t *p = (const uint8_t*)data,
                     *e = (const uint8_t*)data + count;
       p != e; ++p) {
    if (cnt++ % 16 == 0) {
      dump << std::endl;
    }
    dump << std::hex << std::setfill('0') << std::setw(2) << (int)*p << " ";
  }
  return dump.str();
}

std::string Ltrim(const std::string& s) {
  return std::regex_replace(s, std::regex("^\\s+"), std::string(""));
}

std::string Rtrim(const std::string& s) {
  return std::regex_replace(s, std::regex("\\s+$"), std::string(""));
}

std::string Trim(const std::string& s) {
  return Ltrim(Rtrim(s));
}

int UpdateRegexExpression(int index, int start, const std::string& pattern,
                          const std::string& str, std::string& converted) {
  LOG_DEBUG_MSG("UpdateRegexExpression is called with index is "
                << index << ", start is " << start << ", pattern is " << pattern
                << ", str is " << str);
  if (index - start > 0) {
    converted.append(pattern.substr(start, index - start));
  }
  converted.append(str);

  LOG_DEBUG_MSG("converted is " << converted);
  return index + 1;
}

std::string ConvertPatternToRegex(const std::string& pattern) {
  LOG_DEBUG_MSG("ConvertPatternToRegex is called with pattern is " << pattern);
  std::string converted("");
  if (pattern.empty() || Trim(pattern).empty()) {
    return converted;
  }

  bool escapeFound = false;
  int start = 0;
  for (int index = 0; index < pattern.length(); index++) {
    char currChar = pattern[index];
    if (currChar == '\\') {
      if (escapeFound) {
        // I.e., \\ - two backslash
        start =
            UpdateRegexExpression(index - 1, start, pattern, "[\\]", converted)
            + 1;
      }
      escapeFound = !escapeFound;
    } else if (escapeFound) {
      start =
          UpdateRegexExpression(index - 1, start, pattern,
                                "[" + std::string(1, currChar) + "]", converted)
          + 1;
      escapeFound = false;
    } else if (currChar == '_') {
      start = UpdateRegexExpression(index, start, pattern, ".", converted);
    } else if (currChar == '%') {
      start = UpdateRegexExpression(index, start, pattern, ".*", converted);
    }
  }
  // Handle the trailing std::string.
  if (pattern.length() - start > 0) {
    converted.append(pattern.substr(start));
  }
  LOG_DEBUG_MSG("converted is " << converted);
  return converted;
}

int StringToInt(const std::string& s, size_t* idx, int base) {
  LOG_DEBUG_MSG("StringToInt is called with s is "
                << s << ", idx is " << (idx ? *idx : -1) << ", base is "
                << base);
  if (s.empty())
    return 0;

  int retval = 0;

  try {
    retval = std::stoi(s, idx, base);
  } catch (std::exception& e) {
    LOG_ERROR_MSG("Failed to convert " << s << " to int, Exception caught: '"
                                       << e.what() << "'");
  }
  return retval;
}

long StringToLong(const std::string& s, size_t* idx, int base) {
  LOG_DEBUG_MSG("StringToLong is called with s is "
                << s << ", idx is " << (idx ? *idx : -1) << ", base is "
                << base);
  if (s.empty())
    return 0;

  long retval = 0;

  try {
    retval = std::stol(s, idx, base);
  } catch (std::exception& e) {
    LOG_ERROR_MSG("Failed to convert " << s << " to long, Exception caught: '"
                                       << e.what() << "'");
  }
  return retval;
}

bool CheckEnvVarSetToTrue(const std::string& envVar) {
  std::string envVarVal = ignite::odbc::common::GetEnv(envVar);
  std::transform(envVarVal.begin(), envVarVal.end(), envVarVal.begin(),
                 ::toupper);
  LOG_DEBUG_MSG(envVar << " is set to \"" << envVarVal << "\"");
  return envVarVal == "TRUE";
}

std::string GetFormatedDriverVersion() {
  std::stringstream formattedVersion;
  formattedVersion << std::setfill('0') << std::setw(2) << DRIVER_VERSION_MAJOR;
  formattedVersion << ".";
  formattedVersion << std::setfill('0') << std::setw(2) << DRIVER_VERSION_MINOR;
  formattedVersion << ".";
  formattedVersion << std::setfill('0') << std::setw(4) << DRIVER_VERSION_PATCH;
  return formattedVersion.str();
}
}  // namespace utility
}  // namespace odbc
}  // namespace trino
