# Preseed detection results so the required-library checks do not depend on
# an installed OpenMP runtime.  The library paths are stand-ins, never linked.
set(OpenMP_C_FLAGS "-D_OPENMP=201511")
set(OpenMP_C_LIB_NAMES old)
set(OpenMP_C_SPEC_DATE 201511)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/old.a" "")
set(OpenMP_old_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/old.a" CACHE FILEPATH "")
find_package(OpenMP REQUIRED COMPONENTS C)
if(NOT _OPENMP_C_REQUIRED_LIB_VARS STREQUAL "OpenMP_old_LIBRARY")
  message(FATAL_ERROR "Parent required libraries: ${_OPENMP_C_REQUIRED_LIB_VARS}")
endif()

add_subdirectory(RepeatedRequiredLibraries)
