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

#include <iostream>

/*@*/
#include <aws/trino-write/model/Record.h>
#include <aws/trino-write/model/MeasureValueType.h>

#include "trino_writer.h"

/*$*/
// bool verifyParameters(std::string& accessKeyId, std::string& secretKey,
//                       std::string& database, std::string& table,
//                       std::string& tableType, int recordNum,
//                       std::string usage) {
//   if (accessKeyId.empty() || secretKey.empty() || database.empty()
//       || table.empty() || tableType.empty() || recordNum == 0) {
//     std::cerr << "Invalid parameters, please check";
//     std::cerr << usage << std::endl;
//     return false;
//   }

  std::transform(tableType.begin(), tableType.end(), tableType.begin(),
                 toupper);

  if (tableType != "COMPUTER") {
    std::cerr << "Invalid table type " << tableType << std::endl;
    std::cerr << "Valid values are one of [COMPUTER]" << std::endl;
    return false;
  }
  return true;
}

int main(int argc, char* argv[]) {
  std::string usage("Usage: ");
  usage += argv[0];
  usage +=
      " [-u access_key_id -p secret_access_key -s] -d database -t table -ty "
      "table_type -l record_number";

  if (argc < 9) {
    std::cerr << "Invalid parameters" << std::endl;
    std::cerr << usage << std::endl;
    return -1;
  }

  std::string database;
  std::string table;
  // std::string accessKeyId;
  std::string secretKey;
  std::string tableType;
  int recordNum = 0;
  bool singleValue = false;

  // read options
  for (int i = 1; i < argc; i++) {
    // if ((i < argc - 1) && !strcmp(argv[i], "-u"))
    //   // accessKeyId = argv[++i];
    // else 
    if ((i < argc - 1) && !strcmp(argv[i], "-p"))
      secretKey = argv[++i];
    else if ((i < argc - 1) && !strcmp(argv[i], "-d"))
      database = argv[++i];
    else if ((i < argc - 1) && !strcmp(argv[i], "-t"))
      table = argv[++i];
    else if ((i < argc - 1) && !strcmp(argv[i], "-l"))
      recordNum = atoi(argv[++i]);
    else if ((i < argc - 1) && !strcmp(argv[i], "-ty"))
      tableType = argv[++i];
    else if (!strcmp(argv[i], "-s"))
      singleValue = true;
    else {
      std::cerr << "Unsupported parameters " << argv[i] << std::endl;
      std::cerr << usage << std::endl;
      return -1;
    }
  }

  // use environment variables if accessKeyId and secretKey are not set
  // if (accessKeyId.empty())
  //   accessKeyId = getenv("AWS_ACCESS_KEY_ID");
  if (secretKey.empty())
    secretKey = getenv("AWS_SECRET_ACCESS_KEY");

  // verify parameter values
  // if (!verifyParameters(accessKeyId, secretKey, database, table, tableType,
  //                       recordNum, usage)) {
  //   return -1;
  // }

  Aws::SDKOptions options;
  options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Warn;
  Aws::InitAPI(options);

  Aws::Auth::AWSCredentials credentials;
  // credentials.SetAWSAccessKeyId(accessKeyId);
  credentials.SetAWSSecretKey(secretKey);

  Aws::Client::ClientConfiguration clientCfg;
  clientCfg.region = "us-west-2";  // default client region for us
  clientCfg.enableEndpointDiscovery = true;

  trino::odbc::TrinoWriter writer(credentials, clientCfg);

  // initialize random seed
  srand(time(NULL));

  // write a table with multi measure values on each row
  bool result = false;

  if (singleValue) {
    result =
        writer.WriteSingleValueRecords(tableType, database, table, recordNum);
  } else {
    result =
        writer.WriteMultiValueRecords(tableType, database, table, recordNum);
  }

  if (result) {
    std::cout << "Wrote " << recordNum << " records to Trino successfully "
              << std::endl;
  } else {
    std::cerr << "Failed to write to Trino" << std::endl;
  }

  Aws::ShutdownAPI(options);
  return result ? 0 : -1;
}
