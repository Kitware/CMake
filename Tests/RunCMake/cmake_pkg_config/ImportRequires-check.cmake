set(expected
  "alpha: Alpha
bravo: Bravo;Alpha
charlie: Charlie;Bravo;Alpha
delta: Delta
echo: Echo;Bravo;Alpha;Delta
"
)

file(READ "${RunCMake_TEST_BINARY_DIR}/import-requires.txt" actual)

if(NOT(expected STREQUAL actual))
  set(RunCMake_TEST_FAILED "cmake_pkg_config import-requires.txt does not match expected:\n${actual}")
endif()
