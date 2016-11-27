set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "1")
set(EXPECTED_FILES_NAME_GENERATOR_SPECIFIC_FORMAT TRUE)
set(EXPECTED_FILE_1_COMPONENT "test")

if(GENERATOR_TYPE STREQUAL "DEB")
  set(EXPECTED_FILE_CONTENT_1 "^.*/usr/${whitespaces_}.*/usr/empty/$")
elseif(GENERATOR_TYPE STREQUAL "RPM")
  set(EXPECTED_FILE_CONTENT_1 "^/usr/empty$")
elseif(GENERATOR_TYPE STREQUAL "TGZ")
  set(EXPECTED_FILE_CONTENT_1 "^[^\n]*empty/$")
endif()
