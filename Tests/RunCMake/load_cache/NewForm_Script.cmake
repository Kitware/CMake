load_cache(${RunCMake_BINARY_DIR}/test_project READ_WITH_PREFIX LOAD_CACHE_TEST_
  CACHE_STRING
  CACHE_BOOL
  CACHE_INTERNAL)

if(NOT LOAD_CACHE_TEST_CACHE_STRING STREQUAL "cache string")
  message(FATAL_ERROR "CACHE_STRING: was ${CACHE_STRING}, expected \"cache string\"")
endif()

if(NOT LOAD_CACHE_TEST_CACHE_BOOL)
  message(FATAL_ERROR "CACHE_BOOL: was falsey, expected ON")
endif()

if(NOT LOAD_CACHE_TEST_CACHE_INTERNAL STREQUAL "cache internal")
  message(FATAL_ERROR "CACHE_INTERNAL: was ${CACHE_INTENRAL}, expected \"cache internal\"")
endif()
