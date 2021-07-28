function(checkScripts_ FILE COMPARE_LIST)
  set(whitespaces_ "[\t\n\r ]*")

  execute_process(COMMAND ${RPM_EXECUTABLE} -qp --scripts ${FILE}
          WORKING_DIRECTORY "${CPACK_TEMPORARY_DIRECTORY}"
          OUTPUT_VARIABLE FILE_SCRIPTS_
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(COMPARE_LIST STREQUAL "")
    if(NOT FILE_SCRIPTS_ STREQUAL "")
      message(FATAL_ERROR "No scripts were expected in '${FILE}'; file info: '${FILE_SCRIPTS_}'")
    endif()
  else()
    string(REPLACE "\n" ";" FILE_SCRIPTS_LIST_ "${FILE_SCRIPTS_}")

    foreach(COMPARE_REGEX_ IN LISTS COMPARE_LIST)
      unset(FOUND_)

      foreach(COMPARE_ IN LISTS FILE_SCRIPTS_LIST_)
        if(COMPARE_ MATCHES "${COMPARE_REGEX_}")
          set(FOUND_ true)
          break()
        endif()
      endforeach()

      if(NOT FOUND_)
        message(FATAL_ERROR "Missing scripts in '${FILE}'; file info: '${FILE_SCRIPTS_}'; missing: '${COMPARE_REGEX_}'")
      endif()
    endforeach()
  endif()
endfunction()

if(RunCMake_SUBTEST_SUFFIX MATCHES "no_scripts.*")
  checkScripts_("${FOUND_FILE_1}" "")
  checkScripts_("${FOUND_FILE_2}" "")
else()
  checkScripts_("${FOUND_FILE_1}" "echo \"pre install foo\";echo \"post install foo\";echo \"pre uninstall foo\";echo \"post uninstall foo\";echo \"pre trans foo\";echo \"post trans foo\"")
  checkScripts_("${FOUND_FILE_2}" "echo \"pre install\";echo \"post install\";echo \"pre uninstall\";echo \"post uninstall\";echo \"pre trans\";echo \"post trans\"")
endif()
