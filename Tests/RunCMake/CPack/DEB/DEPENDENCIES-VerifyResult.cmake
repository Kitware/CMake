function(checkDependencies_ FILE REGEX)
  set(whitespaces_ "[\t\n\r ]*")

  getPackageInfo("${FILE}" "FILE_INFO_")
  if(NOT FILE_INFO_ MATCHES "${REGEX}")
    message(FATAL_ERROR "Unexpected dependencies in '${FILE}'; file info: '${FILE_INFO_}'")
  endif()
endfunction()

foreach(dependency_type_ DEPENDS CONFLICTS ENHANCES BREAKS REPLACES RECOMMENDS SUGGESTS)
  string(TOLOWER "${dependency_type_}" lower_dependency_type_)
  string(SUBSTRING ${lower_dependency_type_} 1 -1 lower_dependency_type_tail_)
  string(SUBSTRING ${dependency_type_} 0 1 dependency_type_head_)
  set(dependency_type_name_ "${dependency_type_head_}${lower_dependency_type_tail_}")

  checkDependencies_("${FOUND_FILE_1}" ".*${dependency_type_name_}${whitespaces_}:${whitespaces_}${lower_dependency_type_}-application, ${lower_dependency_type_}-application-b.*")
  checkDependencies_("${FOUND_FILE_2}" ".*${dependency_type_name_}${whitespaces_}:${whitespaces_}.*${lower_dependency_type_}-application, ${lower_dependency_type_}-application-b.*")
  checkDependencies_("${FOUND_FILE_3}" ".*${dependency_type_name_}${whitespaces_}:${whitespaces_}${lower_dependency_type_}-headers.*")
  checkDependencies_("${FOUND_FILE_4}" ".*${dependency_type_name_}${whitespaces_}:${whitespaces_}${lower_dependency_type_}-default, ${lower_dependency_type_}-default-b.*")
  checkDependencies_("${FOUND_FILE_5}" ".*${dependency_type_name_}${whitespaces_}:${whitespaces_}${lower_dependency_type_}-default, ${lower_dependency_type_}-default-b.*")
endforeach()

checkDependencies_("${FOUND_FILE_1}" ".*Provides${whitespaces_}:${whitespaces_}provided-default, provided-default-b")
checkDependencies_("${FOUND_FILE_2}" ".*Provides${whitespaces_}:${whitespaces_}provided-default, provided-default-b")
checkDependencies_("${FOUND_FILE_3}" ".*Provides${whitespaces_}:${whitespaces_}provided-default, provided-default-b")
checkDependencies_("${FOUND_FILE_4}" ".*Provides${whitespaces_}:${whitespaces_}provided-lib.*")
checkDependencies_("${FOUND_FILE_5}" ".*Provides${whitespaces_}:${whitespaces_}provided-lib_auto.*, provided-lib_auto-b.*")

# PREDEPENDS
checkDependencies_("${FOUND_FILE_1}" ".*Pre-Depends${whitespaces_}:${whitespaces_}predepends-application, predepends-application-b.*")
checkDependencies_("${FOUND_FILE_2}" ".*Pre-Depends${whitespaces_}:${whitespaces_}.*predepends-application, predepends-application-b.*")
checkDependencies_("${FOUND_FILE_3}" ".*Pre-Depends${whitespaces_}:${whitespaces_}predepends-headers.*")
checkDependencies_("${FOUND_FILE_4}" ".*Pre-Depends${whitespaces_}:${whitespaces_}predepends-default, predepends-default-b.*")
checkDependencies_("${FOUND_FILE_5}" ".*Pre-Depends${whitespaces_}:${whitespaces_}predepends-default, predepends-default-b.*")
