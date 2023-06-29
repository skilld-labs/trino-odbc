# The MIT License (MIT)
#
# Copyright (c) 2015 Crascit
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Modifications Copyright Amazon.com, Inc. or its affiliates. 
# SPDX-License-Identifier: MIT.

macro(fetch_googletest _download_module_path _download_root)
    set(GOOGLETEST_DOWNLOAD_ROOT ${_download_root})
    configure_file(
        ${_download_module_path}/googletest-download.cmake
        ${_download_root}/CMakeLists.txt
        @ONLY
        )
    unset(GOOGLETEST_DOWNLOAD_ROOT)

    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY
            ${_download_root}
        )
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY
            ${_download_root}
        )

    # adds the targers: gtest, gtest_main, gmock, gmock_main
    add_subdirectory(
        ${_download_root}/googletest-src EXCLUDE_FROM_ALL
        ${_download_root}/googletest-build EXCLUDE_FROM_ALL
        )
endmacro()