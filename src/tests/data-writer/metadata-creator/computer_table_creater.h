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

#ifndef _IGNITE_ODBC_COMPUTER_TABLE_CREATER
#define _IGNITE_ODBC_COMPUTER_TABLE_CREATER

#include "measure_metadata_creater.h"
#include <aws/timestream-write/model/MeasureValueType.h>

using Aws::TimestreamWrite::Model::MeasureValueType;

namespace timestream {
namespace odbc {

class ComputerTableCreater : public MeasureMetadataCreater {
 public:
  ComputerTableCreater() = default;

  virtual ~ComputerTableCreater() = default;

  virtual void CreateDimensions(Aws::Vector< Dimension >& dimensions) {
    Dimension region;
    region.WithName("region").WithValue("us-east-1");

    Dimension az;
    az.WithName("az").WithValue("az1");

    Dimension hostname;
    hostname.WithName("hostname").WithValue("host1");

    dimensions.push_back(region);
    dimensions.push_back(az);
    dimensions.push_back(hostname);
  }

  virtual void CreateRecords(Aws::Vector< Dimension >& dimensions,
                             Aws::Vector< Record >& values) {
    Record index;
    index.WithDimensions(dimensions)
        .WithMeasureName("index")
        .WithMeasureValueType(MeasureValueType::BIGINT);

    Record cpuUtilization;
    cpuUtilization.WithDimensions(dimensions)
        .WithMeasureName("cpu_utilization")
        .WithMeasureValueType(MeasureValueType::DOUBLE);

    Record memoryUtilization;
    memoryUtilization.WithDimensions(dimensions)
        .WithMeasureName("memory_utilization")
        .WithMeasureValueType(MeasureValueType::DOUBLE);

    values.push_back(index);
    values.push_back(cpuUtilization);
    values.push_back(memoryUtilization);
  }

  virtual void CreateMeasureValues(Aws::Vector< MeasureValue >& values) {
    MeasureValue index;
    index.WithName("index").WithType(MeasureValueType::BIGINT);

    MeasureValue cpuUtilization;
    cpuUtilization.WithName("cpu_utilization")
        .WithType(MeasureValueType::DOUBLE);

    MeasureValue memoryUtilization;
    memoryUtilization.WithName("memory_utilization")
        .WithType(MeasureValueType::DOUBLE);

    values.push_back(index);
    values.push_back(cpuUtilization);
    values.push_back(memoryUtilization);
  }

  virtual const char* GetMetricName() {
    return "computer_metrics";
  }

  static double fRand(double fMin, double fMax) {
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
  }

  static void AssignMeasureValues(Aws::Vector< MeasureValue >& values,
                                  int index) {
    values[0].WithValue(std::to_string(index));
    values[1].WithValue(std::to_string(fRand(0, 100)));
    values[2].WithValue(std::to_string(rand() % 1024));
  }

  static void AssignRecordValues(Aws::Vector< Record >& values, int index) {
    values[0].WithMeasureValue(std::to_string(index));
    values[1].WithMeasureValue(std::to_string(fRand(0, 100)));
    values[2].WithMeasureValue(std::to_string(rand() % 1024));
  }

  virtual RecordValueAssignFunPtr GetRecordValueAssignFunPtr() {
    return AssignRecordValues;
  }

  virtual MeasureValueAssignFunPtr GetMeasureValueAssignFunPtr() {
    return AssignMeasureValues;
  }
};
}  // namespace odbc
}  // namespace timestream

#endif  //_IGNITE_ODBC_COMPUTER_TABLE_CREATER
