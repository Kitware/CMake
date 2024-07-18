file(STRINGS ${RunCMake_TEST_BINARY_DIR}/.ninja_log lines ENCODING UTF-8)
list(FILTER lines INCLUDE REGEX ".*install.*util")
list(LENGTH lines len)

if (NOT ${len} STREQUAL ${INSTALL_COUNT})
  set(RunCMake_TEST_FAILED "Wrong number of cmake -P calls to install: ${len}/${INSTALL_COUNT}")
endif()
