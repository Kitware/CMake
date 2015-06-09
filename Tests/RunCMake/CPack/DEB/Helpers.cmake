set(ALL_FILES_GLOB "*.deb")

function(getPackageContent FILE RESULT_VAR)
  execute_process(COMMAND ${DPKG_EXECUTABLE} -c ${FILE}
          OUTPUT_VARIABLE package_content_
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(${RESULT_VAR} "${package_content_}" PARENT_SCOPE)
endfunction()
