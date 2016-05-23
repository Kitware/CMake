set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "5")
set(EXPECTED_FILE_1 "dependencies-0.1.1-*-applications.deb")
set(EXPECTED_FILE_CONTENT_1 "^.*/usr/foo${whitespaces_}.*/usr/foo/test_prog$")
set(EXPECTED_FILE_2 "dependencies-0.1.1-*-applications_auto.deb")
set(EXPECTED_FILE_CONTENT_2 "^.*/usr/foo_auto${whitespaces_}.*/usr/foo_auto/test_prog$")
set(EXPECTED_FILE_3 "dependencies-0.1.1-*-headers.deb")
set(EXPECTED_FILE_CONTENT_3 "^.*/usr/bar${whitespaces_}.*/usr/bar/CMakeLists.txt$")
set(EXPECTED_FILE_4 "dependencies-0.1.1-*-libs.deb")
# dynamic lib extension is .so on Linux and .dylib on Mac so we will use a wildcard .* for it
set(EXPECTED_FILE_CONTENT_4 "^.*/usr/bas${whitespaces_}.*/usr/bas/libtest_lib\\..*$")
set(EXPECTED_FILE_5 "dependencies-0.1.1-*-libs_auto.deb")
set(EXPECTED_FILE_CONTENT_5 "^.*/usr/bas_auto${whitespaces_}.*/usr/bas_auto/libtest_lib\\..*$")
