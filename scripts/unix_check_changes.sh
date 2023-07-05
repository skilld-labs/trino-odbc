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

# Check if there is a change within one day
# This may not be delivered to AWS

lastCommit=$(git log -1 --format=%cd --date=unix)
curTime=$(date +%s)
gap=$(($curTime-$lastCommit))
echo "Now is $curTime, last commit is $lastCommit, gap is $gap"

if [ $gap -gt 86400 ]
then
  echo "No changes for more than one day, skip build and test"
else
  # success case must contain key word 'continue' which is checked in mac github workflow
  echo "There is a change within one day, continue build and test"
fi
