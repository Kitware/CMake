
cmake_policy(SET CMP0102 NEW)

include (CMP0102-Common.cmake)
get_property(is_type_set CACHE CMP0102_TEST_VARIABLE
  PROPERTY TYPE SET)
if (is_type_set)
  get_property(type CACHE CMP0102_TEST_VARIABLE
    PROPERTY TYPE)
  message(FATAL_ERROR
    "There is a cache entry for an undefined variable after "
    "`mark_as_advanced`.")
endif ()
