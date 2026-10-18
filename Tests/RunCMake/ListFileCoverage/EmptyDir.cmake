# On Windows an empty environment variable can read as undefined.
if(NOT CMAKE_HOST_WIN32 AND NOT DEFINED ENV{CMAKE_LISTFILE_COVERAGE_DIR})
  message(FATAL_ERROR "CMAKE_LISTFILE_COVERAGE_DIR is not defined")
endif()
