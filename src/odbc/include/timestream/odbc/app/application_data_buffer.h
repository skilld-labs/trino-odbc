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

#ifndef _TIMESTREAM_ODBC_APP_APPLICATION_DATA_BUFFER
#define _TIMESTREAM_ODBC_APP_APPLICATION_DATA_BUFFER

#include <ignite/common/include/date.h>
#include <ignite/common/include/common/decimal.h>
#include <timestream/odbc/time.h>
#include <timestream/odbc/timestamp.h>
#include <stdint.h>
#include <boost/optional.hpp>

#include <map>

#include "timestream/odbc/common_types.h"
#include "timestream/odbc/type_traits.h"
#include "timestream/odbc/interval_year_month.h"
#include "timestream/odbc/interval_day_second.h"

using ignite::odbc::Date;
using ignite::odbc::common::Decimal;

namespace timestream {
namespace odbc {
namespace app {
/**
 * Conversion result
 */
struct ConversionResult {
  enum class Type {
    /** Conversion successful. No data lost. */
    AI_SUCCESS,

    /** Conversion successful, but fractional truncation occurred. */
    AI_FRACTIONAL_TRUNCATED,

    /** Conversion successful, but right-side variable length data truncation
       occurred. */
    AI_VARLEN_DATA_TRUNCATED,

    /** Conversion is not supported. */
    AI_UNSUPPORTED_CONVERSION,

    /** Indicator buffer needed to complete the operation but it is NULL. */
    AI_INDICATOR_NEEDED,

    /** No data found. */
    AI_NO_DATA,

    /** General operation failure. */
    AI_FAILURE
  };
};

/**
 * User application data buffer.
 */
class ApplicationDataBuffer {
 public:
  /**
   * Default constructor.
   */
  ApplicationDataBuffer();

  /**
   * Constructor.
   *
   * @param type Underlying data type.
   * @param buffer Data buffer pointer.
   * @param buflen Data buffer length.
   * @param reslen Resulting data length.
   */
  ApplicationDataBuffer(type_traits::OdbcNativeType::Type type, void* buffer,
                        SqlLen buflen, SqlLen* reslen);

  /**
   * Copy constructor.
   *
   * @param other Other instance.
   */
  ApplicationDataBuffer(const ApplicationDataBuffer& other);

  /**
   * Destructor.
   */
  ~ApplicationDataBuffer();

  /**
   * Copy assigment operator.
   *
   * @param other Other instance.
   * @return This.
   */
  ApplicationDataBuffer& operator=(const ApplicationDataBuffer& other);

  /**
   * Set offset in bytes for all bound pointers.
   *
   * @param offset Offset.
   */
  void SetByteOffset(int offset) {
    this->byteOffset = offset;
  }

  /**
   * Set offset in elements for all bound pointers.
   *
   * @param
   */
  void SetElementOffset(SqlUlen idx) {
    this->elementOffset = idx;
  }

  /**
   * Put in buffer value of type optional int8_t.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutInt8(boost::optional< int8_t > value);

  /**
   * Put in buffer value of type int8_t.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutInt8(int8_t value);

  /**
   * Put in buffer value of type optiona int16_t.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutInt16(boost::optional< int16_t > value);

  /**
   * Put in buffer value of type int16_t.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutInt16(int16_t value);

  /**
   * Put in buffer value of type optiona int32_t.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutInt32(boost::optional< int32_t > value);

  /**
   * Put in buffer value of type int32_t.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutInt32(int32_t value);

  /**
   * Put in buffer value of type optional int64_t.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutInt64(boost::optional< int64_t > value);

  /**
   * Put in buffer value of type int64_t.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutInt64(int64_t value);

  /**
   * Put in buffer value of type optional float.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutFloat(boost::optional< float > value);

  /**
   * Put in buffer value of type float.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutFloat(float value);

  /**
   * Put in buffer value of type optional double.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutDouble(boost::optional< double > value);

  /**
   * Put in buffer value of type double.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutDouble(double value);

  /**
   * Put in buffer value of type optional string.
   *
   * @param optional value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutString(const boost::optional< std::string >& value);

  /**
   * Put in buffer value of type string.
   *
   * @param value Value.
   * @return Conversion result.
   */
  ConversionResult::Type PutString(const std::string& value);

