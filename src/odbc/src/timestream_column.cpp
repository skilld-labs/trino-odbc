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

#include <chrono>
#include <ctime>
#include <time.h>
#include "timestream/odbc/timestream_column.h"
#include "timestream/odbc/utility.h"
#include <aws/timestream-query/model/ColumnInfo.h>
#include <aws/timestream-query/model/TimeSeriesDataPoint.h>

using Aws::TimestreamQuery::Model::ScalarType;
using Aws::TimestreamQuery::Model::TimeSeriesDataPoint;
using timestream::odbc::type_traits::OdbcNativeType;

namespace timestream {
namespace odbc {
#define BUFFER_SIZE 1024

TimestreamColumn::TimestreamColumn(
                                   uint32_t columnIdx,
                                   const meta::ColumnMeta& columnMeta)
    :
      columnIdx_(columnIdx), 
      columnMeta_(columnMeta) {
}

ConversionResult::Type TimestreamColumn::ReadToBuffer(const Datum& datum, ApplicationDataBuffer& dataBuf) const {
  LOG_DEBUG_MSG("ReadToBuffer is called");
  const boost::optional< Aws::TimestreamQuery::Model::ColumnInfo >& columnInfo =
      columnMeta_.GetColumnInfo();

  if (!columnInfo || !columnInfo->TypeHasBeenSet()) {
    LOG_ERROR_MSG("ColumnInfo is not found or type is not set");
    return ConversionResult::Type::AI_FAILURE;
  }

  ConversionResult::Type retval = ParseDatum(datum, dataBuf);

  return retval;
}

ConversionResult::Type TimestreamColumn::ParseDatum(
    const Datum& datum, ApplicationDataBuffer& dataBuf) const {
  LOG_DEBUG_MSG("ParseDatum is called");

  ConversionResult::Type retval = ConversionResult::Type::AI_FAILURE;
  if (datum.ScalarValueHasBeenSet()) {
    retval = ParseScalarType(datum, dataBuf);
  } else if (datum.TimeSeriesValueHasBeenSet()) {
    retval = ParseTimeSeriesType(datum, dataBuf);
  } else if (datum.ArrayValueHasBeenSet()) {
    retval = ParseArrayType(datum, dataBuf);
  } else if (datum.RowValueHasBeenSet()) {
    retval = ParseRowType(datum, dataBuf);
  } else if (datum.NullValueHasBeenSet()) {
    dataBuf.PutString("-");
    retval = ConversionResult::Type::AI_SUCCESS;
  } else {
    LOG_ERROR_MSG("Unsupported data type");
  }

  return retval;
}

ConversionResult::Type TimestreamColumn::ParseScalarType(
    const Aws::TimestreamQuery::Model::Datum& datum,
    ApplicationDataBuffer& dataBuf) const {
  LOG_DEBUG_MSG("ParseScalarType is called");

  Aws::String value = datum.GetScalarValue();
  LOG_DEBUG_MSG("value is " << value << ", scalar type is "
                            << static_cast< int >(columnMeta_.GetScalarType()));

  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;

  switch (columnMeta_.GetScalarType()) {
    case ScalarType::VARCHAR:
      convRes = dataBuf.PutString(value);
      break;
    case ScalarType::DOUBLE:
      // There could be a precision problem for stod as double can not be
      // represented in binary form using finite precision. For example for
      // double value 35.2 in string, std::stod("35.2") could
      // return 35.200000000000003 on Windows. These rounding errors are a
      // common issue in floating-point arithmetic and can not be avoided.
      convRes = dataBuf.PutDouble(std::stod(value));
      break;
    case ScalarType::BOOLEAN:
      convRes = dataBuf.PutInt8(value == "true" ? 1 : 0);
      break;
    case ScalarType::INTEGER:
      convRes = dataBuf.PutInt32(utility::StringToInt(value));
      break;
    case ScalarType::BIGINT:
      convRes = dataBuf.PutInt64(utility::StringToLong(value));
      break;
    case ScalarType::NOT_SET:
    case ScalarType::UNKNOWN:
      convRes = dataBuf.PutNull();
      break;
    case ScalarType::TIMESTAMP: {
      tm tmTime;
      memset(&tmTime, 0, sizeof(tm));
      int32_t fractionNs;
      std::sscanf(value.c_str(), "%4d-%2d-%2d %2d:%2d:%2d.%9d", &tmTime.tm_year,
                  &tmTime.tm_mon, &tmTime.tm_mday, &tmTime.tm_hour,
                  &tmTime.tm_min, &tmTime.tm_sec, &fractionNs);
      tmTime.tm_year -= 1900;
      tmTime.tm_mon--;
#ifdef _WIN32
      int64_t seconds = _mkgmtime(&tmTime);
#else
      int64_t seconds = timegm(&tmTime);
#endif
      LOG_DEBUG_MSG("timestamp is " << tmTime.tm_year << " " << tmTime.tm_mon
                                    << " " << tmTime.tm_mday << " "
                                    << tmTime.tm_hour << ":" << tmTime.tm_min
                                    << ":" << tmTime.tm_sec << "."
                                    << fractionNs);

      LOG_DEBUG_MSG("seconds is " << seconds);
      convRes = dataBuf.PutTimestamp(Timestamp(seconds, fractionNs));
      break;
    }
    case ScalarType::DATE: {
      tm tmTime;
      memset(&tmTime, 0, sizeof(tm));
      int32_t fractionNs;
      std::sscanf(value.c_str(), "%4d-%2d-%2d", &tmTime.tm_year, &tmTime.tm_mon,
                  &tmTime.tm_mday);
      tmTime.tm_year -= 1900;
      tmTime.tm_mon--;
#ifdef _WIN32
      int64_t milliSecond = _mkgmtime(&tmTime) * 1000;
#else
      int64_t milliSecond = timegm(&tmTime);
      milliSecond *= 1000;
#endif
      convRes = dataBuf.PutDate(Date(milliSecond));
      break;
    }
    case ScalarType::TIME: {
      uint32_t hour;
      uint32_t minute;
      uint32_t second;
      int32_t fractionNs;
      std::sscanf(value.c_str(), "%2d:%2d:%2d.%9d", &hour, &minute, &second,
                  &fractionNs);
      int32_t secondValue = (hour * 60 + minute) * 60 + second;
      convRes = dataBuf.PutTime(Time(secondValue, fractionNs));
      break;
    }
    case ScalarType::INTERVAL_YEAR_TO_MONTH: {
      int32_t year, month;
      std::sscanf(value.c_str(), "%d-%d", &year, &month);
      convRes = dataBuf.PutInterval(IntervalYearMonth(year, month));
      break;
    }
    case ScalarType::INTERVAL_DAY_TO_SECOND: {
      int32_t day, hour, minute, second, fraction;
      std::sscanf(value.c_str(), "%d %2d:%2d:%2d.%9d", &day, &hour, &minute,
                  &second, &fraction);
      convRes = dataBuf.PutInterval(
          IntervalDaySecond(day, hour, minute, second, fraction));
      break;
    }
    default:
      return ConversionResult::Type::AI_UNSUPPORTED_CONVERSION;
  }

  LOG_DEBUG_MSG("convRes is " << static_cast< int >(convRes));
  return convRes;
}

ConversionResult::Type TimestreamColumn::ParseTimeSeriesType(
    const Datum& datum, ApplicationDataBuffer& dataBuf) const {
  LOG_DEBUG_MSG("ParseTimeSeriesType is called");

  const Aws::Vector< TimeSeriesDataPoint >& valueVec =
      datum.GetTimeSeriesValue();

  std::string result = "[";
  for (const auto& itr : valueVec) {
    result += "{time: ";
    if (itr.TimeHasBeenSet()) {
      result += itr.GetTime();
    }
    result += ", value: ";

    if (itr.ValueHasBeenSet()) {
      char buf[BUFFER_SIZE]{};
      SqlLen resLen;
      ApplicationDataBuffer tmpBuf(OdbcNativeType::Type::AI_CHAR,
                                   static_cast< void* >(buf), BUFFER_SIZE,
                                   &resLen);
      ParseDatum(itr.GetValue(), tmpBuf);
      result += buf;
    }

    result += "},";
  }
  if (!valueVec.empty()) {
    result.pop_back();
  }
  result += "]";

  ConversionResult::Type convRes = dataBuf.PutString(result);

  LOG_DEBUG_MSG("convRes is " << static_cast< int >(convRes));
  return convRes;
}

ConversionResult::Type TimestreamColumn::ParseArrayType(
    const Datum& datum, ApplicationDataBuffer& dataBuf) const {
  LOG_DEBUG_MSG("ParseArrayType is called");

  const Aws::Vector< Datum >& valueVec = datum.GetArrayValue();

  std::string result("");
  if (valueVec.empty()) {
    result = "-";
  } else {
    result = "[";
    for (const auto& itr : valueVec) {
      char buf[BUFFER_SIZE]{};
      SqlLen resLen;
      ApplicationDataBuffer tmpBuf(OdbcNativeType::Type::AI_CHAR,
                                   static_cast< void* >(buf), BUFFER_SIZE,
                                   &resLen);
      ParseDatum(itr, tmpBuf);
      result += buf;

      result += ",";
    }
    result.pop_back();
    result += "]";
  }

  ConversionResult::Type convRes = dataBuf.PutString(result);

  LOG_DEBUG_MSG("convRes is " << static_cast< int >(convRes));
  return convRes;
}

ConversionResult::Type TimestreamColumn::ParseRowType(
    const Datum& datum, ApplicationDataBuffer& dataBuf) const {
  LOG_DEBUG_MSG("ParseRowType is called");

  const Row& row = datum.GetRowValue();

  if (!row.DataHasBeenSet()) {
    LOG_DEBUG_MSG("No data is set for the row");
    return ConversionResult::Type::AI_NO_DATA;
  }

  const Aws::Vector< Datum >& valueVec = row.GetData();
  std::string result = "(";
  for (const auto& itr : valueVec) {
    char buf[BUFFER_SIZE]{};
    SqlLen resLen;
    ApplicationDataBuffer tmpBuf(OdbcNativeType::Type::AI_CHAR,
                                 static_cast< void* >(buf), BUFFER_SIZE,
                                 &resLen);
    ParseDatum(itr, tmpBuf);
    result += buf;

    result += ",";
  }
  if (!valueVec.empty()) {
    result.pop_back();
  }
  result += ")";

  ConversionResult::Type convRes = dataBuf.PutString(result);

  LOG_DEBUG_MSG("convRes is " << static_cast< int >(convRes));
  return convRes;
}
}  // namespace odbc
}  // namespace timestream
