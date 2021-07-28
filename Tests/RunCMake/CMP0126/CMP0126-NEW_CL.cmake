
cmake_policy(SET CMP0126 NEW)

set(VAR 1)
set(VAR 2 CACHE STRING "")

if (NOT VAR EQUAL 1)
  message(FATAL_ERROR "normal variable does not exist anymore.")
endif()