  /**
   * Put in buffer value of type string.
   *
   * @param value Value.
   * @param written Number of written characters.
   * @return Conversion result.
   */
  ConversionResult::Type PutString(const std::string& value, int32_t& written);

  /**
   * Put NULL.
   * @return Conversion result.
   */
  ConversionResult::Type PutNull();

  /**
   * Put optional decimal value to buffer.
   *
   * @param value Value to put.
   * @return Conversion result.
   */
  ConversionResult::Type PutDecimal(const boost::optional< Decimal >& value);

  /**
   * Put decimal value to buffer.
   *
   * @param value Value to put.
   * @return Conversion result.
   */
  ConversionResult::Type PutDecimal(const Decimal& value);

  /**
   * Put optional date to buffer.
   *
   * @param value Value to put.
   * @return Conversion result.
   */
  ConversionResult::Type PutDate(const boost::optional< Date >& value);

  /**
   * Put date to buffer.
   *
   * @param value Value to put.
   * @return Conversion result.
   */
  ConversionResult::Type PutDate(const Date& value);

  /**
   * Put optional timestamp to buffer.
   *
   * @param value Value to put.
   * @return Conversion result.
   */
  ConversionResult::Type PutTimestamp(
      const boost::optional< Timestamp >& value);

  /**
   * Put timestamp to buffer.
   *
   * @param value Value to put.
   * @return Conversion result.
   */
  ConversionResult::Type PutTimestamp(const Timestamp& value);

  /**
   * Put optional time to buffer.
   *
   * @param value Value to put.
   * @return Conversion result.
   */
  ConversionResult::Type PutTime(const boost::optional< Time >& value);

  /**
   * Put time to buffer.
   *
   * @param value Value to put.
   * @return Conversion result.
   */
  ConversionResult::Type PutTime(const Time& value);

  /**
   * Put interval year to month to buffer.
   *
   * @param value Value to put.
   * @return Conversion result.
   */
  ConversionResult::Type PutInterval(const IntervalYearMonth& value);

  /**
   * Put interval day to second to buffer.
   *
   * @param value Value to put.
   * @return Conversion result.
   */
  ConversionResult::Type PutInterval(const IntervalDaySecond& value);
  /**
   * Get string.
   *
   * @return String value of buffer.
   */
  std::string GetString(size_t maxLen) const;

  /**
   * Get value of type int8_t.
   *
   * @return Integer value of type int8_t.
   */
  int8_t GetInt8() const;

  /**
   * Get value of type int16_t.
   *
   * @return Integer value of type int16_t.
   */
  int16_t GetInt16() const;

  /**
   * Get value of type int32_t.
   *
   * @return Integer value of type int32_t.
   */
  int32_t GetInt32() const;

  /**
   * Get value of type int64_t.
   *
   * @return Integer value of type int64_t.
   */
  int64_t GetInt64() const;

  /**
   * Get value of type float.
   *
   * @return Integer value of type float.
   */
  float GetFloat() const;

  /**
   * Get value of type double.
   *
   * @return Value of type double.
   */
  double GetDouble() const;

  /**
   * Get value of type Date.
   *
   * @return Value of type Date.
   */
  Date GetDate() const;

  /**
   * Get value of type Timestamp.
   *
   * @return Value of type Timestamp.
   */
  Timestamp GetTimestamp() const;

  /**
   * Get value of type Time.
   *
   * @return Value of type Timestamp.
   */
  Time GetTime() const;

  /**
   * Get value of type Decimal.
   *
   * @param val Result is placed here.
   */
  void GetDecimal(Decimal& val) const;

  /**
   * Get raw data.
   *
   * @return Buffer data.
   */
  const void* GetData() const;

  /**
   * Get result data length.
   *
   * @return Data length pointer.
   */
  const SqlLen* GetResLen() const;

  /**
   * Get raw data.
   *
   * @return Buffer data.
   */
  void* GetData();

  /**
   * Get result data length.
   *
   * @return Data length pointer.
   */
  SqlLen* GetResLen();

  /**
   * Get buffer size in bytes.
   *
   * @return Buffer size.
   */
  SqlLen GetSize() const {
    return buflen;
  }

  /**
   * Check if the data is going to be provided at execution.
   *
   * @return True if the data is going to be provided
   *     at execution.
   */
  bool IsDataAtExec() const;

