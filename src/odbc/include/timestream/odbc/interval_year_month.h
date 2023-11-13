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

/**
 * @file
 * Declares trino::odbc::IntervalYearMonth class.
 */

#ifndef _TRINO_ODBC_INTERVAL_YEAR_MONTH
#define _TRINO_ODBC_INTERVAL_YEAR_MONTH

#include <ignite/common/common.h>
#include <stdint.h>

namespace trino {
namespace odbc {
/**
 * IntervalYearMonth type.
 */
class IGNITE_IMPORT_EXPORT IntervalYearMonth {
 public:
  /**
   * Copy constructor.
   *
   * @param another Another instance.
   */
  IntervalYearMonth(const IntervalYearMonth& another);

  /**
   * Constructor.
   *
   * @param year Number of years.
   * @param month Number of months.
   */
  IntervalYearMonth(int32_t year, int32_t month);

  /**
   * Copy operator.
   *
   * @param another Another instance.
   * @return This.
   */
  IntervalYearMonth& operator=(const IntervalYearMonth& another);

  /**
   * Returns number of years.
   *
   * @return number of years
   */
  int32_t GetYear() const {
    return year_;
  }

  /**
   * Returns number of months
   *
   * @return Number of months
   */
  int32_t GetMonth() const {
    return month_;
  }

  /**
   * Comparison operator override.
   *
   * @param val1 First value.
   * @param val2 Second value.
   * @return True if equal.
   */
  friend bool IGNITE_IMPORT_EXPORT operator==(const IntervalYearMonth& val1,
                                              const IntervalYearMonth& val2);

  /**
   * Comparison operator override.
   *
   * @param val1 First value.
   * @param val2 Second value.
   * @return True if not equal.
   */
  friend bool IGNITE_IMPORT_EXPORT operator!=(const IntervalYearMonth& val1,
                                              const IntervalYearMonth& val2);

  /**
   * Comparison operator override.
   *
   * @param val1 First value.
   * @param val2 Second value.
   * @return True if less.
   */
  friend bool IGNITE_IMPORT_EXPORT operator<(const IntervalYearMonth& val1,
                                             const IntervalYearMonth& val2);

  /**
   * Comparison operator override.
   *
   * @param val1 First value.
   * @param val2 Second value.
   * @return True if less or equal.
   */
  friend bool IGNITE_IMPORT_EXPORT operator<=(const IntervalYearMonth& val1,
                                              const IntervalYearMonth& val2);

  /**
   * Comparison operator override.
   *
   * @param val1 First value.
   * @param val2 Second value.
   * @return True if gretter.
   */
  friend bool IGNITE_IMPORT_EXPORT operator>(const IntervalYearMonth& val1,
                                             const IntervalYearMonth& val2);

  /**
   * Comparison operator override.
   *
   * @param val1 First value.
   * @param val2 Second value.
   * @return True if gretter or equal.
   */
  friend bool IGNITE_IMPORT_EXPORT operator>=(const IntervalYearMonth& val1,
                                              const IntervalYearMonth& val2);

 private:
  /** Number of years. */
  int32_t year_;

  /** Number of months. */
  int32_t month_;
};
}  // namespace odbc
}  // namespace trino

#endif  //_TRINO_ODBC_INTERVAL_YEAR_MONTH
