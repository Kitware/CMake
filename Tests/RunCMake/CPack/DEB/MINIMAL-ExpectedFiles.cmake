set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "1")
set(EXPECTED_FILE_1 "minimal_0.1.1-1_*.deb")
set(EXPECTED_FILE_CONTENT_1 "^.*/usr/${whitespaces_}.*/usr/foo/${whitespaces_}.*/usr/foo/CMakeLists.txt$")
