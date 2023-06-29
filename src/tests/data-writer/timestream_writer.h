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

#ifndef _IGNITE_ODBC_TIMESTREAM_WRITER
#define _IGNITE_ODBC_TIMESTREAM_WRITER

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/timestream-write/TimestreamWriteClient.h>

#include "metadata-creator/measure_metadata_creater.h"

namespace timestream {
namespace odbc {

class TimestreamWriter {
 public:
  TimestreamWriter(Aws::Auth::AWSCredentials& credentials,
                   Aws::Client::ClientConfiguration& clientCfg) {
    client_ = std::make_shared< Aws::TimestreamWrite::TimestreamWriteClient >(
        credentials, clientCfg);
  }

  ~TimestreamWriter() = default;

  bool WriteSingleValueRecords(const Aws::String& tableMeta,
                               const Aws::String& database,
                               const Aws::String& table, int loop);

  bool WriteMultiValueRecords(const Aws::String& tableMeta,
                              const Aws::String& database,
                              const Aws::String& table, int loop);

 private:
  std::shared_ptr< MeasureMetadataCreater > CreateMetadataCreater(
      Aws::String tableType);

  std::shared_ptr< Aws::TimestreamWrite::TimestreamWriteClient > client_;
};
}  // namespace odbc
}  // namespace timestream

#endif  //_IGNITE_ODBC_TIMESTREAM_WRITER
