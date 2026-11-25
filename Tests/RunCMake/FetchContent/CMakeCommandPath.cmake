cmake_minimum_required(VERSION 3.30)
include(FetchContent)

file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_LIST_DIR}/../../ExternalProject/gitrepo.tgz")
set(repo "${CMAKE_CURRENT_BINARY_DIR}/gitrepo.git")
FetchContent_Populate(dep GIT_REPOSITORY "${repo}" GIT_TAG master)
file(TOUCH "${dep_SOURCE_DIR}/keep")

# Give the same executable a different path without copying a CMake installation.
cmake_path(GET CMAKE_COMMAND FILENAME cmake_name)
cmake_path(REPLACE_FILENAME CMAKE_COMMAND "./${cmake_name}")
# Allow for filesystems with one-second timestamp resolution.
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1.125)
FetchContent_Populate(dep GIT_REPOSITORY "${repo}" GIT_TAG master)
if(NOT EXISTS "${dep_SOURCE_DIR}/keep")
  message(FATAL_ERROR "Changing CMAKE_COMMAND caused a second clone")
endif()
