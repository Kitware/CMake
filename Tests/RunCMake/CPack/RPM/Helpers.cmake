set(ALL_FILES_GLOB "*.rpm")

function(getPackageContent FILE RESULT_VAR)
  execute_process(COMMAND ${RPM_EXECUTABLE} -pql ${FILE}
          OUTPUT_VARIABLE package_content_
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(${RESULT_VAR} "${package_content_}" PARENT_SCOPE)
endfunction()
