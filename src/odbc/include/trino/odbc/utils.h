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

#ifndef _TRINO_ODBC_COMMON_UTILS
#define _TRINO_ODBC_COMMON_UTILS

#include <ignite/common/include/date.h>
#include <ignite/common/common.h>
#include <ignite/common/include/common/platform_utils.h>
#include <trino/odbc/timestamp.h>
#include <stdint.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <sstream>
#include <string>

#include "trino/odbc/time.h"

#ifdef IGNITE_FRIEND
#define IGNITE_FRIEND_EXPORT IGNITE_EXPORT
#else
#define IGNITE_FRIEND_EXPORT
#endif

using trino::odbc::Time;
using trino::odbc::Timestamp;

namespace trino {
namespace odbc {
namespace common {
/**
 * Replace all alphabetic symbols of the string with their lowercase
 * versions.
 * @param str String to be transformed.
 */
inline void IntoLower(std::string& str) {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

/**
 * Get lowercase version of the string.
 *
 * @param str Input string.
 * @return Lowercased version of the string.
 */
inline std::string ToLower(const std::string& str) {
  std::string res(str);
  IntoLower(res);
  return res;
}

/**
 * Replaces illegal URI characters in string with %-encoded characters.
 *
 * @param str String to be transformed.
 * @return str New string with replaced characters.
 */
inline std::string EncodeURIComponent(std::string unencoded) {
  std::ostringstream oss;

  for (char& c : unencoded) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '~' || c == '.'
        || c == '*' || c == '!' || c == '(' || c == ')') {
      oss << c;
    } else {
      oss << "%" << std::uppercase << std::hex << (0xff & c);
    }
  }
  return oss.str();
}

/**
 * Replaces whitespaces in string with underscores.
 *
 * @param str String to be transformed.
 */
inline void SpaceToUnderscore(std::string& str) {
  std::replace(str.begin(), str.end(), ' ', '_');
}

/**
 * Skip leading spaces.
 *
 * @param begin Iterator to the beginning of the character sequence.
 * @param end Iterator to the end of the character sequence.
 * @return Iterator to first non-blanc character.
 */
template < typename Iterator >
Iterator SkipLeadingSpaces(Iterator begin, Iterator end) {
  Iterator res = begin;

  while (isspace(*res) && res != end)
    ++res;

  return res;
}

/**
 * Skip trailing spaces.
 *
 * @param begin Iterator to the beginning of the character sequence.
 * @param end Iterator to the end of the character sequence.
 * @return Iterator to last non-blanc character.
 */
template < typename Iterator >
Iterator SkipTrailingSpaces(Iterator begin, Iterator end) {
  Iterator res = end - 1;

  while (isspace(*res) && res != begin - 1)
    --res;

  return res + 1;
}

/**
 * Get string representation of long in decimal form.
 *
 * @param val Long value to be converted to string.
 * @return String contataining decimal representation of the value.
 */
inline std::string LongToString(long val) {
  std::stringstream tmp;
  tmp << val;
  return tmp.str();
}

/**
 * Parse string to try and get int value.
 *
 * @param str String to be parsed.
 * @return String contataining decimal representation of the value.
 */
inline int ParseInt(const std::string& str) {
  return atoi(str.c_str());
}

/**
 * Copy characters.
 *
 * @param val Value.
 * @return Result.
 */
IGNITE_IMPORT_EXPORT char* CopyChars(const char* val);

/**
 * Release characters.
 *
 * @param val Value.
 */
IGNITE_IMPORT_EXPORT void ReleaseChars(char* val);

/**
 * Casts value of one type to another type, using stringstream.
 *
 * @param val Input value.
 * @param res Resulted value.
 */
template < typename T1, typename T2 >
void LexicalCast(const T2& val, T1& res) {
  std::stringstream converter;

  converter << val;
  converter >> res;
}

/**
 * Casts value of one type to another type, using stringstream.
 *
 * @param val Input value.
 * @return Resulted value.
 */
template < typename T1, typename T2 >
T1 LexicalCast(const T2& val) {
  T1 res;

  LexicalCast< T1, T2 >(val, res);

  return res;
}

/**
 * Check if all characters are digits.
 *
 * @param val Value to check.
 */
IGNITE_IMPORT_EXPORT bool AllDigits(const std::string& val);

/**
 * Converts 32-bit integer to big endian format
 *
 * @param value Input value
 * @return Resulting value
 */
IGNITE_IMPORT_EXPORT uint32_t ToBigEndian(uint32_t value);

/**
 * Convert Date type to standard C type time_t.
 *
 * @param date Date type value.
 * @return Corresponding value of time_t.
 */
inline time_t DateToCTime(const Date& date) {
  return static_cast< time_t >(date.GetSeconds());
}

/**
 * Convert Timestamp type to standard C type time_t.
 *
 * @param ts Timestamp type value.
 * @return Corresponding value of time_t.
 */
inline time_t TimestampToCTime(const Timestamp& ts) {
  return static_cast< time_t >(ts.GetSeconds());
}

/**
 * Convert Time type to standard C type time_t.
 *
 * @param time Time type value.
 * @return Corresponding value of time_t.
 */
inline time_t TimeToCTime(const Time& time) {
  return static_cast< time_t >(time.GetSeconds());
}

/**
 * Convert Date type to standard C type time_t.
 *
 * @param date Date type value.
 * @param ctime Corresponding value of struct tm.
 * @return True on success.
 */
inline bool DateToCTm(const Date& date, tm& ctime) {
  time_t tmt = DateToCTime(date);

  return ignite::odbc::common::IgniteGmTime(tmt, ctime);
}

/**
 * Convert Timestamp type to standard C type struct tm.
 *
 * @param ts Timestamp type value.
 * @param ctime Corresponding value of struct tm.
 * @return True on success.
 */
inline bool TimestampToCTm(const Timestamp& ts, tm& ctime) {
  time_t tmt = TimestampToCTime(ts);

  return ignite::odbc::common::IgniteGmTime(tmt, ctime);
}

/**
 * Convert Time type to standard C type struct tm.
 *
 * @param time Time type value.
 * @param ctime Corresponding value of struct tm.
 * @return True on success.
 */
inline bool TimeToCTm(const Time& time, tm& ctime) {
  time_t tmt = TimeToCTime(time);

  return ignite::odbc::common::IgniteGmTime(tmt, ctime);
}

/**
 * Convert standard C type time_t to Date.
 *
 * @param ctime Standard C type time_t.
 * @return Corresponding value of Date.
 */
inline Date CTimeToDate(time_t ctime) {
  int64_t c = ctime;
  return Date(c * 1000);
}

/**
 * Convert standard C type time_t to Time.
 *
 * @param ctime Standard C type time_t.
 * @param ns Nanosecond.
 * @return Corresponding value of Time.
 */
inline Time CTimeToTime(time_t ctime, int32_t ns) {
  return Time(ctime, ns);
}

/**
 * Convert standard C type time_t to Timestamp type.
 *
 * @param ctime Standard C type time_t.
 * @param ns Nanosecond.
 * @return Corresponding value of Timestamp.
 */
inline Timestamp CTimeToTimestamp(time_t ctime, int32_t ns) {
  return Timestamp(ctime, ns);
}

/**
 * Convert standard C type struct tm to Date type.
 *
 * @param ctime Standard C type struct tm.
 * @return Corresponding value of Date.
 */
inline Date CTmToDate(const tm& ctime) {
  time_t time = ignite::odbc::common::IgniteTimeGm(ctime);

  return CTimeToDate(time);
}

/**
 * Convert standard C type struct tm to Time type.
 *
 * @param ctime Standard C type struct tm.
 * @param ns Nanosecond.
 * @return Corresponding value of Time.
 */
inline Time CTmToTime(const tm& ctime, int32_t ns = 0) {
  time_t time = ignite::odbc::common::IgniteTimeGm(ctime);

  return CTimeToTime(time, ns);
}

/**
 * Convert standard C type struct tm to Timestamp type.
 *
 * @param ctime Standard C type struct tm.
 * @param ns Nanoseconds second fraction.
 * @return Corresponding value of Timestamp.
 */
inline Timestamp CTmToTimestamp(const tm& ctime, int32_t ns) {
  time_t time = ignite::odbc::common::IgniteTimeGm(ctime);

  return CTimeToTimestamp(time, ns);
}

/**
 * Make Date in human understandable way.
 *
 * Created Date uses GMT timezone.
 *
 * @param year Year.
 * @param month Month.
 * @param day Day.
 * @param hour Hour.
 * @param min Min.
 * @param sec Sec.
 * @return Date.
 */
IGNITE_FRIEND_EXPORT Date MakeDateGmt(int year = 1900, int month = 1,
                                      int day = 1, int hour = 0, int min = 0,
                                      int sec = 0);

/**
 * Make Date in human understandable way.
 *
 * Created Date uses local timezone.
 *
 * @param year Year.
 * @param month Month.
 * @param day Day.
 * @param hour Hour.
 * @param min Min.
 * @param sec Sec.
 * @return Date.
 */
IGNITE_FRIEND_EXPORT Date MakeDateLocal(int year = 1900, int month = 1,
                                        int day = 1, int hour = 0, int min = 0,
                                        int sec = 0);

/**
 * Make Time in human understandable way.
 *
 * Created Time uses GMT timezone.
 *
 * @param hour Hour.
 * @param min Minute.
 * @param sec Second.
 * @param ns Nanosecond.
 * @return Time.
 */
IGNITE_FRIEND_EXPORT Time MakeTimeGmt(int hour = 0, int min = 0, int sec = 0,
                                      int ns = 0);

/**
 * Make Time in human understandable way.
 *
 * Created Time uses Local timezone.
 *
 * @param hour Hour.
 * @param min Minute.
 * @param sec Second.
 * @param ns Nanosecond.
 * @return Time.
 */
IGNITE_FRIEND_EXPORT Time MakeTimeLocal(int hour = 0, int min = 0, int sec = 0,
                                        int ns = 0);

/**
 * Make Timestamp in human understandable way.
 *
 * Created Timestamp uses GMT timezone.
 *
 * @param year Year.
 * @param month Month.
 * @param day Day.
 * @param hour Hour.
 * @param min Minute.
 * @param sec Second.
 * @param ns Nanosecond.
 * @return Timestamp.
 */
IGNITE_FRIEND_EXPORT Timestamp MakeTimestampGmt(int year = 1900, int month = 1,
                                                int day = 1, int hour = 0,
                                                int min = 0, int sec = 0,
                                                long ns = 0);

/**
 * Make Date in human understandable way.
 *
 * Created Timestamp uses Local timezone.
 *
 * @param year Year.
 * @param month Month.
 * @param day Day.
 * @param hour Hour.
 * @param min Minute.
 * @param sec Second.
 * @param ns Nanosecond.
 * @return Timestamp.
 */
IGNITE_FRIEND_EXPORT Timestamp MakeTimestampLocal(int year = 1900,
                                                  int month = 1, int day = 1,
                                                  int hour = 0, int min = 0,
                                                  int sec = 0, long ns = 0);

/**
 * Meta-programming class.
 * Defines T1 as ::type if the condition is true, otherwise
 * defines T2 as ::type.
 */
template < bool, typename T1, typename T2 >
struct Conditional {
  typedef T1 type;
};

/**
 * Specialization for the false case.
 */
template < typename T1, typename T2 >
struct Conditional< false, T1, T2 > {
  typedef T2 type;
};

/**
 * Returns the bigger type.
 */
template < typename T1, typename T2 >
struct Bigger {
  typedef typename Conditional< (sizeof(T1) > sizeof(T2)), T1, T2 >::type type;
};

/**
 * Utility class to bind class instance with member function.
 */
template < typename R, typename T >
class BoundInstance {
 public:
  typedef R FunctionReturnType;
  typedef T ClassType;
  typedef FunctionReturnType (ClassType::*MemberFunctionType)();

