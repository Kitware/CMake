set(ALL_FILES_GLOB "*.tar.gz")

function(getPackageContent FILE RESULT_VAR)
  execute_process(COMMAND ${TAR_EXECUTABLE} -ztvf ${FILE}
          OUTPUT_VARIABLE package_content_
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(${RESULT_VAR} "${package_content_}" PARENT_SCOPE)
endfunction()
