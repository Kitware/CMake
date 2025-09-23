set(expected
"Import Simple Found: TRUE
Include Directories: ${RunCMake_SOURCE_DIR}/TestDirectories/Include
Compile Options: TestCflag
Link Directories: ${RunCMake_SOURCE_DIR}/TestDirectories/Library
Link Libraries: @foreign_pkgcfg::import-simple
Link Options: TestLinkOption
"
)

file(READ "${RunCMake_TEST_BINARY_DIR}/import-simple.txt" actual)

if(NOT(expected STREQUAL actual))
  set(RunCMake_TEST_FAILED "cmake_pkg_config import-simple.txt does not match expected:\n${actual}")
endif()
