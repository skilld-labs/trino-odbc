# Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# A copy of the License is located at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# or in the "license" file accompanying this file. This file is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.

CHECK=1
if [[ -z "${AWS_ACCESS_KEY_ID}" ]]; then
  echo "AWS_ACCESS_KEY_ID environment variable is not set"
  CHECK=0
else
  echo "AWS_ACCESS_KEY_ID=${AWS_ACCESS_KEY_ID}"
fi

if [[ -z "${AWS_SECRET_ACCESS_KEY}" ]]; then
  echo "AWS_SECRET_ACCESS_KEY environment variable is not set"
  CHECK=0
else
  echo "AWS_SECRET_ACCESS_KEY=***"
fi

if [[ -z "${ODBC_LIB_PATH}" ]]; then
  echo "ODBC_LIB_PATH environment variable is not set"
  CHECK=0
else
  echo "ODBC_LIB_PATH=${ODBC_LIB_PATH}"
fi

if [[ "${CHECK}" -eq "0" ]]; then
  echo "Missing envrionment variables, please set them accordingly."
else
  echo "Environment variables are all set"
fi

