
find_package (Python3 REQUIRED)

if (NOT Python3_EXECUTABLE MATCHES "^${PYTHON3_VIRTUAL_ENV}/.+")
  message (FATAL_ERROR "Fail to use virtual environment")
endif()
