set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "0")

if (NOT ${RunCMake_SUBTEST_SUFFIX} MATCHES "invalid")
  set(EXPECTED_FILES_COUNT "1")
  set(EXPECTED_FILE_1 "package_checksum*.tar.gz")
  set(EXPECTED_FILE_CONTENT_1 "^[^\n]*package_checksum*-[^\n]*/foo/\n[^\n]*package_checksum-[^\n]*/foo/CMakeLists.txt$")
endif()
