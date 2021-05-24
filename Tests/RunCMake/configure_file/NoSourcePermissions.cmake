configure_file(NoSourcePermissions.sh NoSourcePermissions.sh.out
               NO_SOURCE_PERMISSIONS)

if (UNIX AND NOT MSYS)
  execute_process(COMMAND ${CMAKE_CURRENT_BINARY_DIR}/NoSourcePermissions.sh.out
                  RESULT_VARIABLE result)
  if (result EQUAL "0")
    message(FATAL_ERROR "Copied file has executable permissions")
  endif()
endif()
