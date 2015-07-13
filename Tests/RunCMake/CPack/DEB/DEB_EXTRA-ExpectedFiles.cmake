set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "3")
set(EXPECTED_FILE_1 "deb_extra-*-foo.deb")
set(EXPECTED_FILE_CONTENT_1 "^.*/usr/${whitespaces_}.*/usr/foo/${whitespaces_}.*/usr/foo/CMakeLists.txt$")
set(EXPECTED_FILE_2 "deb_extra-*-bar.deb")
set(EXPECTED_FILE_CONTENT_2 "^.*/usr/${whitespaces_}.*/usr/bar/${whitespaces_}.*/usr/bar/CMakeLists.txt$")
set(EXPECTED_FILE_3 "deb_extra-*-bas.deb")
set(EXPECTED_FILE_CONTENT_3 "^.*/usr/${whitespaces_}.*/usr/bas/${whitespaces_}.*/usr/bas/CMakeLists.txt$")
