configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sourcefile.txt
    ${CMAKE_CURRENT_BINARY_DIR}/sourcefile-source-permissions.txt
    FILE_PERMISSIONS
        OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ
)

if (CMAKE_HOST_UNIX AND NOT MSYS)
  find_program(STAT_EXECUTABLE NAMES stat)
  if(NOT STAT_EXECUTABLE)
    return()
  endif()

  if (CMAKE_HOST_SYSTEM_NAME MATCHES "FreeBSD")
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %Lp "${CMAKE_CURRENT_BINARY_DIR}/sourcefile-source-permissions.txt"
      OUTPUT_VARIABLE output
    )
  elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %A "${CMAKE_CURRENT_BINARY_DIR}/sourcefile-source-permissions.txt"
      OUTPUT_VARIABLE output
    )
  else()
    execute_process(COMMAND "${STAT_EXECUTABLE}" -c %a "${CMAKE_CURRENT_BINARY_DIR}/sourcefile-source-permissions.txt"
      OUTPUT_VARIABLE output
    )
  endif()

  if (NOT output EQUAL "554")
    message(FATAL_ERROR "configure file has different permissions than "
        "desired, generated permissions: ${output}")
  endif()

endif()
