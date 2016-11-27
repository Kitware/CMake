set(ALL_FILES_GLOB "*.tar.gz")

function(getPackageContent FILE RESULT_VAR)
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar -ztvf ${FILE}
          OUTPUT_VARIABLE package_content_
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(${RESULT_VAR} "${package_content_}" PARENT_SCOPE)
endfunction()

function(getPackageNameGlobexpr NAME COMPONENT VERSION REVISION FILE_NO RESULT_VAR)
  if(COMPONENT)
    set(COMPONENT "-${COMPONENT}")
  endif()

  set(${RESULT_VAR} "${NAME}-${VERSION}-*${COMPONENT}.tar.gz" PARENT_SCOPE)
endfunction()
