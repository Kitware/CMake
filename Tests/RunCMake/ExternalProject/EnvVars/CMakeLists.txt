cmake_minimum_required(VERSION 4.0)

project(EnvVars LANGUAGES NONE)

message(STATUS "${VARIABLE}: $ENV{${VARIABLE}}")
message(STATUS "Separator: $ENV{ListSeparator}")
message(STATUS "List: $ENV{ListVar}")

if(MYLIST)
  message(STATUS "MYLIST:")
  foreach(__item IN LISTS MYLIST)
    message(STATUS "-- ${__item}")
  endforeach()
endif()

add_custom_target(EchoEnvVars ALL COMMENT "Build environment..."
  COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/EchoVar.cmake")

install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/EchoVar.cmake")

# Avoid repeating always-out-of-date EchoEnvVars during install step.
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY 1)
