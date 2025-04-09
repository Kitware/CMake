cmake_minimum_required(VERSION 4.0)

project(EnvVars LANGUAGES NONE)

message(STATUS "${VARIABLE}: $ENV{${VARIABLE}}")
message(STATUS "Separator: $ENV{ListSeparator}")
message(STATUS "List: $ENV{ListVar}")

add_custom_target(EchoEnvVars ALL COMMAND
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/EchoVar.cmake")

install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/EchoVar.cmake")
