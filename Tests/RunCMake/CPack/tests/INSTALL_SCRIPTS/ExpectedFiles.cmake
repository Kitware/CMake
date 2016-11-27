set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "2")
set(EXPECTED_FILE_1_COMPONENT "foo")
set(EXPECTED_FILE_CONTENT_1 "^/usr/foo${whitespaces_}/usr/foo/CMakeLists.txt$")
set(EXPECTED_FILE_2_COMPONENT "bar")
set(EXPECTED_FILE_CONTENT_2 "^/usr/bar${whitespaces_}/usr/bar/CMakeLists.txt$")
