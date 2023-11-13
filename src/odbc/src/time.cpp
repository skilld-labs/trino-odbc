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

#include "trino/odbc/time.h"

namespace trino {
namespace odbc {
Time::Time() : seconds(0), fractionNs(0) {
  // No-op.
}

Time::Time(const Time& another)
    : seconds(another.seconds), fractionNs(another.fractionNs) {
  // No-op.
}

Time::Time(int64_t ms) : seconds(ms / 1000), fractionNs((ms % 1000) * 100000) {
  // No-op.
}

Time::Time(int32_t sec, int32_t ns) : seconds(sec), fractionNs(ns) {
  // No-op.
}

Time& Time::operator=(const Time& another) {
  seconds = another.seconds;
  fractionNs = another.fractionNs;

  return *this;
}

int64_t Time::GetMilliseconds() const {
  return seconds * 1000 + fractionNs / 1000;
}

int32_t Time::GetSeconds() const {
  return seconds;
}

int32_t Time::GetSecondFraction() const {
  return fractionNs;
}

bool operator==(const Time& val1, const Time& val2) {
  return val1.seconds == val2.seconds && val1.fractionNs == val2.fractionNs;
}

bool operator!=(const Time& val1, const Time& val2) {
  return val1.seconds != val2.seconds || val1.fractionNs != val2.fractionNs;
}

bool operator<(const Time& val1, const Time& val2) {
  return val1.seconds < val2.seconds
         || (val1.seconds == val2.seconds && val1.fractionNs < val2.fractionNs);
}

bool operator<=(const Time& val1, const Time& val2) {
  return val1.seconds < val2.seconds
         || (val1.seconds == val2.seconds
             && val1.fractionNs <= val2.fractionNs);
}

bool operator>(const Time& val1, const Time& val2) {
  return val1.seconds > val2.seconds
         || (val1.seconds == val2.seconds && val1.fractionNs > val2.fractionNs);
}

bool operator>=(const Time& val1, const Time& val2) {
  return val1.seconds > val2.seconds
         || (val1.seconds == val2.seconds
             && val1.fractionNs >= val2.fractionNs);
}
}  // namespace odbc
}  // namespace trino
