set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "5")
set(EXPECTED_FILE_1 "debuginfo-applications-0*.rpm")
set(EXPECTED_FILE_CONTENT_1 "^/usr/foo${whitespaces_}/usr/foo/test_prog$")
set(EXPECTED_FILE_2 "debuginfo*-headers.rpm")
set(EXPECTED_FILE_CONTENT_2 "^/usr/bar${whitespaces_}/usr/bar/CMakeLists.txt$")
set(EXPECTED_FILE_3 "debuginfo*-libs.rpm")
set(EXPECTED_FILE_CONTENT_3 "^/usr/bas${whitespaces_}/usr/bas/libtest_lib.so$")

set(EXPECTED_FILE_4 "debuginfo-applications-debuginfo*.rpm")
set(EXPECTED_FILE_CONTENT_4 ".*")
set(EXPECTED_FILE_5 "debuginfo-libs-debuginfo*.rpm")
set(EXPECTED_FILE_CONTENT_5 ".*")
