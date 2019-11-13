load_cache(${RunCMake_BINARY_DIR}/test_project INCLUDE_INTERNALS CACHE_INTERNAL)

if(NOT CACHE_STRING STREQUAL "cache string")
  message(FATAL_ERROR "CACHE_STRING: was ${CACHE_STRING}, expected \"cache string\"")
endif()

if(NOT CACHE_BOOL)
  message(FATAL_ERROR "CACHE_BOOL: was falsey, expected ON")
endif()

if(NOT CACHE_INTERNAL STREQUAL "cache internal")
  message(FATAL_ERROR "CACHE_INTERNAL: was ${CACHE_INTENRAL}, expected \"cache internal\"")
endif()