  /**
   * Constructor.
   *
   * @param instance Class instance.
   * @param mfunc Member function.
   */
  BoundInstance(ClassType* instance, MemberFunctionType mfunc)
      : instance(instance), mfunc(mfunc) {
    // No-op.
  }

  /**
   * Invoke operator.
   *
   * @return Result of the invokation of the member function on the bound
   * instance.
   */
  FunctionReturnType operator()() {
    return (instance->*mfunc)();
  }

 private:
  /** Instance reference. */
  ClassType* instance;

  /** Member function pointer. */
  MemberFunctionType mfunc;
};

/**
 * Utility function for binding.
 */
template < typename R, typename T >
BoundInstance< R, T > Bind(T* instance, R (T::*mfunc)()) {
  return BoundInstance< R, T >(instance, mfunc);
}

/**
 * Method guard class template.
 *
 * Upon destruction calls provided method on provided class instance.
 *
 * @tparam T Value type.
 */
template < typename T >
class MethodGuard {
 public:
  /** Value type. */
  typedef T ValueType;

  /** Mehtod type. */
  typedef void (ValueType::*MethodType)();

  /**
   * Constructor.
   *
   * @param val Instance, to call method on.
   * @param method Method to call.
   */
  MethodGuard(ValueType* val, MethodType method) : val(val), method(method) {
    // No-op.
  }

