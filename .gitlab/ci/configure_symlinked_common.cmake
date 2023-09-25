# Ensure that the symlink tree is set up correctly.
if(NOT CMAKE_SOURCE_DIR STREQUAL "$ENV{CI_PROJECT_DIR}/work/cmake")
  message(FATAL_ERROR "Expected value of CMAKE_SOURCE_DIR:\n  $ENV{CI_PROJECT_DIR}/work/cmake\nActual value:\n  ${CMAKE_SOURCE_DIR}")
endif()
if(NOT CMAKE_BINARY_DIR STREQUAL "$ENV{CI_PROJECT_DIR}/work/build")
  message(FATAL_ERROR "Expected value of CMAKE_BINARY_DIR:\n  $ENV{CI_PROJECT_DIR}/work/build\nActual value:\n  ${CMAKE_BINARY_DIR}")
endif()
