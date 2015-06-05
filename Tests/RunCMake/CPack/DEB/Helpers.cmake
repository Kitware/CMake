set(ALL_FILES_GLOB "*.deb")

find_program(DEB_EXECUTABLE dpkg)
if(NOT DEB_EXECUTABLE)
  message(FATAL_ERROR "error: missing dpkg executable required by the test")
endif()

function(getPackageContent FILE RESULT_VAR)
  execute_process(COMMAND ${DEB_EXECUTABLE} -c ${FILE}
          OUTPUT_VARIABLE package_content_
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(${RESULT_VAR} "${package_content_}" PARENT_SCOPE)
endfunction()
