if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(path "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/impl-Debug.ninja")
else ()
  set(path "${RunCMake_TEST_BINARY_DIR}/build.ninja")
endif ()

if (NOT EXISTS "${path}")
  list(APPEND RunCMake_TEST_FAILED
    "Failed to find `ninja` build file: '${path}'")
endif ()

file(READ "${path}" rspfiles
  REGEX "^ *RSP_FILE =")

if (rspfiles MATCHES "\\$out\\.rsp$")
  message(FATAL_ERROR
    "rspfiles for modules should be specified explicitly")
elseif (NOT rspfiles MATCHES "ddi\\.rsp")
  message(FATAL_ERROR
    "rspfiles for scanning rules should be specified according to scan output filenames")
endif ()

string(REPLACE ";" "\n  " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
