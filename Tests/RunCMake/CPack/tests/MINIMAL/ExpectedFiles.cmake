set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "1")

if(GENERATOR_TYPE STREQUAL "DEB")
  set(EXPECTED_FILE_CONTENT_1 "^.*/usr/${whitespaces_}.*/usr/foo/${whitespaces_}.*/usr/foo/CMakeLists.txt$")
elseif(GENERATOR_TYPE STREQUAL "RPM")
  set(EXPECTED_FILE_CONTENT_1 "^/usr/foo${whitespaces_}/usr/foo/CMakeLists.txt$")
elseif(GENERATOR_TYPE STREQUAL "TGZ")
  set(EXPECTED_FILE_CONTENT_1 "^[^\n]*minimal-0.1.1-[^\n]*/foo/\n[^\n]*minimal-0.1.1-[^\n]*/foo/CMakeLists.txt$")
endif()
