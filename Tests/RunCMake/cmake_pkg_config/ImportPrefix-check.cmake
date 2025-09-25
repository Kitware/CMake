set(expected
"larry: Alpha
curly: Alpha
moe: AltAlpha
shemp: AltAlpha
"
)

file(READ "${RunCMake_TEST_BINARY_DIR}/import-prefix.txt" actual)

if(NOT(expected STREQUAL actual))
  set(RunCMake_TEST_FAILED "cmake_pkg_config import-prefix.txt does not match expected:\n${actual}")
endif()
