set(expected
"Include Directories: /TestDirectories/Include
Link Directories: /TestDirectories/Library
"
)

file(READ "${RunCMake_TEST_BINARY_DIR}/import-system.txt" actual)

if(NOT(expected STREQUAL actual))
  set(RunCMake_TEST_FAILED "cmake_pkg_config import-system.txt does not match expected:\n${actual}")
endif()
