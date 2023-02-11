cmake_minimum_required(VERSION 3.26)
project(CustomCommandExplicitDepends C)

add_custom_command(
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/command.h"
  COMMAND "${CMAKE_COMMAND}" -E touch
          "${CMAKE_CURRENT_BINARY_DIR}/command.h"
  COMMENT "Creating command.h"
  DEPENDS_EXPLICIT_ONLY
)

add_library(dep STATIC dep.c)

add_library(top STATIC
  top.c
  "${CMAKE_CURRENT_BINARY_DIR}/command.h"
)
target_link_libraries(top PRIVATE dep)
