string(FIND "${actual_stdout}" "TEST_ENV_" TEST_ENV_POS)
if (NOT TEST_ENV_POS EQUAL -1)
  message(FATAL_ERROR "Found TEST_ENV_ in environment")
endif()

include("${RunCMake_SOURCE_DIR}/check.cmake")
