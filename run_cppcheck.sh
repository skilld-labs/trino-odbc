#!/bin/bash

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

RESULTS_FILE=cppcheck-results.log

# --force: force checks all define combinations (default max is 12)
# -iaws-sdk-cpp: avoid checking AWS C++ SDK source files in our repo
cppcheck --force --library=boost --library=timestream -UWIN32 ./src/ 2>  ${RESULTS_FILE}

if [ -s ${RESULTS_FILE} ]; then
    echo "!! Cppcheck errors found! Check ${RESULTS_FILE} for details."
    exit 1
else
    echo "No Cppcheck errors found."
fi
