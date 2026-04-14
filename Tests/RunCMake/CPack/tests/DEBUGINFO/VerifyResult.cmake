if(NOT GENERATOR_TYPE STREQUAL "DEB")
  return()
endif()

set(whitespaces_ "[\t\n\r ]*")
set(hashsyms_ "[a-f0-9]+")

set(dbgsym_md5sums_md5sums
  "^(${hashsyms_}  usr/lib/debug/\\.build-id/[^/\r\n]+/[^/\r\n]+\\.debug${whitespaces_})+$")
set(dbgsym_md5sums_md5sums_permissions "-rw-r--r--")

if(PACKAGING_TYPE STREQUAL "COMPONENT")
  verifyDebControl("${FOUND_FILE_4}" "dbgsym_md5sums" "md5sums")
  verifyDebControl("${FOUND_FILE_5}" "dbgsym_md5sums" "md5sums")

elseif(PACKAGING_TYPE STREQUAL "MONOLITHIC")
  verifyDebControl("${FOUND_FILE_2}" "dbgsym_md5sums" "md5sums")

endif()
