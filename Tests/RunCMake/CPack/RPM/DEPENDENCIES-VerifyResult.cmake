function(checkDependencies_ FILE TYPE COMPARE_LIST)
  set(whitespaces_ "[\t\n\r ]*")

  execute_process(COMMAND ${RPM_EXECUTABLE} -qp --${TYPE} ${FILE}
          WORKING_DIRECTORY "${CPACK_TEMPORARY_DIRECTORY}"
          OUTPUT_VARIABLE FILE_DEPENDENCIES_
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  string(REPLACE "\n" ";" FILE_DEPENDENCIES_LIST_ "${FILE_DEPENDENCIES_}")

  foreach(COMPARE_REGEX_ IN LISTS COMPARE_LIST)
    unset(FOUND_)

    foreach(COMPARE_ IN LISTS FILE_DEPENDENCIES_LIST_)
      if(COMPARE_ MATCHES "${COMPARE_REGEX_}")
        set(FOUND_ true)
        break()
      endif()
    endforeach()

    if(NOT FOUND_)
      message(FATAL_ERROR "Missing dependencies in '${FILE}'; check type: '${TYPE}'; file info: '${FILE_DEPENDENCIES_}'; missing: '${COMPARE_REGEX_}'")
    endif()
  endforeach()
endfunction()

# TODO add tests for what should not be present in lists
checkDependencies_("${FOUND_FILE_1}" "requires" "depend-application;depend-application-b")
checkDependencies_("${FOUND_FILE_2}" "requires" "depend-application;depend-application-b;libtest_lib\\.so.*")
checkDependencies_("${FOUND_FILE_3}" "requires" "depend-headers")
checkDependencies_("${FOUND_FILE_4}" "requires" "depend-default;depend-default-b")
checkDependencies_("${FOUND_FILE_5}" "requires" "depend-default;depend-default-b")

checkDependencies_("${FOUND_FILE_1}" "conflicts" "conflict-application;conflict-application-b")
checkDependencies_("${FOUND_FILE_2}" "conflicts" "conflict-application;conflict-application-b")
checkDependencies_("${FOUND_FILE_3}" "conflicts" "conflict-headers")
checkDependencies_("${FOUND_FILE_4}" "conflicts" "conflict-default;conflict-default-b")
checkDependencies_("${FOUND_FILE_5}" "conflicts" "conflict-default;conflict-default-b")

checkDependencies_("${FOUND_FILE_1}" "provides" "provided-default;provided-default-b")
checkDependencies_("${FOUND_FILE_2}" "provides" "provided-default;provided-default-b")
checkDependencies_("${FOUND_FILE_3}" "provides" "provided-default;provided-default-b")
checkDependencies_("${FOUND_FILE_4}" "provides" "provided-lib")
checkDependencies_("${FOUND_FILE_5}" "provides" "provided-lib_auto;provided-lib_auto-b")
