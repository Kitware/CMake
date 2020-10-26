
cmake_minimum_required(VERSION 3.18...3.19)

# define input variable
set (path "")

separate_arguments(CMAKE_PATH_ARGUMENTS UNIX_COMMAND "${CMAKE_PATH_ARGUMENTS}")

if (CHECK_INVALID_OUTPUT)
  # special handling for CMAKE_PATH
  list(GET CMAKE_PATH_ARGUMENTS 0 command)
  if (command STREQUAL "CMAKE_PATH")
    cmake_path(CMAKE_PATH "" "input")
  else()
    cmake_path(${CMAKE_PATH_ARGUMENTS} "")
  endif()
else()
  cmake_path(${CMAKE_PATH_ARGUMENTS})
endif()
