set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "3")
set(EXPECTED_FILE_1 "per_component-pkg_1_0.1.1-1_*.deb")
set(EXPECTED_FILE_CONTENT_1 "^.*/usr/foo${whitespaces_}.*/usr/foo/CMakeLists.txt$")
set(EXPECTED_FILE_2 "second_0.1.1-1_*.deb")
set(EXPECTED_FILE_CONTENT_2 "^.*/usr/foo${whitespaces_}.*/usr/foo/CMakeLists.txt$")
set(EXPECTED_FILE_3 "per_component-pkg_3_0.1.1-1_*.deb")
set(EXPECTED_FILE_CONTENT_3 "^.*/usr/foo${whitespaces_}.*/usr/foo/CMakeLists.txt$")
