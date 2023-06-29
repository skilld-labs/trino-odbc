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

#ifndef _TIMESTREAM_ODBC_SYSTEM_SYSTEM_DSN
#define _TIMESTREAM_ODBC_SYSTEM_SYSTEM_DSN

#include <timestream/odbc/system/odbc_constants.h>

namespace timestream {
namespace odbc {
namespace config {
class Configuration;
}
}  // namespace odbc
}  // namespace timestream

#ifdef _WIN32
/**
 * Display connection window for user to configure connection parameters.
 *
 * @param windowParent Parent window handle.
 * @param config Output configuration.
 * @return True on success and false on fail.
 */
bool DisplayConnectionWindow(void* windowParent,
                             timestream::odbc::config::Configuration& config);
#endif

#endif  //_TIMESTREAM_ODBC_SYSTEM_SYSTEM_DSN
