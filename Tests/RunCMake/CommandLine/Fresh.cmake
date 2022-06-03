message(STATUS "CMAKE_SOURCE_DIR='${CMAKE_SOURCE_DIR}'")
message(STATUS "CMAKE_BINARY_DIR='${CMAKE_BINARY_DIR}'")
message(STATUS "OLD CACHED_VAR_1='${CACHED_VAR_1}'")
set(CACHED_VAR_1 "CACHED-VALUE-1" CACHE STRING "")
message(STATUS "NEW CACHED_VAR_1='${CACHED_VAR_1}'")
set(kept "${CMAKE_BINARY_DIR}/kept")
set(removed "${CMAKE_BINARY_DIR}/CMakeFiles/removed")
if(FIRST)
  file(WRITE "${kept}" "")
  file(WRITE "${removed}" "")
else()
  if(NOT EXISTS "${kept}")
    message(FATAL_ERROR "File was not kept:\n ${kept}")
  endif()
  if(EXISTS "${removed}")
    message(FATAL_ERROR "File was not removed:\n ${removed}")
  endif()
endif()