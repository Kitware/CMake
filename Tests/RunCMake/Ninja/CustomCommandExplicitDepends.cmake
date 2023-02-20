cmake_minimum_required(VERSION 3.26)
project(CustomCommandExplicitDepends C)

add_custom_command(
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/command-option.h"
  COMMAND "${CMAKE_COMMAND}" -E touch
          "${CMAKE_CURRENT_BINARY_DIR}/command-option.h"
  COMMENT "Creating command-option.h"
  DEPENDS_EXPLICIT_ONLY
)

set(CMAKE_ADD_CUSTOM_COMMAND_DEPENDS_EXPLICIT_ONLY ON)
add_custom_command(
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/command-variable-on.h"
  COMMAND "${CMAKE_COMMAND}" -E touch
          "${CMAKE_CURRENT_BINARY_DIR}/command-variable-on.h"
  COMMENT "Creating command-variable-on.h"
)

set(CMAKE_ADD_CUSTOM_COMMAND_DEPENDS_EXPLICIT_ONLY OFF)
add_custom_command(
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/command-variable-off.h"
  COMMAND "${CMAKE_COMMAND}" -E touch
          "${CMAKE_CURRENT_BINARY_DIR}/command-variable-off.h"
  COMMENT "Creating command-variable-off.h"
)

add_library(dep SHARED dep.c)

add_library(top SHARED
  top.c
  "${CMAKE_CURRENT_BINARY_DIR}/command-option.h"
  "${CMAKE_CURRENT_BINARY_DIR}/command-variable-on.h"
  "${CMAKE_CURRENT_BINARY_DIR}/command-variable-off.h"
)
target_link_libraries(top PRIVATE dep)
