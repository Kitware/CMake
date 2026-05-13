set(CACHE_EXPECTED_FILE "${RunCMake_TEST_SOURCE_DIR}/${case}-CMakeCacheRegex.txt")
set(CACHE_ACTUAL_FILE "${RunCMake_TEST_BINARY_DIR}/CMakeCache.txt")

if(NOT EXISTS ${CACHE_EXPECTED_FILE})
  message(FATAL_ERROR "Could not find ${RunCMake_TEST_SOURCE_DIR}/${case}-CMakeCacheRegex.txt")
endif()

file(READ ${CACHE_EXPECTED_FILE} CACHE_EXPECTED)
string(REGEX REPLACE "\r\n" "\n" CACHE_EXPECTED "${CACHE_EXPECTED}")
string(REGEX REPLACE "\n+$" "" CACHE_EXPECTED "${CACHE_EXPECTED}")
file(READ ${CACHE_ACTUAL_FILE} CACHE_ACTUAL)
string(REGEX REPLACE "\r\n" "\n" CACHE_ACTUAL "${CACHE_ACTUAL}")
string(REGEX REPLACE "\n+$" "" CACHE_ACTUAL "${CACHE_ACTUAL}")

if(NOT "${CACHE_ACTUAL}" MATCHES "${CACHE_EXPECTED}")
  set(RunCMake_TEST_FAILED "${CACHE_ACTUAL_FILE} does not match ${CACHE_EXPECTED_FILE}:

CMakeCache.txt contents = [\n${CACHE_ACTUAL}\n]
Expected = [\n${CACHE_EXPECTED}\n]")
endif()
