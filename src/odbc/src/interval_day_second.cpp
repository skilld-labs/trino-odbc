/*
 * Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

#include "timestream/odbc/interval_day_second.h"

namespace timestream {
namespace odbc {
IntervalDaySecond::IntervalDaySecond(const IntervalDaySecond& another)
    : day_(another.day_),
      hour_(another.hour_),
      minute_(another.minute_),
      second_(another.second_),
      fractionNs_(another.fractionNs_) {
  // No-op.
}

IntervalDaySecond::IntervalDaySecond(int32_t day, int32_t hour, int32_t min,
                                     int32_t sec, int32_t ns)
    : day_(day), hour_(hour), minute_(min), second_(sec), fractionNs_(ns) {
  // No-op.
}

int64_t IntervalDaySecond::InSeconds() const {
  int64_t result =
      ((static_cast< int64_t >(day_) * 24 + hour_) * 60 + minute_) * 60
      + second_;
  return result;
}

IntervalDaySecond& IntervalDaySecond::operator=(
    const IntervalDaySecond& another) {
  day_ = another.day_;
  hour_ = another.hour_;
  minute_ = another.minute_;
  second_ = another.second_;
  fractionNs_ = another.fractionNs_;

  return *this;
}

bool operator==(const IntervalDaySecond& val1, const IntervalDaySecond& val2) {
  return (val1.GetFraction() == val2.GetFraction())
         && (val1.InSeconds() == val2.InSeconds());
}

bool operator!=(const IntervalDaySecond& val1, const IntervalDaySecond& val2) {
  return !(val1 == val2);
}

bool operator<(const IntervalDaySecond& val1, const IntervalDaySecond& val2) {
  int64_t sec1 = val1.InSeconds();
  int64_t sec2 = val2.InSeconds();
  return (sec1 < sec2)
         || ((sec1 == sec2) && (val1.GetFraction() < val2.GetFraction()));
}

bool operator<=(const IntervalDaySecond& val1, const IntervalDaySecond& val2) {
  return (val1 < val2) || (val1 == val2);
}

bool operator>(const IntervalDaySecond& val1, const IntervalDaySecond& val2) {
  return !(val1 <= val2);
}

bool operator>=(const IntervalDaySecond& val1, const IntervalDaySecond& val2) {
  return !(val1 < val2);
}
}  // namespace odbc
}  // namespace timestream
