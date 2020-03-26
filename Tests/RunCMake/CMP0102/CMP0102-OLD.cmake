
cmake_policy(SET CMP0102 OLD)

include (CMP0102-Common.cmake)
get_property(is_type_set CACHE CMP0102_TEST_VARIABLE
  PROPERTY TYPE SET)
if (NOT is_type_set)
  message(FATAL_ERROR
    "There is a cache entry for an undefined variable after "
    "`mark_as_advanced`.")
endif ()
get_property(type CACHE CMP0102_TEST_VARIABLE
  PROPERTY TYPE)
if (NOT type STREQUAL "UNINITIALIZED")
  message(FATAL_ERROR
    "The cache type for CMP0102_TEST_VARIABLE is not "
    "UNINITIALIZED")
endif ()
