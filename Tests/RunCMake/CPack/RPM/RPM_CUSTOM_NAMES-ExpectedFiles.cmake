set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "3")
set(EXPECTED_FILE_1 "custom_names-pkg_1*.rpm")
set(EXPECTED_FILE_CONTENT_1 "^/usr/foo${whitespaces_}/usr/foo/CMakeLists.txt$")
set(EXPECTED_FILE_2 "second*.rpm")
set(EXPECTED_FILE_CONTENT_2 "^/usr/foo${whitespaces_}/usr/foo/CMakeLists.txt$")
set(EXPECTED_FILE_3 "pkg_3_abc.rpm")
set(EXPECTED_FILE_CONTENT_3 "^/usr/foo${whitespaces_}/usr/foo/CMakeLists.txt$")
