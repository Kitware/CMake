function(checkPackageInfo_ TYPE FILE REGEX)
  set(whitespaces_ "[\t\n\r ]*")

  getPackageInfo("${FILE}" "FILE_INFO_")
  if(NOT FILE_INFO_ MATCHES "${REGEX}")
    message(FATAL_ERROR "Unexpected ${TYPE} in '${FILE}'; file info: '${FILE_INFO_}'")
  endif()
endfunction()

# check package name
checkPackageInfo_("name" "${FOUND_FILE_1}" ".*Package${whitespaces_}:${whitespaces_}per_component-pkg_1")
checkPackageInfo_("name" "${FOUND_FILE_2}" ".*Package${whitespaces_}:${whitespaces_}second")
checkPackageInfo_("name" "${FOUND_FILE_3}" ".*Package${whitespaces_}:${whitespaces_}per_component-pkg_3")

# check package group
checkPackageInfo_("group" "${FOUND_FILE_1}" ".*Section${whitespaces_}:${whitespaces_}default")
checkPackageInfo_("group" "${FOUND_FILE_2}" ".*Section${whitespaces_}:${whitespaces_}second_group")
checkPackageInfo_("group" "${FOUND_FILE_3}" ".*Section${whitespaces_}:${whitespaces_}default")
