#
#   Copyright <2022> Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License").
#   You may not use this file except in compliance with the License.
#   A copy of the License is located at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#   or in the "license" file accompanying this file. This file is distributed
#   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#   express or implied. See the License for the specific language governing
#   permissions and limitations under the License.
#

project(tests)

set(INTEGRATION_TEST "${CMAKE_CURRENT_SOURCE_DIR}/integration-test")
set(UNIT_TEST "${CMAKE_CURRENT_SOURCE_DIR}/unit-test")
set(DATA_WRITER "${CMAKE_CURRENT_SOURCE_DIR}/data-writer")

# Projects to build
add_subdirectory(${INTEGRATION_TEST})
add_subdirectory(${UNIT_TEST})
add_subdirectory(${DATA_WRITER})
