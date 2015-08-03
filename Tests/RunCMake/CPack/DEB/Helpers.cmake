set(ALL_FILES_GLOB "*.deb")

function(getPackageContent FILE RESULT_VAR)
  execute_process(COMMAND ${DPKG_EXECUTABLE} -c ${FILE}
          OUTPUT_VARIABLE package_content_
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(${RESULT_VAR} "${package_content_}" PARENT_SCOPE)
endfunction()

function(verifyDebControl FILE PREFIX VERIFY_FILES)
  execute_process(COMMAND ${DPKG_EXECUTABLE} --control ${FILE} control_${PREFIX}
          ERROR_VARIABLE err_)

  if(err_)
    message(FATAL_ERROR "Debian controll verification failed for file: "
        "'${FILE}'; error output: '${err_}'")
  endif()

  foreach(FILE_ IN LISTS VERIFY_FILES)
    file(READ "${CMAKE_CURRENT_BINARY_DIR}/control_${PREFIX}/${FILE_}" content_)
    if(NOT content_ MATCHES "${${PREFIX}_${FILE_}}")
      message(FATAL_ERROR "Unexpected content in for '${PREFIX}_${FILE_}'!"
          " Content: '${content_}'")
    endif()
  endforeach()
endfunction()
