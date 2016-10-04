set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "0")

if(NOT RunCMake_SUBTEST_SUFFIX STREQUAL "invalid")
  set(EXPECTED_FILES_COUNT "3")
  set(EXPECTED_FILE_1 "main_component-0.1.1-1.*.rpm")
  set(EXPECTED_FILE_CONTENT_1 "^/usr/foo${whitespaces_}/usr/foo/CMakeLists.txt$")
  set(EXPECTED_FILE_2 "main_component*-headers.rpm")
  set(EXPECTED_FILE_CONTENT_2 "^/usr/bar${whitespaces_}/usr/bar/CMakeLists.txt$")
  set(EXPECTED_FILE_3 "main_component*-libs.rpm")
  set(EXPECTED_FILE_CONTENT_3 "^/usr/bas${whitespaces_}/usr/bas/CMakeLists.txt$")
endif()