  /**
   * Destructor.
   */
  ~MethodGuard() {
    if (val && method)
      (val->*method)();
  }

  /**
   * Release control over object.
   */
  void Release() {
    val = 0;
    method = 0;
  }

 private:
  /** Instance, to call method on. */
  ValueType* val;

  /** Method to call. */
  MethodType method;
};

/**
 * Deinit guard class template.
 *
 * Upon destruction calls provided deinit function on provided instance.
 *
 * @tparam T Value type.
 */
template < typename T >
class DeinitGuard {
 public:
  /** Value type. */
  typedef T ValueType;

  /** Deinit function type. */
  typedef void (*FuncType)(ValueType*);

  /**
   * Constructor.
   *
   * @param val Instance, to call method on.
   * @param method Method to call.
   */
  DeinitGuard(ValueType* val, FuncType method) : val(val), func(method) {
    // No-op.
  }

  /**
   * Destructor.
   */
  ~DeinitGuard() {
    if (val && func)
      (func)(val);
  }

  /**
   * Release control over object.
   */
  void Release() {
    val = 0;
    func = 0;
  }

 private:
  /** Instance, to call method on. */
  ValueType* val;

  /** Method to call. */
  FuncType func;
};

/**
 * Get dynamic library full name.
 * @param name Name without extension.
 * @return Full name.
 */
IGNITE_IMPORT_EXPORT std::string GetDynamicLibraryName(const char* name);
}  // namespace common
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_COMMON_UTILS
