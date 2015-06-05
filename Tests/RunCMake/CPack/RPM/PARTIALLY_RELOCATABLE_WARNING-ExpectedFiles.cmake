set(whitespaces_ "[\t\n\r ]*")

set(EXPECTED_FILES_COUNT "1")
set(EXPECTED_FILE_1 "PARTIALLY_RELOCATABLE_WARNING-0.1.1-*.rpm")
set(EXPECTED_FILE_CONTENT_1 "^/not_relocatable${whitespaces_}/not_relocatable/CMakeLists.txt${whitespaces_}/opt$")
