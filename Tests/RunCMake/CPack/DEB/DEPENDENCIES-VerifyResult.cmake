function(checkDepends_ FILE REGEX)
  set(whitespaces_ "[\t\n\r ]*")

  getPackageInfo("${FILE}" "FILE_INFO_")
  if(NOT FILE_INFO_ MATCHES "${REGEX}")
    message(FATAL_ERROR "Unexpected dependencies in '${FILE}'; file info: '${FILE_INFO_}'")
  endif()
endfunction()

checkDepends_("${FOUND_FILE_1}" ".*Depends${whitespaces_}:${whitespaces_}depend-application, depend-application-b.*")
# use wildcard as we are using dependency auto detection
checkDepends_("${FOUND_FILE_2}" ".*Depends${whitespaces_}:${whitespaces_}.*depend-application, depend-application-b.*")
checkDepends_("${FOUND_FILE_3}" ".*Depends${whitespaces_}:${whitespaces_}depend-headers.*")
checkDepends_("${FOUND_FILE_4}" ".*Depends${whitespaces_}:${whitespaces_}depend-default, depend-default-b.*")
checkDepends_("${FOUND_FILE_5}" ".*Depends${whitespaces_}:${whitespaces_}depend-default, depend-default-b.*")