  /**
   * Get size of the data that is going to be provided at
   * execution.
   *
   * @return Size of the data that is going to be provided
   *     at execution.
   */
  SqlLen GetDataAtExecSize() const;

  /**
   * Get single element size.
   *
   * @return Size of the single element.
   */
  SqlLen GetElementSize() const;

  /**
   * Get size of the input buffer.
   *
   * @return Input buffer size, or zero if the data is going
   *     to be provided at execution.
   */
  SqlLen GetInputSize() const;

  /**
   * Get buffer type.
   *
   * @return Buffer type.
   */
  type_traits::OdbcNativeType::Type GetType() const {
    return type;
  }

 private:
  /**
   * Put value of numeric type in the buffer.
   *
   * @param value Numeric value to put.
   * @return Conversion result.
   */
  template < typename T >
  ConversionResult::Type PutNum(T value);

  /**
   * Put numeric value to numeric buffer.
   *
   * @param value Numeric value.
   * @return Conversion result.
   */
  template < typename Tbuf, typename Tin >
  ConversionResult::Type PutNumToNumBuffer(Tin value);

  /**
   * Put value to string buffer.
   *
   * @param value Value that can be converted to string.
   * @return Conversion result.
   */
  template < typename CharT, typename Tin >
  ConversionResult::Type PutValToStrBuffer(const Tin& value);

  /**
   * Put value to string buffer.
   * Specialisation for int8_t.
   * @param value Value that can be converted to string.
   * @return Conversion result.
   */
  template < typename CharT >
  ConversionResult::Type PutValToStrBuffer(const int8_t& value);

  /**
   * Put string to string buffer.
   *
   * @param value String value.
   * @param written Number of characters written.
   * @return Conversion result.
   */
  template < typename OutCharT, typename InCharT >
  ConversionResult::Type PutStrToStrBuffer(
      const std::basic_string< InCharT >& value, int32_t& written);

  /**
   * Put raw data to any buffer.
   *
   * @param data Data pointer.
   * @param len Data length.
   * @param written Number of characters written.
   * @return Conversion result.
   */
  ConversionResult::Type PutRawDataToBuffer(const void* data, size_t len,
                                            int32_t& written);

  /**
   * Get int of type T.
   *
   * @return Integer value of specified type.
   */
  template < typename T >
  T GetNum() const;

  /**
   * Apply buffer offset to pointer.
   * Adds offset to pointer if offset pointer is not null.
   * @param ptr Pointer.
   * @param elemSize Element size.
   * @return Pointer with applied offset.
   */
  template < typename T >
  T* ApplyOffset(T* ptr, size_t elemSize) const;

  /**
   * Get Timestamp in string format
   * @param tmTime The timestamp data without nanosecond
   * @param fraction The nanosecond
   * @param pattern The result string format pattern
   * @return timestamp in string format "YYYY-MM--DD hh:mm:ss.xxxxxxxxx"
   */
  std::string GetTimestampString(tm& tmTime, int32_t fraction,
                                 const char* pattern);

  /**
   * Set the interval buffer type.
   *
   * @param buffer The interval buffer for output
   */
  void SetIntervalType(SQL_INTERVAL_STRUCT* buffer);

  /**
   * Set the interval buffer interval value.
   *
   * @param buffer The interval buffer for output
   * @param value The value retrieved from Timestream
   */
  void SetIntervalBufferValue(SQL_INTERVAL_STRUCT* buffer,
                              const IntervalYearMonth& value);

  /**
   * Set the interval buffer interval value.
   *
   * @param buffer The interval buffer for output
   * @param value The value retrieved from Timestream
   */
  void SetIntervalBufferValue(SQL_INTERVAL_STRUCT* buffer,
                              const IntervalDaySecond& value);

  /** Underlying data type. */
  type_traits::OdbcNativeType::Type type;

  /** Buffer pointer. */
  void* buffer;

  /** Buffer length. */
  SqlLen buflen;

  /** Result length. */
  SqlLen* reslen;

  /** Current byte offset */
  int byteOffset;

  /** Current element offset. */
  SqlUlen elementOffset;
};

/** Column binging map type alias. */
typedef std::map< uint16_t, ApplicationDataBuffer > ColumnBindingMap;
}  // namespace app
}  // namespace odbc
}  // namespace timestream

#endif  //_TIMESTREAM_ODBC_APP_APPLICATION_DATA_BUFFER
