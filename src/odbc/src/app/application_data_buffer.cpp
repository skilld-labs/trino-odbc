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

#include "timestream/odbc/app/application_data_buffer.h"

#include <algorithm>
#include <codecvt>
#include <sstream>
#include <string>
#include <vector>

#include <sqltypes.h>
#include "timestream/odbc/log.h"
#include "timestream/odbc/system/odbc_constants.h"
#include "timestream/odbc/utility.h"

namespace timestream {
namespace odbc {
namespace app {
using namespace type_traits;

ApplicationDataBuffer::ApplicationDataBuffer()
    : type(type_traits::OdbcNativeType::AI_UNSUPPORTED),
      buffer(0),
      buflen(0),
      reslen(0),
      byteOffset(0),
      elementOffset(0) {
  // No-op.
}

ApplicationDataBuffer::ApplicationDataBuffer(
    type_traits::OdbcNativeType::Type type, void* buffer, SqlLen buflen,
    SqlLen* reslen)
    : type(type),
      buffer(buffer),
      buflen(buflen),
      reslen(reslen),
      byteOffset(0),
      elementOffset(0) {
  // No-op.
}

ApplicationDataBuffer::ApplicationDataBuffer(const ApplicationDataBuffer& other)
    : type(other.type),
      buffer(other.buffer),
      buflen(other.buflen),
      reslen(other.reslen),
      byteOffset(other.byteOffset),
      elementOffset(other.elementOffset) {
  // No-op.
}

ApplicationDataBuffer::~ApplicationDataBuffer() {
  // No-op.
}

ApplicationDataBuffer& ApplicationDataBuffer::operator=(
    const ApplicationDataBuffer& other) {
  type = other.type;
  buffer = other.buffer;
  buflen = other.buflen;
  reslen = other.reslen;
  byteOffset = other.byteOffset;
  elementOffset = other.elementOffset;

  return *this;
}

template < typename T >
ConversionResult::Type ApplicationDataBuffer::PutNum(T value) {
  using namespace type_traits;

  LOG_DEBUG_MSG("PutNum is called with value: " << value);

  SqlLen* resLenPtr = GetResLen();
  void* dataPtr = GetData();

  LOG_DEBUG_MSG("type: " << type);

  switch (type) {
    case OdbcNativeType::AI_SIGNED_TINYINT: {
      return PutNumToNumBuffer< signed char >(value);
    }

    case OdbcNativeType::AI_BIT:
    case OdbcNativeType::AI_UNSIGNED_TINYINT: {
      return PutNumToNumBuffer< unsigned char >(value);
    }

    case OdbcNativeType::AI_SIGNED_SHORT: {
      return PutNumToNumBuffer< SQLSMALLINT >(value);
    }

    case OdbcNativeType::AI_UNSIGNED_SHORT: {
      return PutNumToNumBuffer< SQLUSMALLINT >(value);
    }

    case OdbcNativeType::AI_SIGNED_LONG: {
      return PutNumToNumBuffer< SQLINTEGER >(value);
    }

    case OdbcNativeType::AI_UNSIGNED_LONG: {
      return PutNumToNumBuffer< SQLUINTEGER >(value);
    }

    case OdbcNativeType::AI_SIGNED_BIGINT: {
      return PutNumToNumBuffer< SQLBIGINT >(value);
    }

    case OdbcNativeType::AI_UNSIGNED_BIGINT: {
      return PutNumToNumBuffer< SQLUBIGINT >(value);
    }

    case OdbcNativeType::AI_FLOAT: {
      return PutNumToNumBuffer< SQLREAL >(value);
    }

    case OdbcNativeType::AI_DOUBLE: {
      return PutNumToNumBuffer< SQLDOUBLE >(value);
    }

    case OdbcNativeType::AI_CHAR: {
      return PutValToStrBuffer< char >(value);
    }

    case OdbcNativeType::AI_WCHAR: {
      return PutValToStrBuffer< SQLWCHAR >(value);
    }

    case OdbcNativeType::AI_NUMERIC: {
      if (dataPtr) {
        SQL_NUMERIC_STRUCT* out =
            reinterpret_cast< SQL_NUMERIC_STRUCT* >(dataPtr);

        uint64_t uval = static_cast< uint64_t >(value < 0 ? -value : value);

        out->precision = ignite::odbc::common::bits::DigitLength(uval);
        out->scale = 0;
        out->sign = value < 0 ? 0 : 1;

        memset(out->val, 0, SQL_MAX_NUMERIC_LEN);

        memcpy(out->val, &uval,
               std::min< int >(SQL_MAX_NUMERIC_LEN, sizeof(uval)));
        LOG_DEBUG_MSG("out->val: " << out->val);
      }

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_NUMERIC_STRUCT));

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_BINARY:
    case OdbcNativeType::AI_DEFAULT: {
      if (dataPtr)
        memcpy(dataPtr, &value,
               std::min(sizeof(value), static_cast< size_t >(buflen)));

      if (resLenPtr)
        *resLenPtr = sizeof(value);

      return static_cast< size_t >(buflen) < sizeof(value)
                 ? ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED
                 : ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_TDATE: {
      return PutDate(Date(static_cast< int64_t >(value)));
    }

    case OdbcNativeType::AI_TTIMESTAMP: {
      return PutTimestamp(Timestamp(static_cast< int64_t >(value)));
    }

    case OdbcNativeType::AI_TTIME: {
      return PutTime(Time(static_cast< int64_t >(value)));
    }

    default:
      break;
  }

  return ConversionResult::Type::AI_UNSUPPORTED_CONVERSION;
}

template < typename Tbuf, typename Tin >
ConversionResult::Type ApplicationDataBuffer::PutNumToNumBuffer(Tin value) {
  LOG_DEBUG_MSG("PutNumToNumBuffer is called with value " << value);
  void* dataPtr = GetData();
  SqlLen* resLenPtr = GetResLen();

  if (dataPtr) {
    Tbuf* out = reinterpret_cast< Tbuf* >(dataPtr);
    *out = static_cast< Tbuf >(value);
  }

  if (resLenPtr)
    *resLenPtr = static_cast< SqlLen >(sizeof(Tbuf));

  return ConversionResult::Type::AI_SUCCESS;
}

template < typename CharT, typename Tin >
ConversionResult::Type ApplicationDataBuffer::PutValToStrBuffer(
    const Tin& value) {
  LOG_DEBUG_MSG("PutValToStrBuffer is called with value " << value);
  std::stringstream converter;
  converter << value;
  int32_t written = 0;
  return PutStrToStrBuffer< CharT >(converter.str(), written);
}

template < typename CharT >
ConversionResult::Type ApplicationDataBuffer::PutValToStrBuffer(
    const int8_t& value) {
  LOG_DEBUG_MSG("PutValToStrBuffer is called with value " << value);
  std::stringstream converter;
  // NOTE: Need to cast to larger integer - or will mistake it for a character.
  converter << static_cast< int32_t >(value);
  int32_t written = 0;
  return PutStrToStrBuffer< CharT >(converter.str(), written);
}

template < typename OutCharT, typename InCharT >
ConversionResult::Type ApplicationDataBuffer::PutStrToStrBuffer(
    const std::basic_string< InCharT >& value, int32_t& written) {
  LOG_DEBUG_MSG("PutStrToStrBuffer is called with value " << value);
  written = 0;

  SqlLen inCharSize = static_cast< SqlLen >(sizeof(InCharT));
  SqlLen outCharSize = static_cast< SqlLen >(sizeof(OutCharT));
  LOG_DEBUG_MSG("inCharSize is " << inCharSize << ", outCharSize is "
                                 << outCharSize << ", buflen is " << buflen);

  SqlLen* resLenPtr = GetResLen();
  void* dataPtr = GetData();

  if (!dataPtr)
    return ConversionResult::Type::AI_SUCCESS;

  if (buflen < outCharSize)
    return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;

  size_t lenWrittenOrRequired = 0;
  bool isTruncated = false;
  if (inCharSize == 1) {
    if (outCharSize == 2 || outCharSize == 4) {
      lenWrittenOrRequired = utility::CopyUtf8StringToSqlWcharString(
          reinterpret_cast< const char* >(value.c_str()),
          reinterpret_cast< SQLWCHAR* >(dataPtr), buflen, isTruncated);
    } else if (sizeof(OutCharT) == 1) {
      lenWrittenOrRequired = utility::CopyUtf8StringToSqlCharString(
          reinterpret_cast< const char* >(value.c_str()),
          reinterpret_cast< SQLCHAR* >(dataPtr), buflen, isTruncated);
    } else {
      LOG_ERROR_MSG("Unexpected conversion from UTF8 string.");
      assert(false);
    }
  } else {
    LOG_ERROR_MSG(
        "Unexpected conversion from unknown type string, char size is "
        << inCharSize);
    assert(false);
  }

  written = static_cast< int32_t >(lenWrittenOrRequired);
  LOG_DEBUG_MSG("written is " << written);
  if (resLenPtr) {
    *resLenPtr = static_cast< SqlLen >(written);
  }

  if (isTruncated)
    return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;

  return ConversionResult::Type::AI_SUCCESS;
}

ConversionResult::Type ApplicationDataBuffer::PutRawDataToBuffer(
    const void* data, size_t len, int32_t& written) {
  LOG_DEBUG_MSG("PutRawDataToBuffer is called with len " << len);
  SqlLen iLen = static_cast< SqlLen >(len);

  SqlLen* resLenPtr = GetResLen();
  void* dataPtr = GetData();

  if (resLenPtr)
    *resLenPtr = iLen;

  SqlLen toCopy = std::min(buflen, iLen);

  if (dataPtr != 0 && toCopy > 0)
    memcpy(dataPtr, data, static_cast< size_t >(toCopy));

  written = static_cast< int32_t >(toCopy);
  LOG_DEBUG_MSG("written is " << written);

  return toCopy < iLen ? ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED
                       : ConversionResult::Type::AI_SUCCESS;
}

ConversionResult::Type ApplicationDataBuffer::PutInt8(
    boost::optional< int8_t > value) {
  LOG_DEBUG_MSG("PutInt8 is called");
  if (value)
    return PutInt8(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutInt8(int8_t value) {
  return PutNum(value);
}

ConversionResult::Type ApplicationDataBuffer::PutInt16(
    boost::optional< int16_t > value) {
  LOG_DEBUG_MSG("PutInt16 is called");
  if (value)
    return PutInt16(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutInt16(int16_t value) {
  return PutNum(value);
}

ConversionResult::Type ApplicationDataBuffer::PutInt32(
    boost::optional< int32_t > value) {
  LOG_DEBUG_MSG("PutInt32 is called");
  if (value)
    return PutInt32(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutInt32(int32_t value) {
  return PutNum(value);
}

ConversionResult::Type ApplicationDataBuffer::PutInt64(
    boost::optional< int64_t > value) {
  LOG_DEBUG_MSG("PutInt64 is called");
  if (value)
    return PutInt64(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutInt64(int64_t value) {
  return PutNum(value);
}

ConversionResult::Type ApplicationDataBuffer::PutFloat(
    boost::optional< float > value) {
  LOG_DEBUG_MSG("PutFloat is called");
  if (value)
    return PutFloat(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutFloat(float value) {
  return PutNum(value);
}

ConversionResult::Type ApplicationDataBuffer::PutDouble(
    boost::optional< double > value) {
  LOG_DEBUG_MSG("PutDouble is called");
  if (value)
    return PutDouble(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutDouble(double value) {
  return PutNum(value);
}

ConversionResult::Type ApplicationDataBuffer::PutString(
    const boost::optional< std::string >& value) {
  LOG_DEBUG_MSG("PutString is called");
  if (value)
    return PutString(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutString(
    const std::string& value) {
  int32_t written = 0;

  return PutString(value, written);
}

ConversionResult::Type ApplicationDataBuffer::PutString(
    const std::string& value, int32_t& written) {
  using namespace type_traits;
  LOG_DEBUG_MSG("PutString is called with value " << value << ", type is "
                                                  << type);

  switch (type) {
    case OdbcNativeType::AI_SIGNED_TINYINT:
    case OdbcNativeType::AI_BIT:
    case OdbcNativeType::AI_UNSIGNED_TINYINT:
    case OdbcNativeType::AI_SIGNED_SHORT:
    case OdbcNativeType::AI_UNSIGNED_SHORT:
    case OdbcNativeType::AI_SIGNED_LONG:
    case OdbcNativeType::AI_UNSIGNED_LONG:
    case OdbcNativeType::AI_SIGNED_BIGINT:
    case OdbcNativeType::AI_UNSIGNED_BIGINT:
    case OdbcNativeType::AI_NUMERIC: {
      std::stringstream converter;

      converter << value;

      int64_t numValue;

      converter >> numValue;

      written = static_cast< int32_t >(value.size());

      return PutNum(numValue);
    }

    case OdbcNativeType::AI_FLOAT:
    case OdbcNativeType::AI_DOUBLE: {
      std::stringstream converter;

      converter << value;

      double numValue;

      converter >> numValue;

      written = static_cast< int32_t >(value.size());

      return PutNum(numValue);
    }

    case OdbcNativeType::AI_CHAR:
    case OdbcNativeType::AI_BINARY:
    case OdbcNativeType::AI_DEFAULT: {
      return PutStrToStrBuffer< char >(value, written);
    }

    case OdbcNativeType::AI_WCHAR: {
      return PutStrToStrBuffer< SQLWCHAR >(value, written);
    }

    default:
      break;
  }

  return ConversionResult::Type::AI_UNSUPPORTED_CONVERSION;
}

ConversionResult::Type ApplicationDataBuffer::PutNull() {
  LOG_DEBUG_MSG("PutNull is called. No data put into buffer");

  SqlLen* resLenPtr = GetResLen();

  if (!resLenPtr)
    return ConversionResult::Type::AI_INDICATOR_NEEDED;

  *resLenPtr = SQL_NULL_DATA;

  return ConversionResult::Type::AI_SUCCESS;
}

ConversionResult::Type ApplicationDataBuffer::PutDecimal(
    const boost::optional< ignite::odbc::common::Decimal >& value) {
  LOG_DEBUG_MSG("PutDecimal is called");
  if (value)
    return PutDecimal(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutDecimal(
    const ignite::odbc::common::Decimal& value) {
  LOG_DEBUG_MSG("PutDecimal is called with type " << type);
  using namespace type_traits;

  SqlLen* resLenPtr = GetResLen();
  switch (type) {
    case OdbcNativeType::AI_SIGNED_TINYINT:
    case OdbcNativeType::AI_BIT:
    case OdbcNativeType::AI_UNSIGNED_TINYINT:
    case OdbcNativeType::AI_SIGNED_SHORT:
    case OdbcNativeType::AI_UNSIGNED_SHORT:
    case OdbcNativeType::AI_SIGNED_LONG:
    case OdbcNativeType::AI_UNSIGNED_LONG:
    case OdbcNativeType::AI_SIGNED_BIGINT:
    case OdbcNativeType::AI_UNSIGNED_BIGINT: {
      PutNum< int64_t >(value.ToInt64());

      return ConversionResult::Type::AI_FRACTIONAL_TRUNCATED;
    }

    case OdbcNativeType::AI_FLOAT:
    case OdbcNativeType::AI_DOUBLE: {
      PutNum< double >(value.ToDouble());

      return ConversionResult::Type::AI_FRACTIONAL_TRUNCATED;
    }

    case OdbcNativeType::AI_CHAR:
    case OdbcNativeType::AI_WCHAR: {
      std::stringstream converter;

      converter << value;

      int32_t dummy = 0;

      return PutString(converter.str(), dummy);
    }

    case OdbcNativeType::AI_NUMERIC: {
      SQL_NUMERIC_STRUCT* numeric =
          reinterpret_cast< SQL_NUMERIC_STRUCT* >(GetData());

      ignite::odbc::common::Decimal zeroScaled;
      value.SetScale(0, zeroScaled);

      ignite::odbc::common::FixedSizeArray< int8_t > bytesBuffer;

      const ignite::odbc::common::BigInteger& unscaled =
          zeroScaled.GetUnscaledValue();

      unscaled.MagnitudeToBytes(bytesBuffer);

      for (int32_t i = 0; i < SQL_MAX_NUMERIC_LEN; ++i) {
        int32_t bufIdx = bytesBuffer.GetSize() - 1 - i;
        if (bufIdx >= 0)
          numeric->val[i] = bytesBuffer[bufIdx];
        else
          numeric->val[i] = 0;
      }

      numeric->scale = 0;
      numeric->sign = unscaled.GetSign() < 0 ? 0 : 1;
      numeric->precision = unscaled.GetPrecision();

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_NUMERIC_STRUCT));

      if (bytesBuffer.GetSize() > SQL_MAX_NUMERIC_LEN)
        return ConversionResult::Type::AI_FRACTIONAL_TRUNCATED;

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_DEFAULT:
    case OdbcNativeType::AI_BINARY:
    default:
      break;
  }

  return ConversionResult::Type::AI_UNSUPPORTED_CONVERSION;
}

ConversionResult::Type ApplicationDataBuffer::PutDate(
    const boost::optional< Date >& value) {
  LOG_DEBUG_MSG("PutDate is called");
  if (value)
    return PutDate(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutDate(const Date& value) {
  LOG_DEBUG_MSG("PutDate is called with type " << type);
  using namespace type_traits;

  tm tmTime;

  timestream::odbc::common::DateToCTm(value, tmTime);
  LOG_DEBUG_MSG("tmTime.tm_year "
                << tmTime.tm_year << ", tmTime.tm_mon " << tmTime.tm_mon
                << ", tmTime.tm_mday " << tmTime.tm_mday << ", tmTime.tm_hour "
                << tmTime.tm_hour << ", tmTime.tm_min " << tmTime.tm_min
                << ", tmTime.tm_sec " << tmTime.tm_sec << ", tmTime.tm_wday "
                << tmTime.tm_wday << ", tmTime.tm_yday " << tmTime.tm_yday
                << ", tmTime.tm_isdst " << tmTime.tm_isdst);

  SqlLen* resLenPtr = GetResLen();
  void* dataPtr = GetData();

  switch (type) {
    case OdbcNativeType::AI_CHAR: {
      char* buffer = reinterpret_cast< char* >(dataPtr);
      const size_t valLen = sizeof("HHHH-MM-DD") - 1;

      if (resLenPtr)
        *resLenPtr = valLen;

      if (buffer) {
        char tmpStr[32]{};
        snprintf(tmpStr, 32, "%4d-%02d-%02d", tmTime.tm_year + 1900,
                 tmTime.tm_mon + 1, tmTime.tm_mday);
        strncpy(buffer, tmpStr,
                std::min(buflen, static_cast< SqlLen >(valLen + 1)));

        LOG_DEBUG_MSG("valLen is " << valLen << ", buflen is " << buflen
                                   << ", tmpStr is " << tmpStr);
        if (static_cast< SqlLen >(valLen) + 1 > buflen) {
          buffer[buflen - 1] = 0;
          return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;
        }
      }

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_WCHAR: {
      SQLWCHAR* buffer = reinterpret_cast< SQLWCHAR* >(dataPtr);
      const size_t valLen = sizeof("HHHH-MM-DD") - 1;

      if (resLenPtr)
        *resLenPtr = valLen;

      if (buffer) {
        char tmpStr[32]{};
        snprintf(tmpStr, 32, "%4d-%02d-%02d", tmTime.tm_year + 1900,
                 tmTime.tm_mon + 1, tmTime.tm_mday);

        LOG_DEBUG_MSG("valLen is " << valLen << ", buflen is " << buflen
                                   << ", tmpStr is " << tmpStr);
        bool isTruncated = false;
        utility::CopyStringToBuffer(tmpStr, buffer, GetSize(), isTruncated,
                                    true);
      }

      if (static_cast< SqlLen >(valLen) + 1 > GetSize())
        return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_TDATE: {
      SQL_DATE_STRUCT* buffer = reinterpret_cast< SQL_DATE_STRUCT* >(dataPtr);

      buffer->year = tmTime.tm_year + 1900;
      buffer->month = tmTime.tm_mon + 1;
      buffer->day = tmTime.tm_mday;

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_DATE_STRUCT));

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_TTIMESTAMP: {
      SQL_TIMESTAMP_STRUCT* buffer =
          reinterpret_cast< SQL_TIMESTAMP_STRUCT* >(dataPtr);

      buffer->year = tmTime.tm_year + 1900;
      buffer->month = tmTime.tm_mon + 1;
      buffer->day = tmTime.tm_mday;
      buffer->hour = tmTime.tm_hour;
      buffer->minute = tmTime.tm_min;
      buffer->second = tmTime.tm_sec;
      buffer->fraction = 0;

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_TIMESTAMP_STRUCT));

      return ConversionResult::Type::AI_SUCCESS;
    }

    default:
      break;
  }

  return ConversionResult::Type::AI_UNSUPPORTED_CONVERSION;
}

std::string ApplicationDataBuffer::GetTimestampString(tm& tmTime,
                                                      int32_t fraction,
                                                      const char* pattern) {
  LOG_DEBUG_MSG("GetTimestampString is called with pattern " << pattern);
  char result[64]{};
  size_t writtenLen = strftime(result, 48, pattern, &tmTime);

  snprintf(result + writtenLen, 16, "%09d", fraction);

  LOG_DEBUG_MSG("result is " << result);
  return std::string(result);
}

ConversionResult::Type ApplicationDataBuffer::PutTimestamp(
    const boost::optional< Timestamp >& value) {
  LOG_DEBUG_MSG("PutTimestamp is called");
  if (value)
    return PutTimestamp(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutTimestamp(
    const Timestamp& value) {
  LOG_DEBUG_MSG("PutTimestamp is called with type " << type);
  tm tmTime;
  memset(&tmTime, 0, sizeof(tm));

  timestream::odbc::common::TimestampToCTm(value, tmTime);
  LOG_DEBUG_MSG("tmTime.tm_year "
                << tmTime.tm_year << ", tmTime.tm_mon " << tmTime.tm_mon
                << ", tmTime.tm_mday " << tmTime.tm_mday << ", tmTime.tm_hour "
                << tmTime.tm_hour << ", tmTime.tm_min " << tmTime.tm_min
                << ", tmTime.tm_sec " << tmTime.tm_sec << ", tmTime.tm_wday "
                << tmTime.tm_wday << ", tmTime.tm_yday " << tmTime.tm_yday
                << ", tmTime.tm_isdst " << tmTime.tm_isdst);

  SqlLen* resLenPtr = GetResLen();
  void* dataPtr = GetData();

  switch (type) {
    case OdbcNativeType::AI_CHAR: {
      const size_t valLen = sizeof("HHHH-MM-DD HH:MM:SS.xxxxxxxxx");

      if (resLenPtr)
        *resLenPtr = valLen - 1;

      char* buffer = reinterpret_cast< char* >(dataPtr);
      if (buffer) {
        std::string tmpStr = GetTimestampString(
            tmTime, value.GetSecondFraction(), "%Y-%m-%d %H:%M:%S.");
        strncpy(buffer, tmpStr.c_str(),
                std::min(static_cast< SqlLen >(valLen), buflen));

        LOG_DEBUG_MSG("valLen is " << valLen << ", buflen is " << buflen
                                   << ", tmpStr is " << tmpStr);
        if (static_cast< SqlLen >(valLen) > buflen) {
          buffer[buflen - 1] = 0;
          return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;
        }
      }
      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_WCHAR: {
      const size_t valLen = sizeof("HHHH-MM-DD HH:MM:SS.xxxxxxxxx");

      if (resLenPtr)
        *resLenPtr = valLen - 1;

      SQLWCHAR* buffer = reinterpret_cast< SQLWCHAR* >(dataPtr);

      if (buffer) {
        std::string tmpStr = GetTimestampString(
            tmTime, value.GetSecondFraction(), "%Y-%m-%d %H:%M:%S.");

        LOG_DEBUG_MSG("valLen is " << valLen << ", buflen is " << buflen
                                   << ", tmpStr is " << tmpStr);
        bool isTruncated = false;
        utility::CopyStringToBuffer(&tmpStr[0], buffer, buflen, isTruncated,
                                    true);

        if (isTruncated) {
          return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;
        }
      }

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_TDATE: {
      SQL_DATE_STRUCT* buffer = reinterpret_cast< SQL_DATE_STRUCT* >(dataPtr);

      buffer->year = tmTime.tm_year + 1900;
      buffer->month = tmTime.tm_mon + 1;
      buffer->day = tmTime.tm_mday;

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_DATE_STRUCT));

      return ConversionResult::Type::AI_FRACTIONAL_TRUNCATED;
    }

    case OdbcNativeType::AI_TTIME: {
      SQL_TIME_STRUCT* buffer = reinterpret_cast< SQL_TIME_STRUCT* >(dataPtr);

      buffer->hour = tmTime.tm_hour;
      buffer->minute = tmTime.tm_min;
      buffer->second = tmTime.tm_sec;

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_TIME_STRUCT));

      return ConversionResult::Type::AI_FRACTIONAL_TRUNCATED;
    }

    case OdbcNativeType::AI_TTIMESTAMP: {
      SQL_TIMESTAMP_STRUCT* buffer =
          reinterpret_cast< SQL_TIMESTAMP_STRUCT* >(dataPtr);

      buffer->year = tmTime.tm_year + 1900;
      buffer->month = tmTime.tm_mon + 1;
      buffer->day = tmTime.tm_mday;
      buffer->hour = tmTime.tm_hour;
      buffer->minute = tmTime.tm_min;
      buffer->second = tmTime.tm_sec;
      buffer->fraction = value.GetSecondFraction();

      LOG_DEBUG_MSG("buffer content is "
                    << buffer->year << " " << buffer->month << " "
                    << buffer->day << " " << buffer->hour << ":"
                    << buffer->minute << ":" << buffer->second << "."
                    << buffer->fraction);

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_TIMESTAMP_STRUCT));

      return ConversionResult::Type::AI_SUCCESS;
    }

    default:
      break;
  }

  return ConversionResult::Type::AI_UNSUPPORTED_CONVERSION;
}

ConversionResult::Type ApplicationDataBuffer::PutTime(
    const boost::optional< Time >& value) {
  LOG_DEBUG_MSG("PutTime is called");
  if (value)
    return PutTime(*value);
  else
    return PutNull();
}

ConversionResult::Type ApplicationDataBuffer::PutTime(const Time& value) {
  LOG_DEBUG_MSG("PutTime is called with type " << type);
  using namespace type_traits;

  tm tmTime;

  memset(&tmTime, 0, sizeof(tm));
  timestream::odbc::common::TimeToCTm(value, tmTime);
  LOG_DEBUG_MSG("tmTime.tm_year "
                << tmTime.tm_year << ", tmTime.tm_mon " << tmTime.tm_mon
                << ", tmTime.tm_mday " << tmTime.tm_mday << ", tmTime.tm_hour "
                << tmTime.tm_hour << ", tmTime.tm_min " << tmTime.tm_min
                << ", tmTime.tm_sec " << tmTime.tm_sec << ", tmTime.tm_wday "
                << tmTime.tm_wday << ", tmTime.tm_yday " << tmTime.tm_yday
                << ", tmTime.tm_isdst " << tmTime.tm_isdst);

  SqlLen* resLenPtr = GetResLen();
  void* dataPtr = GetData();

  switch (type) {
    case OdbcNativeType::AI_CHAR: {
      const size_t valLen = sizeof("HH:MM:SS.xxxxxxxxx");

      if (resLenPtr)
        *resLenPtr = valLen - 1;

      char* buffer = reinterpret_cast< char* >(dataPtr);

      if (buffer) {
        std::string tmpStr =
            GetTimestampString(tmTime, value.GetSecondFraction(), "%H:%M:%S.");
        strncpy(buffer, &tmpStr[0],
                std::min(static_cast< SqlLen >(valLen), buflen));

        LOG_DEBUG_MSG("valLen is " << valLen << ", buflen is " << buflen
                                   << ", tmpStr is " << tmpStr);
        if (static_cast< SqlLen >(valLen) > buflen) {
          buffer[buflen - 1] = 0;
          return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;
        }
      }

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_WCHAR: {
      const size_t valLen = sizeof("HH:MM:SS.xxxxxxxxx");

      if (resLenPtr)
        *resLenPtr = valLen - 1;

      SQLWCHAR* buffer = reinterpret_cast< SQLWCHAR* >(dataPtr);

      if (buffer) {
        std::string tmpStr =
            GetTimestampString(tmTime, value.GetSecondFraction(), "%H:%M:%S.");

        LOG_DEBUG_MSG("valLen is " << valLen << ", buflen is " << buflen
                                   << ", tmpStr is " << tmpStr);
        bool isTruncated = false;
        utility::CopyStringToBuffer(&tmpStr[0], buffer, buflen, isTruncated,
                                    true);

        if (isTruncated) {
          return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;
        }
      }

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_TTIME: {
      SQL_TIME_STRUCT* buffer = reinterpret_cast< SQL_TIME_STRUCT* >(dataPtr);

      buffer->hour = tmTime.tm_hour;
      buffer->minute = tmTime.tm_min;
      buffer->second = tmTime.tm_sec;

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_TIME_STRUCT));

      return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;
    }

    case OdbcNativeType::AI_TTIMESTAMP: {
      SQL_TIMESTAMP_STRUCT* buffer =
          reinterpret_cast< SQL_TIMESTAMP_STRUCT* >(dataPtr);

      buffer->year = tmTime.tm_year + 1900;
      buffer->month = tmTime.tm_mon + 1;
      buffer->day = tmTime.tm_mday;
      buffer->hour = tmTime.tm_hour;
      buffer->minute = tmTime.tm_min;
      buffer->second = tmTime.tm_sec;
      buffer->fraction = value.GetSecondFraction();

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_TIMESTAMP_STRUCT));

      return ConversionResult::Type::AI_SUCCESS;
    }

    default:
      break;
  }

  return ConversionResult::Type::AI_UNSUPPORTED_CONVERSION;
}

ConversionResult::Type ApplicationDataBuffer::PutInterval(
    const IntervalYearMonth& value) {
  LOG_DEBUG_MSG("PutInterval is called with type " << type);
  using namespace type_traits;

  SqlLen* resLenPtr = GetResLen();
  void* dataPtr = GetData();

  switch (type) {
    case OdbcNativeType::AI_CHAR: {
      char tmp[32]{};
      snprintf(tmp, 32, "%d-%d", value.GetYear(), value.GetMonth());
      SqlLen resLen = static_cast< SqlLen >(strlen(tmp));

      if (resLenPtr)
        *resLenPtr = resLen;

      char* buffer = reinterpret_cast< char* >(dataPtr);
      if (buffer) {
        strncpy(buffer, tmp, std::min(resLen + 1, buflen));

        LOG_DEBUG_MSG("resLen is " << resLen << ", buflen is " << buflen
                                   << ", tmp is " << tmp);
        if (resLen + 1 > buflen) {
          buffer[buflen - 1] = 0;
          return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;
        }
      }

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_WCHAR: {
      char tmp[32]{};
      snprintf(tmp, 32, "%d-%d", value.GetYear(), value.GetMonth());
      SqlLen resLen = static_cast< SqlLen >(strlen(tmp));

      if (resLenPtr)
        *resLenPtr = resLen;

      SQLWCHAR* buffer = reinterpret_cast< SQLWCHAR* >(dataPtr);
      if (buffer) {
        bool isTruncated = false;
        utility::CopyStringToBuffer(tmp, buffer, buflen, isTruncated, true);

        LOG_DEBUG_MSG("resLen is " << resLen << ", buflen is " << buflen
                                   << ", tmp is " << tmp);
        if (isTruncated)
          return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;
      }

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_INTERVAL_YEAR:
    case OdbcNativeType::AI_INTERVAL_MONTH:
    case OdbcNativeType::AI_INTERVAL_DAY:
    case OdbcNativeType::AI_INTERVAL_HOUR:
    case OdbcNativeType::AI_INTERVAL_MINUTE:
    case OdbcNativeType::AI_INTERVAL_SECOND:
    case OdbcNativeType::AI_INTERVAL_DAY_TO_HOUR:
    case OdbcNativeType::AI_INTERVAL_DAY_TO_MINUTE:
    case OdbcNativeType::AI_INTERVAL_HOUR_TO_MINUTE:
    case OdbcNativeType::AI_INTERVAL_HOUR_TO_SECOND:
    case OdbcNativeType::AI_INTERVAL_MINUTE_TO_SECOND:
    case OdbcNativeType::AI_INTERVAL_YEAR_TO_MONTH:
    case OdbcNativeType::AI_INTERVAL_DAY_TO_SECOND: {
      SQL_INTERVAL_STRUCT* buffer =
          reinterpret_cast< SQL_INTERVAL_STRUCT* >(dataPtr);
      SetIntervalBufferValue(buffer, value);

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_INTERVAL_STRUCT));

      return ConversionResult::Type::AI_SUCCESS;
    }

    default:
      break;
  }

  return ConversionResult::Type::AI_UNSUPPORTED_CONVERSION;
}

ConversionResult::Type ApplicationDataBuffer::PutInterval(
    const IntervalDaySecond& value) {
  LOG_DEBUG_MSG("PutInterval is called with type " << type);
  using namespace type_traits;

  SqlLen* resLenPtr = GetResLen();
  void* dataPtr = GetData();

  switch (type) {
    case OdbcNativeType::AI_CHAR: {
      char tmp[32]{};
      snprintf(tmp, 32, "%d %02d:%02d:%02d.%09d", value.GetDay(),
               value.GetHour(), value.GetMinute(), value.GetSecond(),
               value.GetFraction());
      SqlLen resLen = static_cast< SqlLen >(strlen(tmp));

      if (resLenPtr)
        *resLenPtr = resLen;

      char* buffer = reinterpret_cast< char* >(dataPtr);
      if (buffer) {
        strncpy(buffer, tmp, std::min(resLen + 1, buflen));

        LOG_DEBUG_MSG("resLen is " << resLen << ", buflen is " << buflen
                                   << ", tmp is " << tmp);
        if (resLen + 1 > buflen) {
          buffer[buflen - 1] = 0;
          return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;
        }
      }

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_WCHAR: {
      char tmp[32]{};
      snprintf(tmp, 32, "%d %02d:%02d:%02d.%09d", value.GetDay(),
               value.GetHour(), value.GetMinute(), value.GetSecond(),
               value.GetFraction());
      SqlLen resLen = static_cast< SqlLen >(strlen(tmp));

      if (resLenPtr)
        *resLenPtr = resLen;

      SQLWCHAR* buffer = reinterpret_cast< SQLWCHAR* >(dataPtr);
      if (buffer) {
        bool isTruncated = false;
        utility::CopyStringToBuffer(tmp, buffer, buflen, isTruncated, true);

        LOG_DEBUG_MSG("resLen is " << resLen << ", buflen is " << buflen
                                   << ", tmp is " << tmp);
        if (isTruncated)
          return ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED;
      }

      return ConversionResult::Type::AI_SUCCESS;
    }

    case OdbcNativeType::AI_INTERVAL_YEAR:
    case OdbcNativeType::AI_INTERVAL_MONTH:
    case OdbcNativeType::AI_INTERVAL_DAY:
    case OdbcNativeType::AI_INTERVAL_HOUR:
    case OdbcNativeType::AI_INTERVAL_MINUTE:
    case OdbcNativeType::AI_INTERVAL_SECOND:
    case OdbcNativeType::AI_INTERVAL_DAY_TO_HOUR:
    case OdbcNativeType::AI_INTERVAL_DAY_TO_MINUTE:
    case OdbcNativeType::AI_INTERVAL_HOUR_TO_MINUTE:
    case OdbcNativeType::AI_INTERVAL_HOUR_TO_SECOND:
    case OdbcNativeType::AI_INTERVAL_MINUTE_TO_SECOND:
    case OdbcNativeType::AI_INTERVAL_YEAR_TO_MONTH:
    case OdbcNativeType::AI_INTERVAL_DAY_TO_SECOND: {
      SQL_INTERVAL_STRUCT* buffer =
          reinterpret_cast< SQL_INTERVAL_STRUCT* >(dataPtr);
      SetIntervalBufferValue(buffer, value);

      if (resLenPtr)
        *resLenPtr = static_cast< SqlLen >(sizeof(SQL_INTERVAL_STRUCT));

      return ConversionResult::Type::AI_SUCCESS;
    }

    default:
      break;
  }

  return ConversionResult::Type::AI_UNSUPPORTED_CONVERSION;
}

std::string ApplicationDataBuffer::GetString(size_t maxLen) const {
  LOG_DEBUG_MSG("GetString is called with type " << type);
  using namespace type_traits;
  std::string res;

  switch (type) {
    case OdbcNativeType::AI_CHAR: {
      size_t paramLen = GetInputSize();

      if (!paramLen)
        break;

      res = utility::SqlCharToString(
          reinterpret_cast< const SQLCHAR* >(GetData()),
          static_cast< int32_t >(paramLen));

      LOG_DEBUG_MSG("res is " << res << ", paramLen is " << paramLen
                              << ", maxLen is " << maxLen);
      if (res.size() > maxLen)
        res.resize(maxLen);

      break;
    }

    case OdbcNativeType::AI_WCHAR: {
      size_t paramLen = GetInputSize();

      if (!paramLen)
        break;

      res = utility::SqlWcharToString(
          reinterpret_cast< const SQLWCHAR* >(GetData()),
          static_cast< int32_t >(paramLen), true);

      LOG_DEBUG_MSG("res is " << res << ", paramLen is " << paramLen
                              << ", maxLen is " << maxLen);
      if (res.size() > maxLen)
        res.resize(maxLen);

      break;
    }

    case OdbcNativeType::AI_SIGNED_TINYINT:
    case OdbcNativeType::AI_SIGNED_SHORT:
    case OdbcNativeType::AI_SIGNED_LONG:
    case OdbcNativeType::AI_SIGNED_BIGINT: {
      std::stringstream converter;

      converter << GetNum< int64_t >();

      res = converter.str();

      break;
    }

    case OdbcNativeType::AI_BIT:
    case OdbcNativeType::AI_UNSIGNED_TINYINT:
    case OdbcNativeType::AI_UNSIGNED_SHORT:
    case OdbcNativeType::AI_UNSIGNED_LONG:
    case OdbcNativeType::AI_UNSIGNED_BIGINT: {
      std::stringstream converter;

      converter << GetNum< uint64_t >();

      res = converter.str();

      break;
    }

    case OdbcNativeType::AI_FLOAT: {
      std::stringstream converter;

      converter << GetNum< float >();

      res = converter.str();

      break;
    }

    case OdbcNativeType::AI_NUMERIC:
    case OdbcNativeType::AI_DOUBLE: {
      std::stringstream converter;

      converter << GetNum< double >();

      res = converter.str();

      break;
    }

    default:
      break;
  }

  return res;
}

int8_t ApplicationDataBuffer::GetInt8() const {
  return GetNum< int8_t >();
}

int16_t ApplicationDataBuffer::GetInt16() const {
  return GetNum< int16_t >();
}

int32_t ApplicationDataBuffer::GetInt32() const {
  return GetNum< int32_t >();
}

int64_t ApplicationDataBuffer::GetInt64() const {
  return GetNum< int64_t >();
}

float ApplicationDataBuffer::GetFloat() const {
  return GetNum< float >();
}

double ApplicationDataBuffer::GetDouble() const {
  return GetNum< double >();
}

const void* ApplicationDataBuffer::GetData() const {
  return ApplyOffset(buffer, GetElementSize());
}

const SqlLen* ApplicationDataBuffer::GetResLen() const {
  return ApplyOffset(reslen, sizeof(*reslen));
}

void* ApplicationDataBuffer::GetData() {
  return ApplyOffset(buffer, GetElementSize());
}

SqlLen* ApplicationDataBuffer::GetResLen() {
  return ApplyOffset(reslen, sizeof(*reslen));
}

template < typename T >
T ApplicationDataBuffer::GetNum() const {
  LOG_DEBUG_MSG("GetNum is called with type " << type);
  using namespace type_traits;

  T res = T();

  switch (type) {
    case OdbcNativeType::AI_CHAR:
    case OdbcNativeType::AI_WCHAR: {
      SqlLen paramLen = GetInputSize();

      if (!paramLen)
        break;

      std::string str = GetString(paramLen);

      std::stringstream converter;

      converter << str;

      // Workaround for char types which are recognised as
      // symbolyc types and not numeric types.
      if (sizeof(T) == 1) {
        short tmp;

        converter >> tmp;

        res = static_cast< T >(tmp);
      } else
        converter >> res;
      break;
    }

    case OdbcNativeType::AI_SIGNED_TINYINT: {
      res =
          static_cast< T >(*reinterpret_cast< const signed char* >(GetData()));
      break;
    }

    case OdbcNativeType::AI_BIT:
    case OdbcNativeType::AI_UNSIGNED_TINYINT: {
      res = static_cast< T >(
          *reinterpret_cast< const unsigned char* >(GetData()));
      break;
    }

    case OdbcNativeType::AI_SIGNED_SHORT: {
      res =
          static_cast< T >(*reinterpret_cast< const signed short* >(GetData()));
      break;
    }

    case OdbcNativeType::AI_UNSIGNED_SHORT: {
      res = static_cast< T >(
          *reinterpret_cast< const unsigned short* >(GetData()));
      break;
    }

    case OdbcNativeType::AI_SIGNED_LONG: {
      res =
          static_cast< T >(*reinterpret_cast< const signed long* >(GetData()));
      break;
    }

    case OdbcNativeType::AI_UNSIGNED_LONG: {
      res = static_cast< T >(
          *reinterpret_cast< const unsigned long* >(GetData()));
      break;
    }

    case OdbcNativeType::AI_SIGNED_BIGINT: {
      res = static_cast< T >(*reinterpret_cast< const int64_t* >(GetData()));
      break;
    }

    case OdbcNativeType::AI_UNSIGNED_BIGINT: {
      res = static_cast< T >(*reinterpret_cast< const uint64_t* >(GetData()));
      break;
    }

    case OdbcNativeType::AI_FLOAT: {
      res = static_cast< T >(*reinterpret_cast< const float* >(GetData()));
      break;
    }

    case OdbcNativeType::AI_DOUBLE: {
      res = static_cast< T >(*reinterpret_cast< const double* >(GetData()));
      break;
    }

    case OdbcNativeType::AI_NUMERIC: {
      const SQL_NUMERIC_STRUCT* numeric =
          reinterpret_cast< const SQL_NUMERIC_STRUCT* >(GetData());

      ignite::odbc::common::Decimal dec(
          reinterpret_cast< const int8_t* >(numeric->val), SQL_MAX_NUMERIC_LEN,
          numeric->scale, numeric->sign ? 1 : -1, false);

      res = static_cast< T >(dec.ToInt64());

      break;
    }

    default:
      break;
  }

  LOG_DEBUG_MSG("res is " << res);
  return res;
}

Date ApplicationDataBuffer::GetDate() const {
  LOG_DEBUG_MSG("GetDate is called with type " << type);
  using namespace type_traits;

  tm tmTime;

  std::memset(&tmTime, 0, sizeof(tmTime));

  switch (type) {
    case OdbcNativeType::AI_TDATE: {
      const SQL_DATE_STRUCT* buffer =
          reinterpret_cast< const SQL_DATE_STRUCT* >(GetData());

      tmTime.tm_year = buffer->year - 1900;
      tmTime.tm_mon = buffer->month - 1;
      tmTime.tm_mday = buffer->day;

      break;
    }

    case OdbcNativeType::AI_TTIME: {
      const SQL_TIME_STRUCT* buffer =
          reinterpret_cast< const SQL_TIME_STRUCT* >(GetData());

      tmTime.tm_year = 70;
      tmTime.tm_mday = 1;
      tmTime.tm_hour = buffer->hour;
      tmTime.tm_min = buffer->minute;
      tmTime.tm_sec = buffer->second;

      break;
    }

    case OdbcNativeType::AI_TTIMESTAMP: {
      const SQL_TIMESTAMP_STRUCT* buffer =
          reinterpret_cast< const SQL_TIMESTAMP_STRUCT* >(GetData());

      tmTime.tm_year = buffer->year - 1900;
      tmTime.tm_mon = buffer->month - 1;
      tmTime.tm_mday = buffer->day;
      tmTime.tm_hour = buffer->hour;
      tmTime.tm_min = buffer->minute;
      tmTime.tm_sec = buffer->second;

      break;
    }

    case OdbcNativeType::AI_CHAR: {
      SqlLen paramLen = GetInputSize();

      if (!paramLen)
        break;

      std::string str = utility::SqlCharToString(
          reinterpret_cast< const SQLCHAR* >(GetData()),
          static_cast< int32_t >(paramLen));

      sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", &tmTime.tm_year, &tmTime.tm_mon,
             &tmTime.tm_mday, &tmTime.tm_hour, &tmTime.tm_min, &tmTime.tm_sec);

      tmTime.tm_year = tmTime.tm_year - 1900;
      tmTime.tm_mon = tmTime.tm_mon - 1;

      break;
    }

    case OdbcNativeType::AI_WCHAR: {
      SqlLen paramLen = GetInputSize();

      if (!paramLen)
        break;

      std::string str = utility::SqlWcharToString(
          reinterpret_cast< const SQLWCHAR* >(GetData()),
          static_cast< int32_t >(paramLen), true);

      sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", &tmTime.tm_year, &tmTime.tm_mon,
             &tmTime.tm_mday, &tmTime.tm_hour, &tmTime.tm_min, &tmTime.tm_sec);

      tmTime.tm_year = tmTime.tm_year - 1900;
      tmTime.tm_mon = tmTime.tm_mon - 1;

      break;
    }

    default:
      break;
  }

  Date retval = timestream::odbc::common::CTmToDate(tmTime);
  LOG_DEBUG_MSG("tmTime.tm_year "
                << tmTime.tm_year << ", tmTime.tm_mon " << tmTime.tm_mon
                << ", tmTime.tm_mday " << tmTime.tm_mday << ", tmTime.tm_hour "
                << tmTime.tm_hour << ", tmTime.tm_min " << tmTime.tm_min
                << ", tmTime.tm_sec " << tmTime.tm_sec << ", tmTime.tm_wday "
                << tmTime.tm_wday << ", tmTime.tm_yday " << tmTime.tm_yday
                << ", tmTime.tm_isdst " << tmTime.tm_isdst);
  return retval;
}

Timestamp ApplicationDataBuffer::GetTimestamp() const {
  LOG_DEBUG_MSG("GetTimestamp is called with type " << type);
  using namespace type_traits;

  tm tmTime;

  std::memset(&tmTime, 0, sizeof(tmTime));
  LOG_DEBUG_MSG("tmTime.tm_year "
                << tmTime.tm_year << ", tmTime.tm_mon " << tmTime.tm_mon
                << ", tmTime.tm_mday " << tmTime.tm_mday << ", tmTime.tm_hour "
                << tmTime.tm_hour << ", tmTime.tm_min " << tmTime.tm_min
                << ", tmTime.tm_sec " << tmTime.tm_sec << ", tmTime.tm_wday "
                << tmTime.tm_wday << ", tmTime.tm_yday " << tmTime.tm_yday
                << ", tmTime.tm_isdst " << tmTime.tm_isdst);

  int32_t nanos = 0;

  switch (type) {
    case OdbcNativeType::AI_TDATE: {
      const SQL_DATE_STRUCT* buffer =
          reinterpret_cast< const SQL_DATE_STRUCT* >(GetData());

      tmTime.tm_year = buffer->year - 1900;
      tmTime.tm_mon = buffer->month - 1;
      tmTime.tm_mday = buffer->day;

      break;
    }

    case OdbcNativeType::AI_TTIME: {
      const SQL_TIME_STRUCT* buffer =
          reinterpret_cast< const SQL_TIME_STRUCT* >(GetData());

      tmTime.tm_year = 70;
      tmTime.tm_mday = 1;
      tmTime.tm_hour = buffer->hour;
      tmTime.tm_min = buffer->minute;
      tmTime.tm_sec = buffer->second;

      break;
    }

    case OdbcNativeType::AI_TTIMESTAMP: {
      const SQL_TIMESTAMP_STRUCT* buffer =
          reinterpret_cast< const SQL_TIMESTAMP_STRUCT* >(GetData());

      tmTime.tm_year = buffer->year - 1900;
      tmTime.tm_mon = buffer->month - 1;
      tmTime.tm_mday = buffer->day;
      tmTime.tm_hour = buffer->hour;
      tmTime.tm_min = buffer->minute;
      tmTime.tm_sec = buffer->second;

      nanos = buffer->fraction;

      break;
    }

    case OdbcNativeType::AI_CHAR: {
      SqlLen paramLen = GetInputSize();

      if (!paramLen)
        break;

      std::string str = utility::SqlCharToString(
          reinterpret_cast< const SQLCHAR* >(GetData()),
          static_cast< int32_t >(paramLen));

      sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", &tmTime.tm_year, &tmTime.tm_mon,
             &tmTime.tm_mday, &tmTime.tm_hour, &tmTime.tm_min, &tmTime.tm_sec);

      tmTime.tm_year = tmTime.tm_year - 1900;
      tmTime.tm_mon = tmTime.tm_mon - 1;

      break;
    }

    case OdbcNativeType::AI_WCHAR: {
      SqlLen paramLen = GetInputSize();

      if (!paramLen)
        break;

      std::string str = utility::SqlWcharToString(
          reinterpret_cast< const SQLWCHAR* >(GetData()),
          static_cast< int32_t >(paramLen), true);

      sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", &tmTime.tm_year, &tmTime.tm_mon,
             &tmTime.tm_mday, &tmTime.tm_hour, &tmTime.tm_min, &tmTime.tm_sec);

      tmTime.tm_year = tmTime.tm_year - 1900;
      tmTime.tm_mon = tmTime.tm_mon - 1;

      break;
    }

    default:
      break;
  }

  return timestream::odbc::common::CTmToTimestamp(tmTime, nanos);
}

Time ApplicationDataBuffer::GetTime() const {
  LOG_DEBUG_MSG("GetTime is called with type " << type);
  using namespace type_traits;

  tm tmTime;

  std::memset(&tmTime, 0, sizeof(tmTime));

  tmTime.tm_year = 70;
  tmTime.tm_mon = 0;
  tmTime.tm_mday = 1;

  switch (type) {
    case OdbcNativeType::AI_TTIME: {
      const SQL_TIME_STRUCT* buffer =
          reinterpret_cast< const SQL_TIME_STRUCT* >(GetData());

      tmTime.tm_hour = buffer->hour;
      tmTime.tm_min = buffer->minute;
      tmTime.tm_sec = buffer->second;

      break;
    }

    case OdbcNativeType::AI_TTIMESTAMP: {
      const SQL_TIMESTAMP_STRUCT* buffer =
          reinterpret_cast< const SQL_TIMESTAMP_STRUCT* >(GetData());

      tmTime.tm_hour = buffer->hour;
      tmTime.tm_min = buffer->minute;
      tmTime.tm_sec = buffer->second;

      break;
    }

    case OdbcNativeType::AI_CHAR: {
      SqlLen paramLen = GetInputSize();

      if (!paramLen)
        break;

      std::string str = utility::SqlCharToString(
          reinterpret_cast< const SQLCHAR* >(GetData()),
          static_cast< int32_t >(paramLen));

      sscanf(str.c_str(), "%d:%d:%d", &tmTime.tm_hour, &tmTime.tm_min,
             &tmTime.tm_sec);

      break;
    }

    case OdbcNativeType::AI_WCHAR: {
      SqlLen paramLen = GetInputSize();

      if (!paramLen)
        break;

      std::string str = utility::SqlWcharToString(
          reinterpret_cast< const SQLWCHAR* >(GetData()),
          static_cast< int32_t >(paramLen), true);

      sscanf(str.c_str(), "%d:%d:%d", &tmTime.tm_hour, &tmTime.tm_min,
             &tmTime.tm_sec);

      break;
    }

    default:
      break;
  }

  Time retval = timestream::odbc::common::CTmToTime(tmTime);
  LOG_DEBUG_MSG("tmTime.tm_year "
                << tmTime.tm_year << ", tmTime.tm_mon " << tmTime.tm_mon
                << ", tmTime.tm_mday " << tmTime.tm_mday << ", tmTime.tm_hour "
                << tmTime.tm_hour << ", tmTime.tm_min " << tmTime.tm_min
                << ", tmTime.tm_sec " << tmTime.tm_sec << ", tmTime.tm_wday "
                << tmTime.tm_wday << ", tmTime.tm_yday " << tmTime.tm_yday
                << ", tmTime.tm_isdst " << tmTime.tm_isdst);
  return retval;
}

void ApplicationDataBuffer::GetDecimal(
    ignite::odbc::common::Decimal& val) const {
  LOG_DEBUG_MSG("GetDecimal is called with type " << type);
  using namespace type_traits;

  switch (type) {
    case OdbcNativeType::AI_CHAR:
    case OdbcNativeType::AI_WCHAR: {
      SqlLen paramLen = GetInputSize();

      if (!paramLen)
        break;

      std::string str = GetString(paramLen);

      std::stringstream converter;

      converter << str;

      converter >> val;

      break;
    }

    case OdbcNativeType::AI_SIGNED_TINYINT:
    case OdbcNativeType::AI_BIT:
    case OdbcNativeType::AI_SIGNED_SHORT:
    case OdbcNativeType::AI_SIGNED_LONG:
    case OdbcNativeType::AI_SIGNED_BIGINT: {
      val.AssignInt64(GetNum< int64_t >());

      break;
    }

    case OdbcNativeType::AI_UNSIGNED_TINYINT:
    case OdbcNativeType::AI_UNSIGNED_SHORT:
    case OdbcNativeType::AI_UNSIGNED_LONG:
    case OdbcNativeType::AI_UNSIGNED_BIGINT: {
      val.AssignUint64(GetNum< uint64_t >());

      break;
    }

    case OdbcNativeType::AI_FLOAT:
    case OdbcNativeType::AI_DOUBLE: {
      val.AssignDouble(GetNum< double >());

      break;
    }

    case OdbcNativeType::AI_NUMERIC: {
      const SQL_NUMERIC_STRUCT* numeric =
          reinterpret_cast< const SQL_NUMERIC_STRUCT* >(GetData());

      ignite::odbc::common::Decimal dec(
          reinterpret_cast< const int8_t* >(numeric->val), SQL_MAX_NUMERIC_LEN,
          numeric->scale, numeric->sign ? 1 : -1, false);

      val.Swap(dec);

      break;
    }

    default: {
      val.AssignInt64(0);

      break;
    }
  }
  LOG_DEBUG_MSG("val is " << val);
}

template < typename T >
T* ApplicationDataBuffer::ApplyOffset(T* ptr, size_t elemSize) const {
  LOG_DEBUG_MSG("ApplyOffset is called");
  if (!ptr)
    return ptr;

  return utility::GetPointerWithOffset(ptr,
                                       byteOffset + elemSize * elementOffset);
}

bool ApplicationDataBuffer::IsDataAtExec() const {
  LOG_DEBUG_MSG("IsDataAtExec is called");
  const SqlLen* resLenPtr = GetResLen();

  if (!resLenPtr)
    return false;

  int32_t ilen = static_cast< int32_t >(*resLenPtr);

  return ilen <= SQL_LEN_DATA_AT_EXEC_OFFSET || ilen == SQL_DATA_AT_EXEC;
}

SqlLen ApplicationDataBuffer::GetDataAtExecSize() const {
  LOG_DEBUG_MSG("GetDataAtExecSize is called with type " << type);
  using namespace type_traits;

  switch (type) {
    case OdbcNativeType::AI_WCHAR:
    case OdbcNativeType::AI_CHAR:
    case OdbcNativeType::AI_BINARY: {
      const SqlLen* resLenPtr = GetResLen();

      if (!resLenPtr)
        return 0;

      int32_t ilen = static_cast< int32_t >(*resLenPtr);

      if (ilen <= SQL_LEN_DATA_AT_EXEC_OFFSET)
        ilen = SQL_LEN_DATA_AT_EXEC(ilen);
      else
        ilen = 0;

      if (type == OdbcNativeType::AI_WCHAR)
        ilen *= 2;

      LOG_DEBUG_MSG("ilen is " << ilen);
      return ilen;
    }

    case OdbcNativeType::AI_SIGNED_SHORT:
    case OdbcNativeType::AI_UNSIGNED_SHORT:
      return static_cast< SqlLen >(sizeof(short));

    case OdbcNativeType::AI_SIGNED_LONG:
    case OdbcNativeType::AI_UNSIGNED_LONG:
      return static_cast< SqlLen >(sizeof(long));

    case OdbcNativeType::AI_FLOAT:
      return static_cast< SqlLen >(sizeof(float));

    case OdbcNativeType::AI_DOUBLE:
      return static_cast< SqlLen >(sizeof(double));

    case OdbcNativeType::AI_BIT:
    case OdbcNativeType::AI_SIGNED_TINYINT:
    case OdbcNativeType::AI_UNSIGNED_TINYINT:
      return static_cast< SqlLen >(sizeof(char));

    case OdbcNativeType::AI_SIGNED_BIGINT:
    case OdbcNativeType::AI_UNSIGNED_BIGINT:
      return static_cast< SqlLen >(sizeof(SQLBIGINT));

    case OdbcNativeType::AI_TDATE:
      return static_cast< SqlLen >(sizeof(SQL_DATE_STRUCT));

    case OdbcNativeType::AI_TTIME:
      return static_cast< SqlLen >(sizeof(SQL_TIME_STRUCT));

    case OdbcNativeType::AI_TTIMESTAMP:
      return static_cast< SqlLen >(sizeof(SQL_TIMESTAMP_STRUCT));

    case OdbcNativeType::AI_NUMERIC:
      return static_cast< SqlLen >(sizeof(SQL_NUMERIC_STRUCT));

    case OdbcNativeType::AI_DEFAULT:
    case OdbcNativeType::AI_UNSUPPORTED:
    default:
      break;
  }

  return 0;
}

SqlLen ApplicationDataBuffer::GetElementSize() const {
  LOG_DEBUG_MSG("GetElementSize is called with type " << type);
  using namespace type_traits;

  switch (type) {
    case OdbcNativeType::AI_WCHAR:
    case OdbcNativeType::AI_CHAR:
    case OdbcNativeType::AI_BINARY:
      return buflen;

    case OdbcNativeType::AI_SIGNED_SHORT:
      return static_cast< SqlLen >(sizeof(SQLSMALLINT));

    case OdbcNativeType::AI_UNSIGNED_SHORT:
      return static_cast< SqlLen >(sizeof(SQLUSMALLINT));

    case OdbcNativeType::AI_SIGNED_LONG:
      return static_cast< SqlLen >(sizeof(SQLUINTEGER));

    case OdbcNativeType::AI_UNSIGNED_LONG:
      return static_cast< SqlLen >(sizeof(SQLINTEGER));

    case OdbcNativeType::AI_FLOAT:
      return static_cast< SqlLen >(sizeof(SQLREAL));

    case OdbcNativeType::AI_DOUBLE:
      return static_cast< SqlLen >(sizeof(SQLDOUBLE));

    case OdbcNativeType::AI_SIGNED_TINYINT:
      return static_cast< SqlLen >(sizeof(SQLSCHAR));

    case OdbcNativeType::AI_BIT:
    case OdbcNativeType::AI_UNSIGNED_TINYINT:
      return static_cast< SqlLen >(sizeof(SQLCHAR));

    case OdbcNativeType::AI_SIGNED_BIGINT:
      return static_cast< SqlLen >(sizeof(SQLBIGINT));

    case OdbcNativeType::AI_UNSIGNED_BIGINT:
      return static_cast< SqlLen >(sizeof(SQLUBIGINT));

    case OdbcNativeType::AI_TDATE:
      return static_cast< SqlLen >(sizeof(SQL_DATE_STRUCT));

    case OdbcNativeType::AI_TTIME:
      return static_cast< SqlLen >(sizeof(SQL_TIME_STRUCT));

    case OdbcNativeType::AI_TTIMESTAMP:
      return static_cast< SqlLen >(sizeof(SQL_TIMESTAMP_STRUCT));

    case OdbcNativeType::AI_NUMERIC:
      return static_cast< SqlLen >(sizeof(SQL_NUMERIC_STRUCT));

    case OdbcNativeType::AI_DEFAULT:
    case OdbcNativeType::AI_UNSUPPORTED:
    default:
      break;
  }

  return 0;
}

SqlLen ApplicationDataBuffer::GetInputSize() const {
  LOG_DEBUG_MSG("GetInputSize is called");
  if (!IsDataAtExec()) {
    const SqlLen* len = GetResLen();

    return len ? *len : SQL_NTS;
  }

  return GetDataAtExecSize();
}

void ApplicationDataBuffer::SetIntervalType(SQL_INTERVAL_STRUCT* buffer) {
  switch (type) {
    case OdbcNativeType::AI_INTERVAL_YEAR: {
      buffer->interval_type = SQL_IS_YEAR;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_MONTH: {
      buffer->interval_type = SQL_IS_MONTH;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY: {
      buffer->interval_type = SQL_IS_DAY;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_HOUR: {
      buffer->interval_type = SQL_IS_HOUR;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_MINUTE: {
      buffer->interval_type = SQL_IS_MINUTE;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_SECOND: {
      buffer->interval_type = SQL_IS_SECOND;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY_TO_HOUR: {
      buffer->interval_type = SQL_IS_DAY_TO_HOUR;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY_TO_MINUTE: {
      buffer->interval_type = SQL_IS_DAY_TO_MINUTE;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_HOUR_TO_MINUTE: {
      buffer->interval_type = SQL_IS_HOUR_TO_MINUTE;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_HOUR_TO_SECOND: {
      buffer->interval_type = SQL_IS_HOUR_TO_SECOND;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_MINUTE_TO_SECOND: {
      buffer->interval_type = SQL_IS_MINUTE_TO_SECOND;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_YEAR_TO_MONTH: {
      buffer->interval_type = SQL_IS_YEAR_TO_MONTH;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY_TO_SECOND: {
      buffer->interval_type = SQL_IS_DAY_TO_SECOND;
      break;
    }
    default:
      LOG_ERROR_MSG("Unsupported type " << type);
      break;
  }
}

void ApplicationDataBuffer::SetIntervalBufferValue(
    SQL_INTERVAL_STRUCT* buffer, const IntervalYearMonth& value) {
  SetIntervalType(buffer);
  if (value.GetYear() < 0 || (value.GetYear() == 0 && value.GetMonth() < 0)) {
    buffer->interval_sign = SQL_FALSE;
  } else {
    buffer->interval_sign = SQL_TRUE;
  }

  switch (type) {
    case OdbcNativeType::AI_INTERVAL_YEAR: {
      buffer->intval.year_month.year = std::abs(value.GetYear());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_MONTH: {
      buffer->intval.year_month.month = std::abs(value.GetMonth());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_YEAR_TO_MONTH: {
      buffer->intval.year_month.year = std::abs(value.GetYear());
      buffer->intval.year_month.month = std::abs(value.GetMonth());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY: {
      buffer->intval.day_second.day = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_HOUR: {
      buffer->intval.day_second.hour = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_MINUTE: {
      buffer->intval.day_second.minute = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_SECOND: {
      buffer->intval.day_second.second = 0;
      buffer->intval.day_second.fraction = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY_TO_HOUR: {
      buffer->intval.day_second.day = 0;
      buffer->intval.day_second.hour = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY_TO_MINUTE: {
      buffer->intval.day_second.day = 0;
      buffer->intval.day_second.hour = 0;
      buffer->intval.day_second.minute = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_HOUR_TO_MINUTE: {
      buffer->intval.day_second.hour = 0;
      buffer->intval.day_second.minute = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_HOUR_TO_SECOND: {
      buffer->intval.day_second.hour = 0;
      buffer->intval.day_second.minute = 0;
      buffer->intval.day_second.second = 0;
      buffer->intval.day_second.fraction = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_MINUTE_TO_SECOND: {
      buffer->intval.day_second.minute = 0;
      buffer->intval.day_second.second = 0;
      buffer->intval.day_second.fraction = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY_TO_SECOND: {
      buffer->intval.day_second.day = 0;
      buffer->intval.day_second.hour = 0;
      buffer->intval.day_second.minute = 0;
      buffer->intval.day_second.second = 0;
      buffer->intval.day_second.fraction = 0;
      break;
    }
    default: {
      LOG_ERROR_MSG("Unsupported type " << type);
      break;
    }
  }
}

void ApplicationDataBuffer::SetIntervalBufferValue(
    SQL_INTERVAL_STRUCT* buffer, const IntervalDaySecond& value) {
  SetIntervalType(buffer);
  if (value.GetDay() < 0 || (value.GetDay() == 0 && value.GetHour() < 0)
      || (value.GetDay() == 0 && value.GetHour() == 0 && value.GetMinute() < 0)
      || (value.GetDay() == 0 && value.GetHour() == 0 && value.GetMinute() == 0
          && value.GetSecond() < 0)
      || (value.GetDay() == 0 && value.GetHour() == 0 && value.GetMinute() == 0
          && value.GetSecond() == 0 && value.GetFraction() < 0)) {
    buffer->interval_sign = SQL_FALSE;
  } else {
    buffer->interval_sign = SQL_TRUE;
  }

  switch (type) {
    case OdbcNativeType::AI_INTERVAL_YEAR: {
      buffer->intval.year_month.year = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_MONTH: {
      buffer->intval.year_month.month = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_YEAR_TO_MONTH: {
      buffer->intval.year_month.year = 0;
      buffer->intval.year_month.month = 0;
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY: {
      buffer->intval.day_second.day = std::abs(value.GetDay());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_HOUR: {
      buffer->intval.day_second.hour = std::abs(value.GetHour());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_MINUTE: {
      buffer->intval.day_second.minute = std::abs(value.GetMinute());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_SECOND: {
      buffer->intval.day_second.second = std::abs(value.GetSecond());
      buffer->intval.day_second.fraction = std::abs(value.GetFraction());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY_TO_HOUR: {
      buffer->intval.day_second.day = std::abs(value.GetDay());
      buffer->intval.day_second.hour = std::abs(value.GetHour());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY_TO_MINUTE: {
      buffer->intval.day_second.day = std::abs(value.GetDay());
      buffer->intval.day_second.hour = std::abs(value.GetHour());
      buffer->intval.day_second.minute = std::abs(value.GetMinute());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_HOUR_TO_MINUTE: {
      buffer->intval.day_second.hour = std::abs(value.GetHour());
      buffer->intval.day_second.minute = std::abs(value.GetMinute());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_HOUR_TO_SECOND: {
      buffer->intval.day_second.hour = std::abs(value.GetHour());
      buffer->intval.day_second.minute = std::abs(value.GetMinute());
      buffer->intval.day_second.second = std::abs(value.GetSecond());
      buffer->intval.day_second.fraction = std::abs(value.GetFraction());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_MINUTE_TO_SECOND: {
      buffer->intval.day_second.minute = std::abs(value.GetMinute());
      buffer->intval.day_second.second = std::abs(value.GetSecond());
      buffer->intval.day_second.fraction = std::abs(value.GetFraction());
      break;
    }
    case OdbcNativeType::AI_INTERVAL_DAY_TO_SECOND: {
      buffer->intval.day_second.day = std::abs(value.GetDay());
      buffer->intval.day_second.hour = std::abs(value.GetHour());
      buffer->intval.day_second.minute = std::abs(value.GetMinute());
      buffer->intval.day_second.second = std::abs(value.GetSecond());
      buffer->intval.day_second.fraction = std::abs(value.GetFraction());
      break;
    }
    default: {
      LOG_ERROR_MSG("Unsupported type " << type);
      break;
    }
  }
}

}  // namespace app
}  // namespace odbc
}  // namespace timestream
