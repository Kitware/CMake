set(ALL_FILES_GLOB "*.rpm")

find_program(RPM_EXECUTABLE rpm)
if(NOT RPM_EXECUTABLE)
  message(FATAL_ERROR "error: missing rpm executable required by the test")
endif()

function(getPackageContent FILE RESULT_VAR)
  execute_process(COMMAND ${RPM_EXECUTABLE} -pql ${FILE}
          OUTPUT_VARIABLE package_content_
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(${RESULT_VAR} "${package_content_}" PARENT_SCOPE)
endfunction()
