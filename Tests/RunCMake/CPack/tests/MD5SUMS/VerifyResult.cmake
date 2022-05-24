set(whitespaces_ "[\t\n\r ]*")
set(hashsyms_ "[a-f0-9]+")
set(md5sums_md5sums "^${hashsyms_}  usr/bar/CMakeLists\.txt${whitespaces_}${hashsyms_}  usr/baz/CMakeLists\.txt${whitespaces_}${hashsyms_}  usr/foo/CMakeLists\.txt${whitespaces_}$")
verifyDebControl("${FOUND_FILE_1}" "md5sums" "md5sums")
