set(tree "${RunCMake_TEST_BINARY_DIR}")

# The per-test file merges the files of all three cmake processes.
set(info "${tree}/test one.cmake-coverage.info")
if(EXISTS "${info}")
  file(READ "${info}" actual)
  string(REPLACE "${RunCMake_SOURCE_DIR}/" "<SRC>/" actual "${actual}")
  file(READ "${RunCMake_SOURCE_DIR}/CoverageToolCMake-lcov.txt" expected)
  string(REPLACE "\r\n" "\n" expected "${expected}")
  if(NOT actual STREQUAL expected)
    string(APPEND RunCMake_TEST_FAILED
      "${info} does not match CoverageToolCMake-lcov.txt.\n"
      "Expected:\n${expected}Actual:\n${actual}")
  endif()
else()
  string(APPEND RunCMake_TEST_FAILED "Missing ${info}\n")
endif()

# The scratch folders are gone, and the stale data of nocode with them.
foreach(path IN ITEMS
    "test one_cmake-coverage"
    "nocode_cmake-coverage"
    "nocode.cmake-coverage.info"
    )
  if(EXISTS "${tree}/${path}")
    string(APPEND RunCMake_TEST_FAILED "Unexpected ${tree}/${path}\n")
  endif()
endforeach()

# ctest itself records into the ambient directory.  Its tests do not.
set(cov_dir "${tree}/ambient")
set(forbid_sf_regex "/CoverageScript\\.cmake$")
include(${RunCMake_SOURCE_DIR}/../ListFileCoverage/check-lcov.cmake)

# The merge leaves out the records of scripts in the build tree.
set(info "${tree}/generated.cmake-coverage.info")
if(EXISTS "${info}")
  file(READ "${info}" actual)
  string(REPLACE "${RunCMake_SOURCE_DIR}/" "<SRC>/" actual "${actual}")
  set(expected "TN:generated
SF:<SRC>/CoverageChild.cmake
FNF:0
FNH:0
DA:1,1
LF:1
LH:1
end_of_record
")
  if(NOT actual STREQUAL expected)
    string(APPEND RunCMake_TEST_FAILED
      "${info} does not match.\nExpected:\n${expected}Actual:\n${actual}")
  endif()
else()
  string(APPEND RunCMake_TEST_FAILED "Missing ${info}\n")
endif()
# Tests whose records have no lines and no functions write no file.
foreach(path IN ITEMS
    "buildonly.cmake-coverage.info"
    "buildonly_cmake-coverage"
    "commentonly.cmake-coverage.info"
    "commentonly_cmake-coverage"
    "emptyrecord.cmake-coverage.info"
    "emptyrecord_cmake-coverage"
    )
  if(EXISTS "${tree}/${path}")
    string(APPEND RunCMake_TEST_FAILED "Unexpected ${tree}/${path}\n")
  endif()
endforeach()
