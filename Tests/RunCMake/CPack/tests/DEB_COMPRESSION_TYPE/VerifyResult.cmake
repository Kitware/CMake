# deb(5) allows bzip2 and lzma only for data.tar, so control.tar must
# fall back to gzip for those.
set(control_suffix_gzip ".gz")
set(control_suffix_bzip2 ".gz")
set(control_suffix_lzma ".gz")
set(control_suffix_xz ".xz")
set(control_suffix_zstd ".zst")
set(control_suffix_none "")

set(data_suffix_gzip ".gz")
set(data_suffix_bzip2 ".bz2")
set(data_suffix_lzma ".lzma")
set(data_suffix_xz ".xz")
set(data_suffix_zstd ".zst")
set(data_suffix_none "")

set(expected_members
  "debian-binary"
  "control.tar${control_suffix_${RunCMake_SUBTEST_SUFFIX}}"
  "data.tar${data_suffix_${RunCMake_SUBTEST_SUFFIX}}"
  )

execute_process(COMMAND ${CMAKE_COMMAND} -E tar tf "${FOUND_FILE_1}"
  OUTPUT_VARIABLE actual_members
  RESULT_VARIABLE result
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE "\n" ";" actual_members "${actual_members}")

if(NOT result EQUAL 0 OR NOT actual_members STREQUAL expected_members)
  message(FATAL_ERROR
    "Unexpected members in '${FOUND_FILE_1}':\n"
    "  expected: ${expected_members}\n"
    "  actual: ${actual_members}")
endif()
