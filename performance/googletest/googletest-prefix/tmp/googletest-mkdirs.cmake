# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/zhang/source/repos/amazon-timestream-odbc-driver/performance/googletest/googletest-src"
  "C:/Users/zhang/source/repos/amazon-timestream-odbc-driver/performance/googletest/googletest-build"
  "C:/Users/zhang/source/repos/amazon-timestream-odbc-driver/performance/googletest/googletest-prefix"
  "C:/Users/zhang/source/repos/amazon-timestream-odbc-driver/performance/googletest/googletest-prefix/tmp"
  "C:/Users/zhang/source/repos/amazon-timestream-odbc-driver/performance/googletest/googletest-prefix/src/googletest-stamp"
  "C:/Users/zhang/source/repos/amazon-timestream-odbc-driver/performance/googletest/googletest-prefix/src"
  "C:/Users/zhang/source/repos/amazon-timestream-odbc-driver/performance/googletest/googletest-prefix/src/googletest-stamp"
)

set(configSubDirs Debug;Release;MinSizeRel;RelWithDebInfo)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/zhang/source/repos/amazon-timestream-odbc-driver/performance/googletest/googletest-prefix/src/googletest-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/zhang/source/repos/amazon-timestream-odbc-driver/performance/googletest/googletest-prefix/src/googletest-stamp${cfgdir}") # cfgdir has leading slash
endif()
