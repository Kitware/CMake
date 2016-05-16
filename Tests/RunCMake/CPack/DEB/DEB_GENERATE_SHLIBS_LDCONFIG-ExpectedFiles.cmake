set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "1")
set(EXPECTED_FILE_1 "generate_shlibs_ldconfig_0.1.1-1_*.deb")
# dynamic lib extension is .so on Linux and .dylib on Mac so we will use a wildcard .* for it
set(EXPECTED_FILE_CONTENT_1 "^.*/usr/lib${whitespaces_}.*/usr/lib/libtest_lib\\..*$")
