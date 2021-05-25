
# enforce policy CMP0125 to ensure predictable result of find_* commands
cmake_policy(SET CMP0125 NEW)

cmake_policy(SET CMP0126 NEW)

set(VAR 1)
set(VAR 2 CACHE STRING "")

if (NOT VAR EQUAL 1)
  message(FATAL_ERROR "normal variable does not exist anymore.")
endif()


file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/file.txt" "")
set(VAR file.txt)
set(VAR "" CACHE STRING "" FORCE)
set_property(CACHE VAR PROPERTY TYPE UNINITIALIZED)

find_file(VAR NAMES file.txt PATHS "${CMAKE_CURRENT_BINARY_DIR}")

unset(VAR CACHE)
if (NOT DEFINED VAR)
  message(FATAL_ERROR "find_file: normal variable does not exist anymore.")
endif()
if (NOT VAR STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/file.txt")
  message(FATAL_ERROR "find_file: failed to set normal variable.")
endif()
