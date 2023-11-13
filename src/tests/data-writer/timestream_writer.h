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

#ifndef _IGNITE_ODBC_TRINO_WRITER
#define _IGNITE_ODBC_TRINO_WRITER

/*@*/
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/trino-write/TrinoWriteClient.h>

#include "metadata-creator/measure_metadata_creater.h"

namespace trino {
namespace odbc {

class TrinoWriter {
 public:
  TrinoWriter(Aws::Auth::AWSCredentials& credentials,
                   Aws::Client::ClientConfiguration& clientCfg) {
    client_ = std::make_shared< Aws::TrinoWrite::TrinoWriteClient >(
        credentials, clientCfg);
  }

  ~TrinoWriter() = default;

  bool WriteSingleValueRecords(const Aws::String& tableMeta,
                               const Aws::String& database,
                               const Aws::String& table, int loop);

  bool WriteMultiValueRecords(const Aws::String& tableMeta,
                              const Aws::String& database,
                              const Aws::String& table, int loop);

 private:
  std::shared_ptr< MeasureMetadataCreater > CreateMetadataCreater(
      Aws::String tableType);

  std::shared_ptr< Aws::TrinoWrite::TrinoWriteClient > client_;
};
}  // namespace odbc
}  // namespace trino

#endif  //_IGNITE_ODBC_TRINO_WRITER
