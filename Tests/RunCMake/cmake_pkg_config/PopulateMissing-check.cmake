set(expected
  "juliet: Juliet;Golf;Foxtrot
"
)

file(READ "${RunCMake_TEST_BINARY_DIR}/populate-missing.txt" actual)

if(NOT(expected STREQUAL actual))
  set(RunCMake_TEST_FAILED "cmake_pkg_config populate-missing.txt does not match expected:\n${actual}")
endif()
