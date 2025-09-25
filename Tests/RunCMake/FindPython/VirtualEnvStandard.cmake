cmake_minimum_required(VERSION 3.15)

set (Python3_FIND_VIRTUALENV STANDARD)
find_package (Python3 REQUIRED)

if (Python3_EXECUTABLE MATCHES "^${PYTHON3_VIRTUAL_ENV}/.+")
  message (FATAL_ERROR "Python3 virtual env unexpectedly found.")
endif()
