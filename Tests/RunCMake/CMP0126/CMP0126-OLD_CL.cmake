
cmake_policy(SET CMP0126 OLD)

set(VAR 1)
set(VAR 2 CACHE STRING "")

if (NOT VAR EQUAL 3)
  message(FATAL_ERROR "normal variable still exist.")
endif()
