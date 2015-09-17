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
    message(FATAL_ERROR "Debian control verification failed for file: "
        "'${FILE}'; error output: '${err_}'")
  endif()

  foreach(FILE_ IN LISTS VERIFY_FILES)
    file(READ "${CMAKE_CURRENT_BINARY_DIR}/control_${PREFIX}/${FILE_}" content_)
    if(NOT content_ MATCHES "${${PREFIX}_${FILE_}}")
      message(FATAL_ERROR "Unexpected content in for '${PREFIX}_${FILE_}'!"
          " Content: '${content_}'")
    endif()

    execute_process(COMMAND ls -l "${CMAKE_CURRENT_BINARY_DIR}/control_${PREFIX}/${FILE_}"
          OUTPUT_VARIABLE package_permissions_
          ERROR_VARIABLE package_permissions_error_
          OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT package_permissions_error_)
      if(NOT package_permissions_ MATCHES "${${PREFIX}_${FILE_}_permissions_regex}")
        message(FATAL_ERROR "Unexpected file permissions for ${PREFIX}_${FILE_}: '${package_permissions_}'!")
      endif()
    else()
      message(FATAL_ERROR "Listing file permissions failed (${package_permissions_error_})!")
    endif()
  endforeach()
endfunction()

function(getPackageInfo FILE RESULT_VAR)
  execute_process(COMMAND ${DPKG_EXECUTABLE} -I ${FILE}
          WORKING_DIRECTORY "${CPACK_TEMPORARY_DIRECTORY}"
          OUTPUT_VARIABLE package_info_
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(${RESULT_VAR} "${package_info_}" PARENT_SCOPE)
endfunction()
