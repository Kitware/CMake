
# enforce policy CMP0125 to ensure predictable result of find_* commands
cmake_policy(SET CMP0125 NEW)

cmake_policy(SET CMP0126 OLD)

set(VAR 1)
set(VAR 2 CACHE STRING "")

if (VAR EQUAL 1)
  message(FATAL_ERROR "normal variable still exist.")
endif()


file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/file.txt" "")
set(VAR file.txt)
set(VAR "" CACHE STRING "" FORCE)
set_property(CACHE VAR PROPERTY TYPE UNINITIALIZED)

find_file(VAR NAMES file.txt PATHS "${CMAKE_CURRENT_BINARY_DIR}")

unset(VAR CACHE)
if (DEFINED VAR)
    message(FATAL_ERROR "find_file: normal variable still exist.")
endif()
